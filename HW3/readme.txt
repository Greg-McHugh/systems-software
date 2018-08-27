Compile the compiler.c file.
$gcc compiler.c

Then run the resulting executable. On default, it will be named "a.out"
(unless you used "-o" to give it a different name, but for these instructions I'll assume you didn't).
$./a.out

The executable will be expecting the PL/0 code to be in a file named "input.txt" in the same directory.

All printed output will be printed out to a file "output.txt" in the same directory as the executable.
The scanner and lexical analyzer prints to "lexerOutput.txt".
The parser and code generator prints to "parserOutput.txt".
The virtual machine prints to "vmOutput.txt".


Included in this zip file is a sample program in a "input.txt" file and the resulting "output.txt" file.
Also included is a list of 25 errors that can be reported named "error-list.txt"
and a sample code with resulting output for each possible error.
These sample input and output files for these errors are all in the "errors" directory.
