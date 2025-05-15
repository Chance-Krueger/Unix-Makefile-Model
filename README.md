# Unix-Makefile-Model
## Description
C program extending mymake to mimic the Unix make utility by parsing dependency files, building a graph of targets, checking file timestamps, and rebuilding targets as needed. Supports flexible argument order with -f for makefile input and optional target specification. Handles errors, cycles, and prints executed commands.

## What I learned: 
    - How to parse and process command-line arguments flexibly
    - Building and traversing dependency graphs in C
    - Using file system calls (stat) to check file existence and modification times
    - Implementing recursive algorithms with cycle detection
    - Executing shell commands from C using system()
    - Handling errors and edge cases robustly
    - Managing dynamic memory allocation and freeing it properly
    - Structuring a moderately complex program across multiple files

## How To run it:
### Compile the program using a C compiler, for example:
    - gcc mymake2.c graph.c -o mymake2

### Run the executable from the command line:
    ./mymake2 [-f aMakefile] [aTarget]

    - -f aMakefile is optional; if omitted, it uses the default myMakefile.
    - aTarget is optional; if omitted, it uses the first target in the makefile.
    - The -f flag and the target argument can be in any order.

### Once running
    - The program reads the makefile and builds the dependency graph.
    - It recursively rebuilds targets if needed based on timestamps and dependencies.
    - It prints each command it executes.
    - If no commands run, it prints "target is up to date."

## Example usages:
    - ./mymake2 -f customMakefile clean
    - ./mymake2 all -f customMakefile
    - ./mymake2
        - assuming that myMakefile is made

## Future Improvements
    - Improve error handling with more descriptive messages and recovery options.
    - Optimize file timestamp checks to reduce unnecessary command executions.
    - More optimized and shorten code
