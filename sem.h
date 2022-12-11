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
void processBlock(node*);
void processExpr(node*);
void processN(node*);
void processA(node*);
void processA2(node*);
void processM(node*);
void processStats(node*);
void processStat(node*);
void processMStat(node*);
void processOut(node*);
void processIf(node*);
void processLoop(node*);
void processRO(node*);
void processGoto(node*);
void statSemanticsError(std::string, std::string, int);
void writeAssembly(std::string, std::string = "N/A", std::string = "N/A");
bool findGlobal(std::string);
std::string getTempName(int);
void writeVariables();

#endif
