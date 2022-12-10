#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctype.h>
#include "parser.h"
#include "tree.h"
#include "statsem.h"

using namespace std;


/* Global variables */
int tokenID;
vector<vector<string>> tokens;
bool lookedAhead = false;
string fileName;


// Iterate over scanner depending on if we already looked ahead
void nextToken() {
	if (lookedAhead == false) {
		tokenID++;
	} else {
		lookedAhead = false;
	}
}

// Create a nonterminal token
token* nonterminal(string myStr) {
	token* tk = new Token();
	tk->instance = myStr;
	tk->tokenType = "nonterminal";
	return tk;
}

// Create a terminal token
token* terminal(vector<string> myVector) {
	token* tk = new Token();
	tk->tokenType = myVector[0];
	tk->instance = myVector[1];
	tk->lineNum = stoi(myVector[2]);
	tk->characterNum = stoi(myVector[3]);
	return tk;
}

// Create relational operator terminal token
token* bracketedEqualTerminal(vector<string> myVector) {
	token* tk = new Token();
	tk->tokenType = "operator";
	tk->instance = "[=]";
	tk->lineNum = stoi(myVector[2]);
	tk->characterNum = stoi(myVector[3]);
	return tk;
}

// Print a parser error and exit
void parserError(string message) {
	int line = stoi(tokens[tokenID][2]);

	// Get error line
	ifstream file;
	file.open(fileName);
	if (file.fail()) {
		cout << "File failed to open." << endl;
		exit(0);
	}
	int currentLineNumber = 0;
	string currentLine;
	while (!file.eof()) {
		currentLineNumber++;
		getline(file, currentLine);
		if (currentLineNumber == line) break;
	}
	if (currentLineNumber < line) {
		cout << "Line not found!" << endl;
		exit(0);
	}
	file.close();
	
	// Output error
	cout << "Parser Error: " << message << endl;
	cout << "   ^ On line number " << line << ": " << currentLine << endl;
	exit(0);
}

// BNF: <program> -> <vars> program <block>
void parser(vector<vector<string>>& passedTokens, string file) {
	tokenID = 0;
	tokens = passedTokens;
	fileName = file;
	
	node* tree = createTree(nonterminal("<program>"));
	addSubtree(tree, vars());
	if (tokens[tokenID][0] != "program") parserError("Expected 'program' keyword. Received '" + tokens[tokenID][1] + "'.");
	nextToken();
	addSubtree(tree, block());
	
	//preorderTraversal(tree, 0);
	
	/* Start static semantics (P3) */
	statSemantics(tree, fileName);
}

// BNF: <vars> -> empty | whole Identifier := Integer ; <vars>
node* vars() {
	node* tree = createTree(nonterminal("<vars>"));
	if (tokens[tokenID][0] != "whole") {
		tree = insertNode(tree, nonterminal("Empty"));
		return tree;
	}
	nextToken();	
	if (tokens[tokenID][0] != "identifier") parserError("Expected identifier after 'whole' keyword.");
	tree = insertNode(tree, terminal(tokens[tokenID]));
	nextToken();
	if (tokens[tokenID][1] != ":=") parserError("Expected ':=' for variable initialization.");
	nextToken();
	if (tokens[tokenID][0] != "integer") parserError("Expected integer value for variable initialization.");
	tree = insertNode(tree, terminal(tokens[tokenID]));
	nextToken();
	if (tokens[tokenID][1] != ";") parserError("Expected semicolon after variable initialization.");	
	nextToken();
	addSubtree(tree, vars());
	return tree;
}

// BNF: <block> -> begin <vars> <stats> end
node* block() {
	node* tree = createTree(nonterminal("<block>"));
	if (!(tokens[tokenID][0] == "begin")) parserError("Expected 'begin' keyword. Received '" + tokens[tokenID][1] + "'.");
	nextToken();
	addSubtree(tree, vars());
	addSubtree(tree, stats());
	if (!(tokens[tokenID][0] == "end")) parserError("Expected 'end' keyword. Received '" + tokens[tokenID][1] + "'.");
	nextToken();	
	return tree;
}

// BNF: <expr> -> <N> - <expr> | <N>
node* expr() {
	node* tree = createTree(nonterminal("<expr>"));
	addSubtree(tree, N());
	if (tokens[tokenID][1] == "-") {
		nextToken();
		addSubtree(tree, expr());
	}
	return tree;
}

// BNF: <N> -> <A> + <N> | <A> * <N> | <A>
node* N() {
	node* tree = createTree(nonterminal("<N>"));
	addSubtree(tree, A());
	if (tokens[tokenID][1] == "+" || tokens[tokenID][1] == "*") {
		tree = insertNode(tree, terminal(tokens[tokenID]));
		nextToken();
		addSubtree(tree, N());
	}
	return tree;
}

// BNF: <A> -> <M> <A2>
node* A() {
	node* tree = createTree(nonterminal("<A>"));
	addSubtree(tree, M());
	addSubtree(tree, A2());
	return tree;
}

// BNF: <A2> -> / <M> <A2> | empty
node* A2() {
	node* tree = createTree(nonterminal("<A2>"));
	if (tokens[tokenID][1] != "/") {
		tree = insertNode(tree, nonterminal("Empty"));
		return tree;
	}
	nextToken();
	addSubtree(tree, M());
	addSubtree(tree, A2());
	return tree;
}

// BNF: <M> -> :<M> | <R>
node* M() {
	node* tree = createTree(nonterminal("<M>"));
	if (tokens[tokenID][1] == ":") {
		nextToken();
		addSubtree(tree, M());
	} else addSubtree(tree, R());
	return tree;
}

// BNF: <R> -> ( <expr> ) | Identifier | Integer
node* R() {
	node* tree = createTree(nonterminal("<R>"));
	if (tokens[tokenID][0] == "identifier") {
		tree = insertNode(tree, terminal(tokens[tokenID]));
	} else if (tokens[tokenID][0] == "integer") {
		tree = insertNode(tree, terminal(tokens[tokenID]));
	} else if (tokens[tokenID][1] == "(") {
		addSubtree(tree, expr());
		if (tokens[tokenID][1] != ")") {
			parserError("Expected ')'. Received '" + tokens[tokenID][1] + "'.");
		}
	} else parserError("Expected '(', identifier, or integer. Received '" + tokens[tokenID][1] + "'.");
	nextToken();
	return tree;
}

// BNF: <stats> -> <stat> <mStat>
node* stats() {
	node* tree = createTree(nonterminal("<stats>"));
	addSubtree(tree, stat());
	addSubtree(tree, mStat());
	return tree;
}

// BNF: <mStat> -> empty | <stat> <mStat>
node* mStat() {
	node* tree = createTree(nonterminal("<mStat>"));	
	string options[] = {"input", "output", "begin", "if", "while", "assign", "warp", "label"};
	int size = sizeof(options)/sizeof(options[0]);
	bool found = false;
	for (int i = 0; i < size; i++) {
		if (options[i] == tokens[tokenID][0]) {
			found = true;
		}
	}
	if (found == false) {
		tree = insertNode(tree, nonterminal("Empty"));
		return tree;
	}
	addSubtree(tree, stat());
	addSubtree(tree, mStat());
	return tree;
}

// BNF: <stat> -> <in>; | <out>; | <block> | <if>; | <loop>; | <assign>; | <goto>; | <label>;
node* stat() {
	node* tree = createTree(nonterminal("<stat>"));
	string instance = tokens[tokenID][1];
	if (instance == "input") addSubtree(tree, in());
	else if (instance == "output") addSubtree(tree, out());
	else if (instance == "begin") {
		addSubtree(tree, block());
		return tree;
	} else if (instance == "if") addSubtree(tree, _if());
	else if (instance == "while") addSubtree(tree, loop());
	else if (instance == "assign") addSubtree(tree, assign());
	else if (instance == "warp") addSubtree(tree, _goto());
	else if (instance == "label") addSubtree(tree, label());
	else parserError("No valid option for 'stat'");
	if (tokens[tokenID][1] != ";") parserError("Expected ';' operator. Received '" + tokens[tokenID][1] + "'.");
	nextToken();
	return tree;
}

// BNF: <in> -> input Identifier
node* in() {
	node* tree = createTree(nonterminal("<in>"));
	if (tokens[tokenID][0] != "input") parserError("Expected 'input' keyword.");	
	nextToken();
	if (tokens[tokenID][0] != "identifier") parserError("Expected identifier after 'input' keyword.");
	tree = insertNode(tree, terminal(tokens[tokenID]));
	nextToken();
	return tree;
}

// BNF: <out> -> output <expr>
node* out() {
	node* tree = createTree(nonterminal("<out>"));
	if (tokens[tokenID][0] != "output") parserError("Expected 'output' keyword.");	
	nextToken();
	addSubtree(tree, expr());
	return tree;
}

// BNF: <if> -> if [ <expr> <RO> <expr> ] then <stat> | if [ <expr> <RO> <expr> ] then <stat> pick <stat>
node* _if() {
	node* tree = createTree(nonterminal("<if>"));
	if (tokens[tokenID][0] != "if") parserError("Expected 'if' keyword.");
	nextToken();
	if (tokens[tokenID][1] != "[") parserError("Expected '[' after 'if' keyword.");
	nextToken();
	addSubtree(tree, expr());
	addSubtree(tree, RO());
	addSubtree(tree, expr());
	if (tokens[tokenID][1] != "]") parserError("Expected ']' to close if conditional.");
	nextToken();
	if (tokens[tokenID][0] != "then") parserError("Expected 'then' after if conditional.");
	nextToken();
	addSubtree(tree, stat());
	if (tokens[tokenID][0] == "pick") {
		nextToken();
		addSubtree(tree, stat());
	}
	return tree;
}

// BNF: <loop> -> while [ <expr> <RO> <expr> ] <stat>
node* loop() {
	node* tree = createTree(nonterminal("<loop>"));
	if (tokens[tokenID][0] != "while") parserError("Expected 'while' keyword.");
	nextToken();
	if (tokens[tokenID][1] != "[") parserError("Expected '[' after 'while' keyword.");
	nextToken();
	addSubtree(tree, expr());
	addSubtree(tree, RO());
	addSubtree(tree, expr());
	if (tokens[tokenID][1] != "]") parserError("Expected ']' to close while conditional.");
	nextToken();
	addSubtree(tree, stat());
	return tree;
}

// BNF: <assign> -> assign Identifier = <expr>
node* assign() {
	node* tree = createTree(nonterminal("<assign>"));
	if (tokens[tokenID][0] != "assign") parserError("Expected 'assign' keyword.");
	nextToken();
	if (tokens[tokenID][0] != "identifier") parserError("Expected identifier after 'assign' keyword.");
	tree = insertNode(tree, terminal(tokens[tokenID]));
	nextToken();
	if (tokens[tokenID][1] != "=") parserError("Expected '=' for assign. Received '" + tokens[tokenID][1] + "'.");
	nextToken();
	addSubtree(tree, expr());
	return tree;
}

// BNF: <RO> -> > | < | == | [=] | !=
node* RO() {
	node* tree = createTree(nonterminal("<RO>"));
	string instance = tokens[tokenID][1];
	vector<string> orgToken = tokens[tokenID];
	if (instance == ">" || instance == "<" || instance == "==" || instance == "!=") {
		tree = insertNode(tree, terminal(tokens[tokenID]));
	} else if (instance == "[") {
		nextToken();
		if (tokens[tokenID][1] != "=") parserError("Expected relational operator. Received '['.");
		nextToken();
		if (tokens[tokenID][1] != "]") parserError("Expected relational operator. Received '[='.");
		tree = insertNode(tree, bracketedEqualTerminal(orgToken));
	} else parserError("Expected relational operator. Received '" + instance + "'.");
	nextToken();
	return tree;
}

// BNF: <label> -> label Identifier
node* label() {
	node* tree = createTree(nonterminal("<label>"));
	if (tokens[tokenID][0] != "label") parserError("Expected 'label' keyword.");
	nextToken();
	if (tokens[tokenID][0] != "identifier") parserError("Expected identifier after 'label' keyword.");
	tree = insertNode(tree, terminal(tokens[tokenID]));
	nextToken();
	return tree;
}

// BNF: <goto> -> warp Identifier
node* _goto() {
	node* tree = createTree(nonterminal("<goto>"));
	if (tokens[tokenID][0] != "warp") parserError("Expected 'warp' keyword.");	
	nextToken();
	if (tokens[tokenID][0] != "identifier") parserError("Expected identifier after 'warp' keyword.");
	tree = insertNode(tree, terminal(tokens[tokenID]));	
	nextToken();
	return tree;
}
