#ifndef PARSER_H
#define PARSER_H

// Structs
typedef struct Token {
	std::string instance;
	std::string tokenType;
	int lineNum;
	int characterNum;
}token;
typedef struct Node {
	struct Token *tk;
	struct Node *first, *second, *third, *fourth, *fifth;
}node;


// Function prototypes for main/scanner
void outputVector(std::vector<std::vector<std::string>>&);
void processLine(std::string, std::vector<std::vector<std::string>>&, bool&, int&, int);
void processCharacter(std::string&, std::string&, int&, std::vector<std::vector<std::string>>&, bool&, int&, int, char, unsigned int&);
int getColumn(char);


// Function prototypes for parser
void parserError(std::string, int);
void nextToken();
token* nonterminal(std::string);
token* terminal(std::vector<std::string>);
token* bracketedEqualTerminal(std::vector<std::string>);
void parser(std::vector<std::vector<std::string>>&, std::string);
node* vars();
node* block();
node* expr();
node* N();
node* A();
node* A2();
node* M();
node* R();
node* stats();
node* mStat();
node* stat();
node* in();
node* out();
node* _if();
node* loop();
node* assign();
node* RO();
node* label();
node* _goto();

#endif
