#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctype.h>
#include <stack>
#include "parser.h"
#include "tree.h"
#include "statsem.h"

using namespace std;

stack<string> myStack;
string fileName2;
bool readingGlobals;

void statSemantics(node* root, string file) {
	fileName2 = file;
	
	readingGlobals = true;
	traverse(root);
	cout << "No errors detected" << endl;
}

int find(string myStr) {
	stack<string> tempStack = myStack;
	int i = 0;
	int firstOccurrence = -1;
	while (!tempStack.empty()) {
		if (tempStack.top() == myStr) {
			firstOccurrence = i;
			break;
		}
		tempStack.pop();
		i++;
	}
	return firstOccurrence;
}

void traverse(node* myNode) {
	int varsCount = 0;
	
	// Iterate through each child
	for (int i = 1; i < 6; i++) {
		node* currentChild;
		if (i == 1) {
			currentChild = myNode->first;
		} else if (i == 2) {
			currentChild = myNode->second;
		} else if (i == 3) {
			currentChild = myNode->third;
		} else if (i == 4) {
			currentChild = myNode->fourth;
		} else {
			currentChild = myNode->fifth;
		}
		if (currentChild == NULL) continue;
		
		if (currentChild->tk->instance == "<vars>") {
			processVars(currentChild, varsCount);
		} else if (currentChild->tk->instance == "<assign>") {
			processAssign(currentChild);
		} else if (currentChild->tk->instance == "<in>") {
			processInput(currentChild);
		} else if (currentChild->tk->instance == "<R>") {
			processR(currentChild);
		} else if (currentChild->tk->instance != "Empty") {
			traverse(currentChild);
		}
	}
	if (varsCount != 0) {
		while (varsCount > 0) {
			myStack.pop();
			varsCount--;
		}
	}
}

void processR(node* myNode) {
	if (myNode->first->tk->tokenType == "identifier") {
		int found = find(myNode->first->tk->instance);
		if (found == -1) {
			statSemanticsError("Unknown variable", myNode->first->tk->instance, myNode->first->tk->lineNum);
		}
	} else if (myNode->first->tk->instance == "<expr>") {
		traverse(myNode->first);
	}
}

void processInput(node* myNode) {
	int found = find(myNode->first->tk->instance);
	if (found == -1) {
		statSemanticsError("Input unknown variable", myNode->first->tk->instance, myNode->first->tk->lineNum);
	}
}

void processAssign(node* myNode) {
	int found = find(myNode->first->tk->instance);
	if (found == -1) {
		statSemanticsError("Assigning unknown variable", myNode->first->tk->instance, myNode->first->tk->lineNum);
	}
	traverse(myNode->second);
}

void processVars(node* myNode, int& varsCount) {
	node* currentNode = myNode;
	node* identNode;
	while(true) {
		if (currentNode->tk->instance == "<vars>") {
			identNode = currentNode->first;
		} else {
			// This shouldn't happen
			cout << "Unexpected error" << endl;
		}
		if (identNode->tk->instance == "Empty") {
			break;
		} else {
			int found = find(identNode->tk->instance);
			if (found != -1) {
				statSemanticsError("Duplicate variable name", identNode->tk->instance, identNode->tk->lineNum);
			} else {
				varsCount++;
				myStack.push(identNode->tk->instance);
				if (readingGlobals) varsCount--; // To keep globals on the stack
				currentNode = currentNode->third;
			}
		}
	}
	if (readingGlobals) {
		readingGlobals = false;
	}
}

void statSemanticsError(string error, string variable, int lineNumber) {
	/***** Possible Errors *****/
	// "Duplicate variable name"
	// "Unknown variable"
	// "Assigning unknown variable"
	// "Input unknown variable"
	
	// Get error line
	ifstream file;
	file.open(fileName2);
	if (file.fail()) {
		cout << "File failed to open." << endl;
		exit(0);
	}
	int currentLineNumber = 0;
	string currentLine;
	while (!file.eof()) {
		currentLineNumber++;
		getline(file, currentLine);
		if (currentLineNumber == lineNumber) break;
	}
	if (currentLineNumber < lineNumber) {
		cout << "Line not found!" << endl;
		exit(0);
	}
	file.close();
	
	// Output error
	string message;
	if (error == "Duplicate variable name") {
		message = "Variable with the name '" + variable + "' already defined in this scope.";
	} else if (error == "Unknown variable") {
		message = "Variable with the name '" + variable + "' is not defined in this scope.";
	} else if (error == "Assigning unknown variable") {
		message = "Attempting to assign undeclared variable with the name '" + variable + "'";
	} else if (error == "Input unknown variable") {
		message = "Attempting to retrieve input into undeclared variable with the name '" + variable + "'";
	}
	cout << "Static Semantics Error: " << message << endl;
	cout << "   ^ On line number " << lineNumber << ": " << currentLine << endl;
	exit(0);
}
