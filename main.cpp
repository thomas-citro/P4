#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctype.h>
#include "parser.h"
#include "tree.h"
using namespace std;


/* Global variables */
char otherSymbols[16] = {'+', '-', '*', '/', '^', '.', '(', ')', ',', '{',
						 '}', ';', '[', ']', '<', '>'};
int FSA[9][12] = {
	{1, 2, 0, 1004, 1, 3, 4, 5, 6, 7, 8, -1},
	{1, 1, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001},
	{1002, 2, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002},
	{1003, 1003, 1003, 1003, 1003, 8, 1003, 1003, 1003, 1003, 1003, 1003},
	{1003, 1003, 1003, 1003, 1003, 8, 1003, 1003, 1003, 1003, 1003, 1003},
	{1003, 1003, 1003, 1003, 1003, 8, 1003, 1003, 1003, 1003, 1003, 1003},
	{1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 8, 1003, 1003, 1003},
	{1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 8, 1003, 1003},
	{1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003}
};
string keywords[17] = {"begin", "end", "do", "while", "whole", "label",
			"return", "input", "output", "program", "warp",
			"if", "then", "pick", "declare", "assign", "func"};


int main(int argc, char* argv[]) {
	/* Declare/initialize variables */
	vector<vector<string>> tokens;
	fstream newfile;
	bool processingComment = false;
	int state = 0;
	int lineNumber = 1;
	int lineLength = 0;
	string fileName;
	
	/* Parse command-line arguments */
	if (argc == 2) {
		fileName = argv[1];
	} else {
		cout << "Usage:   ./P3 [file]" << endl;
		exit(0);
	}
	
	/* Process file line by line (Scanner) */
	newfile.open(fileName, ios::in);
	if (newfile.is_open()) {
		string currentLine;
		while(getline(newfile, currentLine)) {
			lineLength = currentLine.length();
			processLine(currentLine, tokens, processingComment, state, lineNumber);
			lineNumber++;
		}
		newfile.close();
	} else {
		cout << "ERROR - Unable to open file" << endl;
		exit(0);
	}
	vector<string> innerVector{"eof", "", to_string(lineNumber - 1), to_string(lineLength)};
	tokens.push_back(innerVector);
	//outputVector(tokens);
	
	/* Start the parser */
	parser(tokens, fileName);
}


void processLine(string line, vector<vector<string>>& tokens, bool& processingComment, int& state, int lineNumber) {
	string currentToken = "";
	string tokenType = "";
	int tokenFirstChar = 0;
	unsigned int i;
	for (i = 0; i < line.length(); i++) {
		if (line[i] == '#' && processingComment) {
			processingComment = false;
			continue;
		} else if (line[i] == '#' && !processingComment) {
			processingComment = true;
			continue;
		}
		if (processingComment) {
			continue;
		}
		processCharacter(currentToken, tokenType, tokenFirstChar, tokens, processingComment, state, lineNumber, line[i], i);
	}
	/* Process end of line */
	processCharacter(currentToken, tokenType, tokenFirstChar, tokens, processingComment, state, lineNumber, ' ', i);
}


void processCharacter(string& currentToken, string& tokenType, int& tokenFirstChar, vector<vector<string>>& tokens, bool& processingComment, int& state, int lineNumber, char currentChar, unsigned int& i) {
	int FSA_Value = FSA[state][getColumn(currentChar)];
	if (FSA_Value > 1000) {
		if (FSA_Value == 1001) {
			tokenType = "identifier";
			bool keyword = false;
			int arraySize = sizeof(keywords) / sizeof(string);
			for (int j = 0; j < arraySize; j++) {
				if (keywords[j] == currentToken) {
					keyword = true;
				}
			}
			if (keyword) {
				tokenType = currentToken;
			}
		} else if (FSA_Value == 1002) {
			tokenType = "integer";
		} else if (FSA_Value == 1003) {
			tokenType = "operator";
		}
		vector<string> innerVector{tokenType, currentToken, to_string(lineNumber), to_string(tokenFirstChar)};
		tokens.push_back(innerVector);
		currentToken = "";
		state = 0;
		i--;
	} else {
		if (!isspace(currentChar)) {
			if (currentToken.empty()) {
				tokenFirstChar = i;
			}
			currentToken += currentChar;
		}
		state = FSA_Value;
	}
}


int getColumn(char c) {
	int column;
	if (isalpha(c)) {
		column = 0;
	} else if (isdigit(c)) {
		column = 1;
	} else if (isspace(c)) {
		column = 2;
	} else if (c == '_') {
		column = 4;
	} else if (c == '=') {
		column = 5;
	} else if (c == '!') {
		column = 6;
	} else if (c == ':') {
		column = 7;
	} else if (c == '|') {
		column = 8;
	} else if (c == '&') {
		column = 9;
	} else {
		bool charFound = false;
		int arraySize = sizeof(otherSymbols) / sizeof(char);
		for (int i = 0; i < arraySize; i++) {
			if (otherSymbols[i] == c) {
				charFound = true;
			}
		}
		if (charFound) {
			column = 10;
		} else {
			cout << "Unrecognized symbol: " << c << endl;
			exit(0);
		}
	}
	return column;
}

void outputVector(vector<vector<string>>& myVector) {
	cout << "--- Vector ---" << endl;
	for (unsigned int i = 0; i < myVector.size(); i++) {\
		cout << "[" << i << "] = ['" << myVector[i][0] << "', '" << myVector[i][1] << "', '" << myVector[i][2] 
		<< "', '" << myVector[i][3] << "']" << endl;
	}
	cout << "--------------" << endl;
}

