#ifndef TREE_H
#define TREE_H

// Function prototypes for tree
node* createTree(token*);
node* insertNode(node*, token*);
void addSubtree(node*, node*);
void preorderTraversal(node*, int);

#endif
