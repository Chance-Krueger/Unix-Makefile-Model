/*
* File: parseMakeFile.c 
* Author: Chance Krueger
* Purpose: Defines functions for parsing a Makefile and constructing
* a graph of parent and child nodes. It handles parsing lines to 
* extract commands and target names, adds nodes to the graph, 
* and frees dynamically allocated memory. Errors in the format 
* trigger a program exit with appropriate cleanup.
*/


# include "header.h"




// Function prototypes
int parseMakeFileData(FILE *fp); // return 1 if errSeen, 0 if not. If fatal error, exit program and return 1;
void addNodeToGraph(struct parentNode *node);

// GLOBAL VARIBLES
struct parentNode *headLL = NULL;
char *targetNode = NULL;
char *curLine = NULL;

/*
 * addParentNode(node) -- Adds a new parent node to the linked list (`headLL`).
 * Takes a pointer to a `struct parentNode` and appends it to the end of the list.
 * Assumes that `node` is properly allocated and initialized. If `headLL` is empty,
 * sets `headLL` to the new node.
 */
void addParentNode(struct parentNode *node) {

	if (headLL == NULL) {
		headLL = node;
		if (targetNode == NULL) {
                        targetNode = node->name;
                }
		return;
	}

	struct parentNode *cur = headLL;

	while (cur->next != NULL) {
	
		cur = cur->next;
	}
	cur->next = node;
}


/*
 * addChildNode(parent, child) -- Links a child node to a given parent node.
 * Takes a pointer to a `struct parentNode` (`parent`) and another `struct parentNode` (`child`).
 * Allocates a new `struct childNode` and adds it to the parent's `children` list.
 * Assumes that `parent` and `child` are valid pointers. Prevents duplicate children.
 */
void addChildNode(struct parentNode *parent, struct parentNode *child) {

	struct childNode *cur = parent->children;

	struct childNode *newChild = malloc(sizeof(struct childNode));
	
	if (newChild == NULL) {
		fprintf(stderr, "Ran Out of Memory.\n");
		exit(1);
	}
	
	newChild->to = child;
	newChild->next = NULL;

	if (cur == NULL) {
		parent->children = newChild;
		return;
	}

	while (cur->next != NULL) {

		if (cur->to == child) {
			return;
		}
		cur = cur->next;
	}
	if (cur->to != child) {
		cur->next = newChild;
	}
}



/*
 * findParentNode(name) -- Searches for a parent node with the given name.
 * Takes a string (`char *name`) and returns a pointer to the `struct parentNode`
 * if found in the linked list. If the node does not exist, returns `NULL`.
 * Assumes `headLL` is properly initialized.
 */
struct parentNode* findParentNode(char *name) {

	struct parentNode *cur = headLL;

        while (cur != NULL) {
                if (strcmp(cur->name, name) == 0) {
                        // Found node.
			return cur;
                }
                cur = cur->next;
        }
	// No node yet created.
	return NULL;
}

/*
 * searchHeadActionForDup(name) -- Checks if a parent node with a given name has children.
 * Takes a string (`char *name`) and returns `1` if the node exists and has children,
 * otherwise returns `0`. If no matching node is found in `headLL`, returns `0`.
 * Assumes that `headLL` is properly initialized.
 */
int searchHeadActionForDup(char *name) {

	struct parentNode *cur = headLL;

	while (cur != NULL) {
		if (strcmp(cur->name, name) == 0) {
			if (cur->children != NULL) {
				return 1;
			}
			return 0;
		}
		cur = cur->next;
	}
	return 0;
}


/*
 * useActionLineAndAddNodes(line) -- Parses a makefile-style action line and adds nodes to the dependency graph.
 * Takes a string (`char *line`) representing a target and its dependencies, then:
 * - Extracts the target and dependencies using `sscanf`.
 * - Ensures the target is unique (no duplicate entries).
 * - Creates the `parentNode` if it does not exist.
 * - Adds dependencies as child nodes.
 * Returns a pointer to the `struct parentNode` representing the target.
 *
 * Assumptions:
 * - `line` is a valid string formatted as `target : dependencies` with exactly one colon (`:`).
 * - The memory for new nodes must be allocated dynamically.
 * - Dependencies are space-separated and can be empty.
 * - Assumes `headLL` is properly initialized before calling.
 *
 * Error Handling:
 * - Exits if memory allocation fails.
 * - Exits if the format is invalid (too many or too few components).
 * - Exits if the target node already exists.
 */
struct parentNode* useActionLineAndAddNodes(char* line) {

	char *tempPointers = calloc((strlen(line) + 1), (sizeof(char)));//?
	
	if (tempPointers == NULL) {
		freeLL();
		fprintf(stderr, "Ran Out of Memory.\n");
		exit(1);
	}
	
	char tempTarget[65];
	int retVal = sscanf(line, "%s : %[^\n]", tempTarget, tempPointers);

	if (retVal  > 2 || (retVal < 1)) {
		free(tempPointers);
		// FREE MEMORY
		freeLL();
		fprintf(stderr, "Invalid Format.\n");
		exit(1);
	}

	char *restPointers = strdup(tempPointers);
	char *target = strdup(tempTarget);
		
	free(tempPointers);

	// OG Action has already been initialized.
	if (searchHeadActionForDup(target) == 1) {
		// FREE MEMORY
		freeLL();
		fprintf(stderr, "Invalid Format. Node already exists.\n");
		exit(1);
	}	

	struct parentNode *actionHead = findParentNode(target);

	// CREATE PARENT HEAD AND ADD TO LL.
	if (actionHead == NULL) {
		actionHead = malloc(sizeof(struct parentNode));
		
		if (actionHead == NULL) {
			freeLL();
			fprintf(stderr, "Ran Out of Memory.\n");
                	exit(1);
		}
		
		actionHead->name = strdup(target);
		actionHead->cmds = NULL;
		actionHead->children = NULL;
		actionHead->next = NULL;
		actionHead->visited = 0;
		actionHead->doesExist = 0;
		actionHead->mustBuild = 0;
		actionHead->completed = 0;
		actionHead->fileDate = (time_t)-1;
		actionHead->target = 1;

		// ADD NEW NODE.
		addParentNode(actionHead);
	}


	free(target);

	char *tempPointer = malloc((strlen(restPointers) + 1) * sizeof(char)); 

	if (tempPointer == NULL) {
		// FREE MEMORY
        	freeLL();
		fprintf(stderr, "Ran Out of Memory.\n");
                exit(1);
        }
	int retval;
	char* restSave = restPointers;
	while ((retval = sscanf(restPointers, "%s", tempPointer)) == 1) {
	
		struct parentNode *action = findParentNode(tempPointer);

		// CREATE NODE AND ADD TO LL
		if (action == NULL) {
			action = malloc(sizeof(struct parentNode));
			action->name = strdup(tempPointer);
                	action->cmds = NULL;
                	action->children = NULL;
                	action->next = NULL;
                	action->visited = 0;
			action->doesExist = 0;
			action->mustBuild = 0;
			action->completed = 0;
			action->fileDate = (time_t)-1;
			action->target = 0;
			addParentNode(action);
		}
		// ADD TO HEAD CHILDREN.
		addChildNode(actionHead, action);

		restPointers += strlen(tempPointer);

		while (isspace(*restPointers)) {
    			restPointers++;
		}

		free(tempPointer);
		tempPointer = malloc((strlen(restPointers) + 1) * sizeof(char));
		if (tempPointer == NULL) {
			// FREE MEMORY
			free(restPointers);
			freeLL();
                	fprintf(stderr, "Ran Out of Memory.\n");
                	exit(1);
        	}
	}

	free(restSave);
	free(tempPointer);
	return actionHead;
}


/*
 * printLL() -- Prints the current state of the parent-child linked list.
 * Used for debugging purposes, this function iterates through the list
 * and displays parent nodes, child dependencies, and associated commands.
 * Assumes that `headLL` is properly initialized. If `headLL` is empty,
 * prints a message indicating that the list is empty.
 */
void printLL() {

	struct parentNode *curP = headLL;
	
	if (headLL == NULL) {
		printf("HEAD LL is EMPTY");
		return;
	}


	while (curP != NULL) {

		printf("CUR PARENT: %s\n", curP->name);

		struct childNode *curC = curP->children;

		while (curC != NULL) {

			printf("	CUR CHILD: %s\n", curC->to->name);

			curC = curC->next;
		}
		
		struct LineNode *curCM = curP->cmds;
		while (curCM != NULL) {
			printf("		Commands: %s\n",curCM->line);
			curCM = curCM->next;
		}
		curP = curP->next;
	}
}



/*
 * addLine(head, newLine) -- Adds a new line entry to a command linked list.
 * Takes a pointer to a linked list (`struct LineNode **head`) and a new command (`char *newLine`).
 * Allocates memory for a new node, stores the command, and appends the node to the list.
 * Assumes `head` is a pointer to a valid linked list or is `NULL` (empty list).
 * Exits if memory allocation fails.
 */
void addLine(struct LineNode **head, char *newLine) {

	struct LineNode *newNode = malloc(sizeof(struct LineNode));

	if (newNode == NULL) {
		freeLL();
		fprintf(stderr, "Ran Out Of Memory.\n");
		exit(1);
	}

	newNode->line = strdup(newLine);  // Store a copy of the line
    	newNode->next = NULL;

	if (*head == NULL) {
		*head = newNode;
		return;
	}

	struct LineNode *cur = *head;
        while (cur->next != NULL) {
            	cur = cur->next;
        }
        cur->next = newNode;

}

/*
 * countColons(line) -- Counts the number of colons (`:`) in a given string.
 * Takes a string (`char *line`) and returns the count of colons.
 * Assumes `line` is a valid null-terminated string.
 */
int countColons(char *line) {
	int colons = 0;

	while (*line != 0) {
		if (*line == ':') {
			colons++;
		}
		line++;
	}
	return colons;
}

/*
 * fixLine(oldLine) -- Cleans and formats a makefile dependency line.
 * Takes a string (`char *oldLine`) and returns a new formatted string:
 * - Removes leading and trailing whitespace.
 * - Adds a space before or after colons (`:`) to ensure proper parsing.
 * - Eliminates consecutive spaces.
 * The returned string is dynamically allocated, so it must be freed by the caller.
 * Assumes `oldLine` is a valid null-terminated string.
 */
char* fixLine(char *oldLine) {
	char *newLine = calloc((strlen(oldLine) + 3), sizeof(char));

	if (newLine == NULL) {
		freeLL();
		fprintf(stderr, "Ran Out of Mem.\n");
		exit(1);
	}

	int index = 0;
	int newIndex = 0;
	while (oldLine[index] != 0) {

		// gets rid of leading whitespace
		if (newIndex == 0) {
			while (isspace(oldLine[index])) {
					index++;
			}
		}

		// trailing spaces
		while (isspace(oldLine[index]) && isspace(oldLine[index + 1])) {
			index++;
		}

		// adds space either before or after ':'
		if (oldLine[index + 1] == ':') {
			newLine[newIndex] = oldLine[index];
			newLine[newIndex + 1] = ' ';
			newIndex += 2;
		} else if (oldLine[index] == ':' && !isspace(oldLine[index + 1])) {
			newLine[newIndex] = oldLine[index];
			newLine[newIndex + 1] = ' ';
			newIndex += 2;
		} else {
			newLine[newIndex] = oldLine[index];
			newIndex++;
		}
		index++;
	}
	newLine[newIndex] = 0;
	char *cpy = strdup(newLine);
	free(newLine);
	return cpy;
}



/*
 * parseMakeFileData(fp) -- Parses a makefile-style input file and constructs a dependency graph.
 * Takes a file pointer (`FILE *fp`) and processes each line:
 * - Identifies command lines (starting with whitespace) and adds them to the linked list.
 * - Parses target dependency lines, ensuring the format is valid.
 * - Creates and links parent and child nodes dynamically based on dependencies.
 *
 * Assumptions:
 * - `fp` is a valid open file pointer.
 * - Lines are properly formatted, with targets followed by a colon (`:`) and dependencies.
 * - The function properly tracks commands (`CMDS`) so they are linked to the correct nodes.
 *
 * Error Handling:
 * - Exits if format validation fails (missing or extra colons).
 * - Ensures proper memory management and frees allocated structures on failure.
 * - If memory allocation fails, calls `freeLL()` to clean up.
 *
 * Returns:
 * - `0` on successful parsing.
 * - If errors are detected, it exits the program after printing an error message.
 */
int parseMakeFileData(FILE *fp) {

	char *line = NULL;
	size_t size = 0;
	int errSeen = 0;
	struct LineNode *CMDS = NULL; 
	
	struct parentNode *temp = NULL;

	while (getline(&line, &size, fp) > 0) {
		
		curLine = line;		
		// IS CMDS
		if (line[0] == '\t') {

			if (headLL == NULL) {
				fprintf(stderr, "Invalid Format. Commands Cannot be First.\n");
				free(line);
				exit(1);
			}

			// RESET CMDS so its unique to this action		
			
			char *noSpace = line;

			while(isspace(*noSpace)) {
				noSpace++;
			}

			// MANUALLY REMOVE TRAILING NEWLINE
			char *p = noSpace;
			while (*p != '\0') {
    				if (*p == '\n') {
        				*p = '\0';
        				break;
    				}
    				p++;
			}
			// CALL FUNCTION THAT WILL SEARCH IF NODE EXISTS ALREADY, IF NOT ADD NODE TO GRAPH, (FIRST CHECK IF HEAD ACTION IS A DUP) 
			if (strlen(noSpace) > 0) {
				addLine(&CMDS, noSpace);
			}

		// IS TARGETS
		} else {

			// Will make sure that the line isn't an empty line.
			int tempIndex = 0;
			while (isspace(line[tempIndex] && line[tempIndex] != 0)) {
				tempIndex++;
			}

			// if it hit the end of the while loop, go to the next iteration (line).
			if (line[tempIndex] == 0 || line[tempIndex] == '\n') {
				continue;
			}
			// ADD CMDS TO TEMP
			if (temp != NULL) {
				temp->cmds = CMDS;
			}

			// Either no colons were found or command was not tabbed.
			if (countColons(line) != 1) {
				//FREE MEMORY
				freeLL();
				free(line);
				fprintf(stderr, "Invalid Format, either no or one too many Colons.\n");
				exit(1);
			}

			char *dupLine = strdup(line);        
			char *fixedLine = fixLine(dupLine);   

			free(dupLine);                       

			// MAKE TEMP THIS
			temp = useActionLineAndAddNodes(fixedLine);
			CMDS = NULL;
			free(fixedLine);
		}
	}
	if (CMDS != NULL) {
		temp->cmds = CMDS;
	}
	free(line);
	curLine = NULL;
	return errSeen;
}
