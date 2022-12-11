#ifndef SEM_H
#define SEM_H

void statSemantics(node*, std::string);
int find(std::string);
void traverse(node*);
void processR(node*);
void processInput(node*);
void processAssign(node*);
void processVars(node*, int&);
void processLabel(node*);
void statSemanticsError(std::string, std::string, int);
void writeAssembly(std::string, std::string, std::string);
bool findGlobal(std::string);
std::string getTempName(int);
void writeTemporaries();

#endif
