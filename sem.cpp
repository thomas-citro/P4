#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctype.h>
#include <stack>
#include "parser.h"
#include "tree.h"
#include "sem.h"

using namespace std;

stack<string> myStack;
vector<string> globals;
string fileName2;
string assemblyFileName;
ofstream assemblyFile;
bool readingGlobals;
int numTemporaries = 0;
int numLabels = 0;


void statSemantics(node* root, string file) {
	fileName2 = file;
	assemblyFileName = fileName2 + ".asm";
	assemblyFile.open(assemblyFileName, ios::out | ios::trunc ); // Creates file and clears contents if already exists
	assemblyFile.close();

	readingGlobals = true;
	traverse(root);
	writeAssembly("STOP");
	writeVariables();
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


void writeAssembly(string statement, string arg1, string arg2) {
	assemblyFile.open(assemblyFileName, ios::app);

	if (arg1 == "N/A") {
		assemblyFile << statement << endl;
	} else if (arg2 == "N/A") {
		assemblyFile << statement << " " << arg1 << endl;
	} else {
		assemblyFile << statement << " " << arg1 << " " << arg2 << endl;
	}
	
	assemblyFile.close();
}


bool findGlobal(string myStr) {
	for (unsigned int i = 0; i < globals.size(); i++) {
		if (globals.at(i) == myStr) {
			return true;
		}
	}
	return false;
}


string getTempName() {
	numTemporaries++;
	return "T" + to_string(numTemporaries);
}


string getLabelName() {
	numLabels++;
	return "L" + to_string(numLabels);
}


void writeVariables() {
	for (unsigned int i = 0; i < globals.size(); i++) {
		writeAssembly(globals.at(i), "0");
	}
	for (int i = 1; i <= numTemporaries; i++) {
		writeAssembly("T" + to_string(i), "0");
	}
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
		} else {
			checkNode(currentChild);
		}
	}
	if (varsCount != 0) {
		while (varsCount > 0) {
			myStack.pop();
			writeAssembly("POP");
			varsCount--;
		}
	}
}

void checkNode(node* myNode) {	
	string inst = myNode->tk->instance;
	if (inst == "<assign>") {
		processAssign(myNode);
	} else if (inst == "<in>") {
		processInput(myNode);
	} else if (inst == "<R>") {
		processR(myNode);
	} else if (inst == "<label>") {
		processLabel(myNode);
	} else if (inst == "<block>") {
		processBlock(myNode);
	} else if (inst == "<expr>") {
		processExpr(myNode);
	} else if (inst == "<N>") {
		processN(myNode);
	} else if (inst == "<A>") {
		processA(myNode);
	} else if (inst == "<A2>") {
		processA2(myNode);
	} else if (inst == "<M>") {
		processM(myNode);
	} else if (inst == "<stats>") {
		processStats(myNode);
	} else if (inst == "<stat>") {
		processStat(myNode);
	} else if (inst == "<mStat>") {
		processMStat(myNode);
	} else if (inst == "<out>") {
		processOut(myNode);
	} else if (inst == "<if>") {
		processIf(myNode);
	} else if (inst == "<loop>") {
		processLoop(myNode);
	} else if (inst == "<RO>") {
		processRO(myNode);
	} else if (inst == "<goto>") {
		processGoto(myNode);
	} else if (myNode->tk->instance != "Empty") {
		cout << "Unexpected error: no processor for nonempty node (" << myNode->tk->instance << ")" << endl;
		exit(0);
	}	
}


// <label> -> label Identifier
void processLabel(node* myNode) {
	writeAssembly(myNode->first->tk->instance + ":");
}


// <R> -> ( <expr> ) | Identifier | Integer
void processR(node* myNode) {
	if (myNode->first->tk->tokenType == "identifier") {
		int found = find(myNode->first->tk->instance);
		if (found == -1) {
			statSemanticsError("Unknown variable", myNode->first->tk->instance, myNode->first->tk->lineNum);
		}
		bool isGlobal = findGlobal(myNode->first->tk->instance);
		if (isGlobal) {
			writeAssembly("LOAD", myNode->first->tk->instance);
		} else {
			writeAssembly("STACKR", to_string(found));
		}
	} else if (myNode->first->tk->instance == "<expr>") {
		checkNode(myNode->first);
		return;
	} else {
		writeAssembly("LOAD", myNode->first->tk->instance);
	}
}


// <in> -> input Identifier
void processInput(node* myNode) {
	int found = find(myNode->first->tk->instance);
	if (found == -1) {
		statSemanticsError("Input unknown variable", myNode->first->tk->instance, myNode->first->tk->lineNum);
	}
	bool isGlobal = findGlobal(myNode->first->tk->instance);
	if (isGlobal) {
		writeAssembly("READ", myNode->first->tk->instance);
	} else {
		string myTemp = getTempName();
		writeAssembly("READ", myTemp);
		writeAssembly("LOAD", myTemp);
		writeAssembly("STACKW", to_string(found));
	}
}


// <assign> -> assign Identifier = <expr>
void processAssign(node* myNode) {
	int found = find(myNode->first->tk->instance);
	if (found == -1) {
		statSemanticsError("Assigning unknown variable", myNode->first->tk->instance, myNode->first->tk->lineNum);
	}
	bool isGlobal = findGlobal(myNode->first->tk->instance);
	checkNode(myNode->second);
	if (isGlobal) {
		writeAssembly("STORE", myNode->first->tk->instance);
	} else {
		writeAssembly("STACKW", to_string(found));
	}
}


// <vars> -> empty | whole Identifier := Integer; <vars>
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
				writeAssembly("LOAD", currentNode->second->tk->instance);
				if (readingGlobals) {
					writeAssembly("STORE", identNode->tk->instance);
					varsCount--; // To keep globals on the stack
					globals.push_back(identNode->tk->instance);
				} else {
					writeAssembly("PUSH");
				}
				currentNode = currentNode->third;
			}
		}
	}
	if (readingGlobals) {
		readingGlobals = false;
	}
}


// <block> -> begin <vars> <stats> end
void processBlock(node* myNode) {
	traverse(myNode);
}


// <expr> -> <N> - <expr> | <N>
void processExpr(node* myNode, bool prevSubtraction) {
	if (myNode->second != NULL) {
		string myTemp = getTempName();
		processExpr(myNode->second, true);
		writeAssembly("STORE", myTemp);
		checkNode(myNode->first);
		if (prevSubtraction) {
			// To ensure associativity rules apply
			writeAssembly("ADD", myTemp);
		} else {
			writeAssembly("SUB", myTemp);
		}
	} else {
		checkNode(myNode->first);
	}
}


// <N> -> <A> + <N> | <A> * <N> | <A>
void processN(node* myNode) {
	if (myNode->third != NULL) {
		string myTemp = getTempName();
		checkNode(myNode->third);
		writeAssembly("STORE", myTemp);
		checkNode(myNode->first);
		if (myNode->second->tk->instance == "+") {
			writeAssembly("ADD", myTemp);
		} else {
			writeAssembly("MULT", myTemp);
		}
	} else {
		checkNode(myNode->first);
	}
}


// <A> -> <M> <A2>
void processA(node* myNode) {
	checkNode(myNode->first);
	string myTemp = getTempName();
	writeAssembly("STORE", myTemp);
	checkNode(myNode->second);
}


// <A2> -> / <M> <A2> | Empty
void processA2(node* myNode) {
	if (myNode->first->tk->instance == "Empty") {
		return;
	} else {
		// <prevM>
		string prevTemp = "T" + to_string(numTemporaries);
		
		// <M>
		checkNode(myNode->first);
		string myTemp = getTempName();
		writeAssembly("STORE", myTemp);
		
		// <prevM> / <M>
		writeAssembly("LOAD", prevTemp);
		writeAssembly("DIV", myTemp);
		
		// <A2>
		checkNode(myNode->second);
	}
}


// <M> -> :<M> | <R>
void processM(node* myNode) {
	if (myNode->first->tk->instance == "<M>") {
		// Negation operator
		string myTemp = getTempName();
		checkNode(myNode->first);
		writeAssembly("STORE", myTemp);
		writeAssembly("LOAD", "0");
		writeAssembly("SUB", myTemp);
	} else {
		checkNode(myNode->first);
	}
}


// <stats> -> <stat> <mStat>
void processStats(node* myNode) {
	traverse(myNode);
}


// <stat> -> <in>; | <out>; | <block> | <if>; | <loop>; | <assign>; | <goto>; | <label>;
void processStat(node* myNode) {
	traverse(myNode);
}


// <mStat> -> empty | <stat> <mStat>
void processMStat(node* myNode) {
	if (myNode->first->tk->instance == "Empty") return;
	traverse(myNode);
}


// <out> -> output <expr>
void processOut(node* myNode) {
	string myTemp = getTempName();
	checkNode(myNode->first);
	writeAssembly("STORE", myTemp);
	writeAssembly("WRITE", myTemp);
}


// <if> -> if [ <expr> <RO> <expr> ] then <stat>
// | if [ <expr> <RO> <expr> ] then <stat> pick <stat>
void processIf(node* myNode) {
	string myLabel = getLabelName();
	string myTemp = getTempName();
	checkNode(myNode->first);
	writeAssembly("STORE", myTemp);
	checkNode(myNode->third);
	
	if (myNode->second->first->tk->instance == "[=]") {
		string myTemp2 = getTempName();
		writeAssembly("STORE", myTemp2);
		
		string posLabel = getLabelName();
		writeAssembly("LOAD", myTemp);
		writeAssembly("BRZPOS", posLabel);
		writeAssembly("LOAD", myTemp2);
		writeAssembly("BRZPOS", myLabel);
		writeAssembly(posLabel + ":", "NOOP");
		writeAssembly("LOAD", myTemp2);
		writeAssembly("BRNEG", myLabel);
	} else {
		writeAssembly("SUB", myTemp);
	}
	checkNode(myNode->second);
	checkNode(myNode->fourth);
	
	if (myNode->fifth != NULL) {
		// Else statement exists
		string myLabel2 = getLabelName();
		writeAssembly("BR", myLabel2);
		writeAssembly(myLabel + ":", "NOOP");
		checkNode(myNode->fifth);
		writeAssembly(myLabel2 + ":", "NOOP");
	} else {
		// No else statement included
		writeAssembly(myLabel + ":", "NOOP");
	}
}


// <loop> -> while [ <expr> <RO> <expr> ] <stat>
void processLoop(node* myNode) {
	string startLabel = getLabelName();
	writeAssembly(startLabel + ":", "NOOP");
	string exitLabel = getLabelName();
	string myTemp = getTempName();
	checkNode(myNode->first);
	writeAssembly("STORE", myTemp);
	checkNode(myNode->third);
	
	if (myNode->second->first->tk->instance == "[=]") {
		string myTemp2 = getTempName();
		writeAssembly("STORE", myTemp2);
		string posLabel = getLabelName();
		writeAssembly("LOAD", myTemp);
		writeAssembly("BRZPOS", posLabel);
		writeAssembly("LOAD", myTemp2);
		writeAssembly("BRZPOS", exitLabel);
		writeAssembly(posLabel + ":", "NOOP");
		writeAssembly("LOAD", myTemp2);
		writeAssembly("BRNEG", exitLabel);
	} else {
		writeAssembly("SUB", myTemp);
	}
	checkNode(myNode->second);
	checkNode(myNode->fourth);
	writeAssembly("BR", startLabel); // Repeat loop
	writeAssembly(exitLabel + ":", "NOOP");
}


// <RO> -> > | < | == | [=] | !=
void processRO(node* myNode) {
	string oper = myNode->first->tk->instance;
	if (oper == ">") {
		writeAssembly("BRNEG", "L" + to_string(numLabels));
	} else if (oper == "<") {
		writeAssembly("BRPOS", "L" + to_string(numLabels));
	} else if (oper == "==") {
		writeAssembly("BRPOS", "L" + to_string(numLabels));
		writeAssembly("BRNEG", "L" + to_string(numLabels));
	} else if (oper == "[=]") {
		// Already handled in processIf function
	} else if (oper == "!=") {
		writeAssembly("BRZERO", "L" + to_string(numLabels));
	} else {
		cout << "Unexpected error" << endl;
		exit(0);
	}
}


// <goto> -> warp Identifier
void processGoto(node* myNode) {
	writeAssembly("BR", myNode->first->tk->instance);
	
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
