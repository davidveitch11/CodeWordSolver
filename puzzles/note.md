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
