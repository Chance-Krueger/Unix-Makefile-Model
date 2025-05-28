/*
* File: mymake2.c
* Author: Chance Krueger
* Purpose: Implements the main function for the `mymake2` program, which parses 
* a makefile, constructs a dependency graph, and executes commands based on 
* file modification timestamps. Handles argument parsing, error checking, 
* and memory management.
*/



# include "header.h"



// TAKE ARGS IN STRING params
int main(int argc, char* argv[]) {

	int errSeen = 0;
	FILE *aMakeFile;
	char *aTargetFile = NULL;

	int needFree = 0;
	int openFile = 0;

	int tempIndex = 0;
	while (tempIndex < argc) {
		if (strcmp(argv[tempIndex], "-f") == 0) {
			openFile++;
		}
		tempIndex++;
	}

	// More than one -f options.
	if (openFile > 1) {
		fprintf(stderr, "Too many File (-f) options were Given.\n");
		return 1;
	} else if (openFile == 1) {

		if (argc > 4) {
			fprintf(stderr, "Too many Arguemnts were Given.\n");
			return 1;
		}

		aMakeFile = fopen(argv[2], "r");
	} else {
		// Needs to open myMakeFile;
		aMakeFile = fopen("myMakefile", "r"); 
	}
	
	
	if (aMakeFile == NULL) {
		fprintf(stderr, "Could not Open aMakeFile.\n");
		return 1;
	}

	errSeen += parseMakeFileData(aMakeFile);

	if (openFile && (argc > 3)) {
                aTargetFile = argv[3];
        } else {
		aTargetFile = strdup(targetNode);
		needFree = 1;
	}

	errSeen += parseTargetFileData(aTargetFile);
	
	if (needFree) {
		free(aTargetFile);
	}

	fclose(aMakeFile);

	return errSeen >= 1;
}