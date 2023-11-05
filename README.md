# textchanger220
A C application which parses command line arguments to take an input file and return a modified version of it as an output file in a new directory. The argument flags are:
1. -s <NAME>: text string being searched for
2. -r <NAME>: text string being used to replace an instance of the search text
3. -l <X,Y>: the first and last lines (inclusive) which will be modified in the output file; any previous or subsequent lines are not altered
4. -w: Switches on "wildcard" search mode, explained below
5. The last two arguments must always be two file paths, the first one being the path to the input file and the second being the path to the output file
Any other arguments are ignored.

-s and -r arguments and the input and output file paths are required. -s and -r must have a non-empty string argument following them. -l is optional but if present must have a valid start and end line argument in the form "start,end", where start>0. If the start or end number is greater than the lines of the input file, they are treated as being at/just after the end line.
-w argument is optional. No argument can be used more than once.

Simple search (no -w argument) searches for a specific string and replaces all instances of that string in the input file in the output file.
If -w is switched on, the search will be in "wildcard mode"; this means that the -s argument must have one asterisk either at the beginning or end: ex. the*, *at. If the asterisk is at the end, the program searches for words (delimited by spaces and punctuation marks) which start with the string given; if at the start, the program searches for words ending with the string given. The entire word will be replaced by whatever string is passed to -r (ending just before the next punctuation mark or space).

The program also handles several distinct errors in precedence order:
1. MISSING_ARGUMENT, if there are not enough arguments passed in the command
2. DUPLICATE_ARGUMENT, if one of the -s, -r, -l, or -w flags is seen more than once
3. INPUT_FILE_MISSING and OUTPUT_FILE_UNWRITEABLE, for cases in which one or both file pointers are invalid for some reason (if both, input file error has precedence)
4. S_ARGUMENT_MISSING, if there is no string argument passed to -s, or the string after -s also begins with "-" (Signifying a flag)
5. R_ARGUMENT_MISSING, if there is no valid string argument for -r (same criteria as above)
6. L_ARGUMENT_INVALID, if one or both numbers passed to -l cannot be parsed or if the start line number is greater than the end line number
7. WILDCARD_INVALID, if the -s argument in a wildcard search does not have an asterisk, has an asterisk which is not at the beginning or end of the string, or has more than one asterisk
