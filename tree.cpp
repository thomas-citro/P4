#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctype.h>
#include "parser.h"
#include "tree.h"

using namespace std;

// Create and initialize the tree with just the root node
node* createTree(token* myTk){
	node *temp = new Node();
	temp->tk = myTk;
	temp->first = temp->second = temp->third = temp->fourth = NULL;
	return temp;
}

// Add the subtree to the given node
void addSubtree(node* myNode, node* subtree){
	if (myNode->first == NULL) {
		myNode->first = subtree;
	} else if (myNode->second == NULL) {
		myNode->second = subtree;
	} else if (myNode->third == NULL) {
		myNode->third = subtree;
	} else if (myNode->fourth == NULL) {
		myNode->fourth = subtree;
	} else if (myNode->fifth == NULL) {
		myNode->fifth = subtree;
	}
}

// Add a node to the current level
node* insertNode(node* myNode, token* myTk) { 
	// If the node is empty, return a new node, end case
	if (myNode == NULL){
		return createTree(myTk);
	}
	// Copy the token
	token* temp = new Token();
	temp->instance = myTk->instance;
	temp->tokenType = myTk->tokenType;
	temp->lineNum = myTk->lineNum;
	temp->characterNum = myTk->characterNum;
	
	if (myNode->first == NULL){
		myNode->first = createTree(temp);
	} else if (myNode->second == NULL) {
		myNode->second = createTree(temp);
	} else if (myNode->third == NULL) {
		myNode->third = createTree(temp);
	} else if (myNode->fourth == NULL) {
		myNode->fourth = createTree(temp);
	} else if (myNode->fifth == NULL) {
		myNode->fifth = createTree(temp);
	}
	return myNode; 
} 

// Preorder traversal (root first, then each branch from left to right)
void preorderTraversal(node* myNode, int depth) {
	if (myNode == NULL) return;
	if (depth == 0) cout << myNode->tk->instance << endl;
	string depthStr = "";
	// Get the depth string of spaces
	for (int i = 0; i < depth + 1; i++) {
		depthStr += "   ";
	}
	
	// Print children
	if (myNode->first != NULL) {
		cout << depthStr << myNode->first->tk->instance << endl;
		preorderTraversal(myNode->first, depth + 1);
	}
	if (myNode->second != NULL) {
		cout << depthStr << myNode->second->tk->instance << endl;
		preorderTraversal(myNode->second, depth + 1);
	}
	if (myNode->third != NULL) {
		cout << depthStr << myNode->third->tk->instance << endl;
		preorderTraversal(myNode->third, depth + 1);
	}
	if (myNode->fourth != NULL) {
		cout << depthStr << myNode->fourth->tk->instance << endl;
		preorderTraversal(myNode->fourth, depth + 1);
	}
	if (myNode->fifth != NULL) {
		cout << depthStr << myNode->fifth->tk->instance << endl;
		preorderTraversal(myNode->fifth, depth + 1);
	}
}
