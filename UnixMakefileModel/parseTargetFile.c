/*
* File: parseTargetFile.c
* Author: Chance Krueger
* Purpose: Defines functions for parsing target data from a file,
* performing postorder traversal of a target tree, and freeing 
* dynamically allocated memory. The functions handle target node 
* verification, printing the target tree, and cleaning up resources.
*/


# include "header.h"

int parseTargetFileData(char *fp); // return 1 if errSeen, 0 if not. If fatal error, exit program and return 1;
void PostOrder(struct parentNode *node); // PRINT TREE IN POSTORDER (target from parseTargetFileData)
struct parentNode *verifyTargetNode(char *name);
int CMDSused = 0;


/*
 * freeLines(head) -- Frees memory allocated for a linked list of command lines.
 * Takes a pointer to the head of a linked list (`struct LineNode *head`) and 
 * deallocates each node's memory. Assumes `head` points to a valid linked list 
 * structure or is NULL. This function does not modify `head` itself.
 */
void freeLines(struct LineNode *head) {
        struct LineNode *cur = head;
        while (cur != NULL) {
                struct LineNode *temp = cur;
                cur = cur->next;
                free(temp->line);
                free(temp);
        }
}

/*
 * freeLL() -- Frees memory allocated for the entire parent node linked list (`headLL`).
 * Iterates through all parent nodes, freeing child nodes and command lists as well.
 * Assumes `headLL` is a valid linked list structure or is NULL. This function cleans
 * up dynamically allocated memory associated with nodes and commands.
 */
void freeLL() {

	struct parentNode *curP = headLL;

	while (curP != NULL) {

		struct childNode *curC = curP->children;

		while (curC != NULL) {

			struct childNode *temp = curC;
                	curC = curC->next;
                	free(temp);
		}
		struct parentNode *temp = curP;
		curP = curP->next;
		freeLines(temp->cmds);
		free(temp->name);
		free(temp);
		if (curLine != NULL) {
			free(curLine);
		}
	}
}

/*
 * verifyTargetNode(name) -- Searches for a parent node with the given name.
 * Takes a string (`char *name`) and returns a pointer to the `struct parentNode`
 * that matches, or `NULL` if no match is found. Assumes `headLL` is properly 
 * initialized and `name` is a valid string.
 */
struct parentNode *verifyTargetNode(char *name) {

	struct parentNode *cur = headLL;

	while (cur != NULL) {
		if (strcmp(cur->name, name) == 0) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}



/*
 * praseCMDS(node) -- Executes the command list associated with a target node.
 * Iterates through `node->cmds`, printing each command before executing it with `system()`.
 * If a command fails (returns a nonzero exit status), prints an error, frees memory, and exits.
 */
void praseCMDS(struct parentNode *node) {

	int result;
	struct LineNode *cmdLine;
        for (cmdLine = node->cmds; cmdLine != NULL; cmdLine = cmdLine->next) {
                if (cmdLine->line[0] != 0) {
			if ((result = system(cmdLine->line)) != 0) {
				freeLL();
				fprintf(stderr, "Invalid Command was Given.\n");
				exit(1);
			}
			printf("%s\n",cmdLine->line);
			CMDSused++;
                }
        }





}



/*
 * POT(node) -- Performs a post-order traversal on the dependency graph starting from `node`.
 * Checks if a target needs to be rebuilt based on file modification timestamps and dependencies.
 * If a cycle is detected, prints an error but continues execution.
 * Executes commands for the node if necessary, updating timestamps after execution.
 * Frees memory and exits if a critical error is encountered.
 */
void POT(struct parentNode *node) {
	if (node->visited) {
		return;
	}

	node->visited = 1;

	struct stat fileData;
	if (stat(node->name, &fileData) == 0) {
		node->fileDate = fileData.st_mtime;
		node->doesExist = 1; //both of these values set by calling stat()
	} else {
		if (!node->target) {
			perror("stat");
			freeLL();
			exit(1);
		} else {
			node->mustBuild = 1;
		}
	}

	if (!node->doesExist) {
		if (node->children == NULL) {
			fprintf(stderr, "Node Does not Exist.\n");
			freeLL();
			exit(1);
		} else {
			node->mustBuild = 1;
		}
	}

	struct childNode *childrenEdges;
	for (childrenEdges = node->children; childrenEdges != NULL; childrenEdges = childrenEdges->next) {
		POT(childrenEdges->to);
		// CYCLE FOUND
		if (childrenEdges->to->visited && (!childrenEdges->to->completed)) {
			fprintf(stderr, "Cycle has been Found. This is a Acyclic Grpah.\n");
			continue;
		} else if (!node->mustBuild) {
			if ((!childrenEdges->to->doesExist) || (childrenEdges->to->fileDate > node->fileDate)) {
				node->mustBuild = 1;
			}
		}
	}

	if (node->mustBuild) {
		praseCMDS(node);
        	if (stat(node->name, &fileData) == 0) {
                	node->fileDate = fileData.st_mtime;
                	node->doesExist = 1; //both of these values set by calling stat()
        	} else {
        		if (!node->target) {
                        	perror("stat");
                        	freeLL();
                        	exit(1);
			}
		}

	}
	node->completed = 1;
}


/*
 * resetVisited() -- Resets the `visited` status of all parent nodes in the dependency graph.
 * Iterates through `headLL` and sets the `visited` field of each node to 0. It assumes that
 * headLL is either set to NULL or with a node.
 */
void resetVisited() {

	struct parentNode *curP = headLL;

	while (curP != NULL) {
		curP->visited = 0;
		curP = curP->next;
	}
}




/*
 * parseTargetFileData(fp) -- Parses a file representing a target node and executes its commands.
 * Takes a filename (`char *fp`), locates the corresponding parent node in `headLL`, and processes 
 * its dependencies using a post-order traversal. If the target is not found, prints an error 
 * and terminates the program. Frees memory after processing to avoid leaks.
 */
int parseTargetFileData(char *fp) {

	struct parentNode *target = NULL;

	char *line = strdup(fp);

	target = verifyTargetNode(line);

	if (target == NULL) {
		fprintf(stderr, "Target Was not Found and or Does not Exist.\n");
		exit(1);
	}
	POT(target);
	resetVisited();
	freeLL();
	if (!CMDSused) {
                printf("%s is up to date.\n", line);
        }
	free(line);
	return 0;
}
