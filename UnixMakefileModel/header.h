/*
* File: header.h
* Author: Chance Krueger
* Purpose: Defines the structures and function prototypes for
* a graph-based makefile parser, including parent nodes,
* child nodes, and command line nodes.
*/


// header.h
#ifndef HEADER_H
#define HEADER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

// Forward declarations
struct childNode;
struct parentNode;
struct LineNode;

/*
 * struct LineNode -- Represents a single command in a target's command list.
 * Stores a command string (`line`) and a pointer to the next command (`next`).
 */
struct LineNode {
    	char *line;
    	struct LineNode *next;
};

/*
 * struct parentNode -- Represents a target in the dependency graph.
 * Stores:
 *   - `name`: The name of the target.
 *   - `cmds`: A linked list of commands to execute for this target.
 *   - `children`: A linked list of dependencies (edges to child nodes).
 *   - `next`: Pointer to the next target in the list.
 *   - `visited`: Flag for traversal to detect cycles.
 *   - `doesExist`: Flag indicating whether the target file exists.
 *   - `mustBuild`: Flag indicating if this target needs to be rebuilt.
 *   - `completed`: Flag marking if traversal for this node is finished.
 *   - `fileDate`: Timestamp of the last modification (from `stat()`).
 *   - `target`: Flag indicating if this node is a target in the makefile.
 */
struct parentNode {
	
	char *name;
	struct LineNode *cmds;
	struct childNode *children;
	struct parentNode *next;
	int visited;
	int doesExist;
	int mustBuild;
	int completed;
	time_t fileDate;
	int target;

};


/*
 * struct childNode -- Represents an edge in the dependency graph.
 * Stores:
 *   - `to`: Pointer to the dependent `parentNode`.
 *   - `next`: Pointer to the next dependency in the list.
 */
struct childNode {

	struct parentNode *to;
	struct childNode *next;

};


// Global Varible
extern struct parentNode *headLL;
extern char *targetNode;
extern char *curLine;


// Function prototypes
int parseMakeFileData(FILE *fp); // return 1 if errSeen, 0 if not. If fatal error, exit program and return 1;
void addNodeToGraph(struct parentNode *node);

int parseTargetFileData(char *fp); // return 1 if errSeen, 0 if not. If fatal error, exit program and return 1;
void PostOrder(struct parentNode *node); // PRINT TREE IN POSTORDER (target from parseTargetFileData)
struct parentNode *findParentNode(char *name);
void freeLines(struct LineNode *head);
void freeLL();

#endif