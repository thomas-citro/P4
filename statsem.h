#ifndef STATSEM_H
#define STATSEM_H

void statSemantics(node*, std::string);
int find(std::string);
void traverse(node*);
void processR(node*);
void processInput(node*);
void processAssign(node*);
void processVars(node*, int&);
void statSemanticsError(std::string, std::string, int);

#endif
