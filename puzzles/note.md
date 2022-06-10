Puzzle files are formatted with the following grammar.

FILE           := KNOWN_GROUP BLANK_LINE WORD_GROUP
KNOWN_GROUP    := KNOWN KNOWN_GROUP
                | KNOWN
KNOWN          := int ' ' letter '\n'
BLANK_LINE     := '\n'
WORD_GROUP     := WORD WORD_GROUP
                | WORD
WORD           := NUMBER_GROUP
NUMBER_GROUP   := int ' ' NUMBER_GROUP
                | int '\n'

a.pzl - taken from https://simplydailypuzzles.com/daily-codeword/ 16/5/2022

An example file would be:
"8 r
19 l

9 6 8 11 18 19
5 9 10 26
8 1 20 19 19 5
"
This file starts with two decoded letters: 8 -> r and 19 -> l. After these is a blank line
and the three code-word clues.
