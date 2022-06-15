
#include <inttypes.h>

#include "puzzel.h"
#include "data.h"

/**
 * @brief Full set of letters.
 * The method used for representing sets of the alphabet uses this value to represent
 * all letters included.
 */
#define FULL_SET 0x3FFFFFF

char solve();
void collatePossibilities();
void collect(struct cwrd* cw);
int findSolutions(char *solutions, int *guessable, int *guessable_num);
char noSolutions(int guessable_num, int guessable);
char recurse(char *solutions);
int count(uint32_t p);
void markDirty(int i);

// Set to 1 and recompile for the most debugging information
#define DEBUG 0

// Set to 1 and recompile for some information on the actions taken by the program
#define VERBOSE 1

// Set to 1 and recompile to limit the amount of recursion the program is capable of
#define TRACE_LIMIT 0

// Shorthand for printing debug info to stdout using printf arguments
#if DEBUG
#define d(m, ...) printf(m, ##__VA_ARGS__)
#else
#define d(m, ...)
#endif

// Shorthand for printing extra info to stdout using printf arguments
#if VERBOSE
#define v(m, ...) printf(m, ##__VA_ARGS__)
#else
#define v(m, ...)
#endif


/**
 * @brief The list of possibilities
 * Note that a possibility is a 32-bit vector representing a sub-set of the alphabet.
 * The least significant bit represents 'a', so 'z' is accessed by ((0x1 << 25) & v)
 */
uint32_t possible[26];

/**
 * @brief The set of letters that have already been used in the puzzle
 */
uint32_t used = 0;

/**
 * @brief The puzzle we are currently trying to solve
 */
struct puzzle* puzzle;

/**
 * @brief The current depth of recursion (only used when VERBOSE is set to 1)
 */
int depth = 0;

int main(int argc, char **argv) {
    // Represent board as array of arrays of integers, also keep pattern
    // Represent known letters as array[26] of chars, index represents
    // code number, value is coded letter. I.e, a[4] -> 'n': replace 4s with ns
    
    // Sweep:
    // For each word, collect all words from dictionary matching pattern
    // For each number in word, collect set of possible letters
    // Collate results to get:
    // For each number, list of sets of possible letters
    // Combine with union operation to get:
    // For each number, set of possible letters
    // If any have zero, return failure
    // Else, if any have one, use as solution and recurse
    // Else, pick smallest set, pick first value in set to be possible solution
    //       recurse with that, if fails, use next value, if no more, return failure

    // New Sweep:
    // Record sets for each code word, recalculate only if one of its letters updates
    // Collate results and union by setting up bit-vector for each number,
    //     once set is made for word, AND with present vector
    // Continue as stated

    // Arguments
    if (argc != 2) {
        printf("Usage: $ %s <file_name>\n", argv[0]);
        printf("where file_name is the location of the puzzle file\n");
        return -1;
    }

    // Initialise the data module - read words list so ready to search for words
    init();
    
    // Parse the puzzle file pointed to
    puzzle = parse(argv[1]);

    // Print out code letters that are in use if in verbose mode
    #if VERBOSE
    printf("code letters needed =");
    for (int i = 0; i < 26; i++) {
        if (puzzle->needed[i]) printf(" %d", i + 1);
    }
    printf("\n");
    printf(" (hence not needed) =");
    for (int i = 0; i < 26; i++) {
        if (!puzzle->needed[i]) printf(" %d", i + 1);
    }
    printf("\n");
    #endif

    // Attempt to solve the puzzle
    if (solve()) {
        printf("Puzzle Solved\n");
    } else {
        printf("Puzzle Not Solved\n");
    }

    // Print out final results
    printf("Mapping:\n");
    for (int i = 0; i < 26; i++) {
        if (puzzle->known[i]) printf("    %d -> %c\n", i + 1, puzzle->known[i]);
        else printf("    %d -> ?\n", i + 1);
    }

    // Print out decoded words if in verbose mode
    #if VERBOSE
    printf("Decoded Words:\n");
    struct cwrd** cwrds = puzzle->cwrds;
    struct cwrd* cwrd;
    while (cwrd = *cwrds) {
        printf("    ");
        for (int i = 0; i < cwrd->len; i++) {
            char c = puzzle->known[cwrd->clets[i] - 1];
            if (c) printf("%c", c);
            else printf(" %d ", i + 1);
        }
        printf("\n");

        cwrds++;
    }
    #endif

    freePuzzle(puzzle);

    return 0;
}

/**
 * @brief Recursively attempt to solve the puzzle
 * 
 * @return char 1 on success, 0 on failure
 */
char solve() {
    // Check for case where all solutions found
    int t = 0;
    for (int i = 0; i < 26; i++) {
        if (puzzle->known[i] || puzzle->needed[i] == 0) {
            t++;
        }
    }
    if (t == 26) {
        v("All required letters decoded\n");
        return 1;
    }

    // Check if recursion limit reached yet
    #if TRACE_LIMIT
    depth++;
    if (depth > 30) {
        printf("Reached recursion depth\n");
        return 0;
    }
    #endif

    // Print out the current known or assumed solutions if in verbose mode
    #if VERBOSE
    printf("Attempting to solve\n");
    printf("Current Known = ");
    for (int i = 0; i < 26; i++) {
        printf("%d ", puzzle->known[i]);
    }
    printf("\n");
    #endif

    // Construct the 'used' value (set of letters which have been used in the puzzle so far)
    used = 0x0;
    for (int i = 0; i < 26; i++) {
        // If code letter i has a known value, set that value's bit in the used vector
        if (puzzle->known[i]) {
            used |= 0x1 << (puzzle->known[i] - 'a');
        }
    }

    d("Used = %x\n", used);

    // Collect possible decodings for each code word and collate results
    collatePossibilities();

    #if DEBUG
    printf("Collated possibilities\n");
    for (int i = 0; i < 26; i++) {
        printf("    %x\n", possible[i]);
    }
    #endif

    // Look for solutions

    // A set of letters (initially empty) given to each code letter
    char solutions[26] = {0};

    // The number of full solutions found
    int num_solns = 0;
    
    // The index of the code letter with the fewest possible solutions (>1) and
    // the number of solutions. I.e., the most easily guessable letter
    int guessable = -1;
    int guessabel_num = -1;

    // Look through possibilities to find solutions
    num_solns = findSolutions(solutions, &guessable, &guessabel_num);

    // Print out the results of finding solutions if in verbose mode
    #if VERBOSE
    printf("Found %d solutions\n", num_solns);
    if (num_solns == 0) {
        printf("Guessing using %d (%d options)\n", guessable, guessabel_num);
    } else {
        printf("Solutions = ");
        for (int i = 0; i < 26; i++) {
            if (solutions[i]) printf("%c ", solutions[i]);
            else printf("0 ");
        }
        printf("\n");
    }
    #endif

    // There was a code letter where there were no possible solutions
    if (num_solns < 0) {
        v("No possible solutions\n");
        return 0;
    }

    // If no solutions found, choose one
    if (num_solns == 0) {
        // Use the shortest one (i = guessable)

        d("    G=%d, GN=%d\n", guessable, guessabel_num);

        // Choose any possible solution to guess and recurse. Returns the best result of guessing
        char ret = noSolutions(guessabel_num, guessable);

        #if TRACE_LIMIT
        depth--;
        #endif

        return ret;
    }

    // Recurse if solutions were found - add solutions and recurse
    char ret = recurse(solutions);

    #if TRACE_LIMIT
    depth--;
    #endif

    return ret;
}

/**
 * @brief Find the possible solutions for each codeword and collate the results.
 * Finds the possible solutions for each codeletter for each codeword.
 * Hence, finds the possible solutions for each codeletter that fit all codewords 
 */
void collatePossibilities() {
    // Reset possibility sets
    for (int i = 0; i < 26; i++) {
        possible[i] = FULL_SET;
    }

    // Collect and collate
    struct cwrd** cs = puzzle->cwrds;
    struct cwrd* cw;
    while (cw = *cs) {
        // If the codeword has been changed (one of its codeletters decoded) since last calculation of
        // possibilities for its letters, recalculate. Else, use already calculated values
        if (cw->dirty) {
            // Calculate possibilities for this codeword
            collect(cw);

            // Reset dirty bit
            cw->dirty = 0;
        }

        // Collate each value in possibilities list with total list, for each codeletter in word
        for (int i = 0; i < cw->len; i++) {
            d("  possibilities (%d) %x\n", cw->clets[i], cw->possible[i]);

            int clet = cw->clets[i] - 1;
            possible[clet] = possible[clet] & cw->possible[i];
        }

        // next
        cs++;
    }

    // Remove from possibility list any letters which have already been used elsewhere
    for (int i = 0; i < 26; i++) {
        possible[i] = possible[i] & (~used);
    }
}

/**
 * @brief Find possible solutions for the puzzle.
 * If possible, find solutions in the global list of possible solutions.
 * If no definite solutions can be found, return the most guessable solutions.
 * If any letter is found to have no possibilities, the puzzle is unsolvable from this state,
 * and returns -1
 * @param solutions array of zeros representing codeletters, where definite solutions can be placed
 * @param guessable the index of the "most guessable" code letter - the one with the fewest possible solutions (>1)
 * @param guessable_num the number of solutions of the "most guessable" codeletter
 * @return int number of solutions found (>= 0) or -1 if an unsolvable letter found
 */
int findSolutions(char *solutions, int *guessable, int *guessable_num) {
    d("Finding solutions\n");

    // Count number of definite solutions found
    int num = 0;

    // For each codeletter
    for (int i = 0; i < 26; i++) {
        // Ignore codeletters where the letter is already known
        if (puzzle->known[i]) {
            continue;
        }

        // Count number of possible solutions for code letter i
        int n = count(possible[i]);

        d("    %d possibilities for number %d", n, i);

        if (n == 0) {
            // No possible solutions - error

            d(" - failure\n");

            // return failure
            return -1;
        } else if (n == 1) {
            // Exactly one possibility - Add to list of solutions
            
            uint32_t mask = 0x1;
            for (char c = 'a'; c <= 'z'; c++) {
                if (mask & possible[i]) {
                    solutions[i] = c;
                    break;
                }

                mask = mask << 1;
            }

            d(" - found solution\n");

            num++;
        } else {
            // More than one possibility - still a guessable codeletter - maybe save for later
            
            // If this letter has the fewest solutions of those seen so far (>1), save if for later
            if (*guessable_num == -1 || *guessable_num > n) {
                *guessable_num = n;
                *guessable = i;

                d(" - set as guessable\n");
            }
        }
    }

    return num;
}

/**
 * @brief Recursively solve using the most guessable codeletter.
 * In the event there is no definite solution, the index guessable represents the codeletter with the
 * fewest possible solutions. For each of these solutions, assume it is the correct solutions and
 * recursively try to continue solving. If the solution works, great. Otherwise, try the next possibility.
 * If no more possibilities remain, return a failure.
 * @param guessable_num number of possible solutions
 * @param guessable code letter that is guessable
 * @return char 1 on success, 0 on failure
 */
char noSolutions(int guessable_num, int guessable) {
    // Check for case where no letter had any possible solutions
    if (guessable_num == -1) {
        v("No guessable solutions\n");
        return 0;
    }

    // Iterate over possible solutions for the guessable code letter
    uint32_t mask = 0x1;
    for (char i = 0; i < 26; i++) {
        if (mask & possible[guessable]) {
            // choose this letter
            char c = i + 'a';

            // Add to list of guesses
            puzzle->known[guessable] = c;

            // Mark this letter as changed for all relevant code words
            markDirty(guessable);

            v("Guessing %d -> %c\n", guessable + 1, c);

            // Attempt to solve with this guess
            if (solve()) {
                // Guessed correctly
                return 1;
            }

            v("(guess failed)\n");
        }

        mask = mask << 1;
    }

    // None of the guesses produced a solution - return failure
    
    // Remove assumed solutions
    puzzle->known[guessable] = 0;

    v("No guess produced solution\n");
    return 0;
}

/**
 * @brief Recursively solve if at least one definite solution found
 * @param solutions solutions found (i-th entry represents decoding of codeletter i)
 * @return char 0 on failure, 1 on success
 */
char recurse(char *solutions) {
    // Add solutions to values and recurse
    for (int i = 0; i < 26; i++) {
        if (solutions[i] == 0) {
            continue;
        }

        // Add to puzzle list
        char s = solutions[i];
        puzzle->known[i] = s;

        // Mark all codewords that use codeletter i as dirty
        markDirty(i);
    }

    // Otherwise recurse
    return solve();
}

/**
 * @brief Collects, for each number, the list of possible letters matching the pattern
 * @param cw Representation of the code word
 */
void collect(struct cwrd* cw) {
    // Reset possibility sets
    for (int i = 0; i < cw->len; i++) {
        cw->possible[i] = 0;
    }
    
    #if DEBUG
    printf("    filled pattern =");
    for (int i = 0; i < cw->len; i++) {
        printf(" %d", cw->known[i]);
    }
    printf("\n");
    #endif

    // Construct pattern
    for (int i = 0; i < cw->len; i++) {
        // If the cw doesn't know position i and the puzzel does, move it into the known field
        if (cw->known[i] == 0 && puzzle->known[cw->clets[i] - 1]) {
            d("  position=%d code-letter=%d known-value=%d puzzle-known=%d\n",
                i, cw->known[i], cw->clets[i] - 1, puzzle->known[cw->clets[i] - 1]);
            
            cw->known[i] = puzzle->known[cw->clets[i] - 1];
        }
    }

    #if DEBUG
    printf("    cw-known-values =");
    for (int i = 0; i < cw->len; i++) {
        printf(" %d", cw->known[i]);
    }
    printf("\n");
    #endif

    // Indicate to datastore that we are now searching using a different pattern
    newPattern(cw->len, cw->pattern, cw->known);

    // Iterate over words matching pattern
    char *word;
    while (word = nextWord()) {
        for (int i = 0; i < cw->len; i++, word++) {
            // Get ith letter c from word

            // Convert to mask
            uint32_t mask = 0x1 << ((*word) - 'a');

            // 'or' onto possible[i]
            cw->possible[i] |= mask;
        }
    }
}

/**
 * @brief Mark any codewords containing the code letter i as dirty
 * 
 * @param i code letter to search for
 */
void markDirty(int i) {
    struct cwrd** cs = puzzle->cwrds;
    struct cwrd* c;
    while (c = *cs) {
        for (int j = 0; j < c->len; j++) {
            if (i == c->clets[j] - 1) {
                c->dirty = 1;
                break;
            }
        }

        cs++;
    }
}

/**
 * @brief Counts the number of 1 bits in a vector
 * 
 * @param p vector to be counted
 * @return int number of bits set in p
 */
int count(uint32_t p) {
    // Add adjacent pairs of bits
    uint32_t t = ((p & 0xAAAAAAAA) >> 1) + (p & 0x55555555);

    // Add pairs of pairs
    t = ((t & 0xCCCCCCCC) >> 2) + (t & 0x33333333);

    // Add groups of four so each byte now holds num bits that was in byte
    t = ((t & 0xF0F0F0F0) >> 4) + (t & 0x0F0F0F0F);

    // Add pairs of adjacent bytes
    t = ((t & 0x0F000F00) >> 8) + (t & 0x000F000F);

    // Add pairs of pairs to get 32-bit total
    t = ((t & 0x00FF0000) >> 16) + (t & 0x000000FF);

    return t;
}
