#ifndef _NODE_H_
#define _NODE_H_

// #include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enum.h"

// #define NAME_LENGTH 32
// #define VAL_LENGTH 64

#define TRUE 1
#define FALSE 0

// typedef uint_32 bool;

// node type declared
typedef struct node {
    int lineNo;  //  node in which line
    //   int depth;   //  node depth, for count white space for print
    NodeType type;  // node type
    char* name;     //  node name
    char* val;      //  node value

    struct node* child;  //  non-terminals node first child node
    struct node* next;   //  non-terminals node next brother node

} Node;

typedef unsigned boolean;
typedef Node* pNode;

static inline char* newString(char* src) {
    if (src == NULL) return NULL;
    int length = strlen(src) + 1;
    char* p = (char*)malloc(sizeof(char) * length);
    assert(p != NULL);
    strncpy(p, src, length);
    return p;
}

static inline pNode newNode(int lineNo, NodeType type, char* name, int argc,
                            ...) {
    pNode curNode = NULL;
    // int nameLength = strlen(name) + 1;

    curNode = (pNode)malloc(sizeof(Node));

    assert(curNode != NULL);

    // curNode->name = (char*)malloc(sizeof(char) * NAME_LENGTH);
    // curNode->val = (char*)malloc(sizeof(char) * VAL_LENGTH);
    // curNode->name = (char*)malloc(sizeof(char) * nameLength);
    // assert(curNode->name != NULL);
    // // assert(curNode->val != NULL);

    curNode->lineNo = lineNo;
    curNode->type = type;
    curNode->name = newString(name);
    // strncpy(curNode->name, name, nameLength);

    va_list vaList;
    va_start(vaList, argc);

    pNode tempNode = va_arg(vaList, pNode);

    curNode->child = tempNode;

    for (int i = 1; i < argc; i++) {
        tempNode->next = va_arg(vaList, pNode);
        if (tempNode->next != NULL) {
            tempNode = tempNode->next;
        }
    }

    va_end(vaList);
    return curNode;
}

static inline pNode newTokenNode(int lineNo, NodeType type, char* tokenName,
                                 char* tokenText) {
    pNode tokenNode = (pNode)malloc(sizeof(Node));
    // int nameLength = strlen(tokenName) + 1;
    // int textLength = strlen(tokenText) + 1;

    assert(tokenNode != NULL);

    tokenNode->lineNo = lineNo;
    tokenNode->type = type;

    // tokenNode->name = (char*)malloc(sizeof(char) * nameLength);
    // tokenNode->val = (char*)malloc(sizeof(char) * textLength);

    // assert(tokenNode->name != NULL);
    // assert(tokenNode->val != NULL);

    // strncpy(tokenNode->name, tokenName, nameLength);
    // strncpy(tokenNode->val, tokenText, textLength);

    tokenNode->name = newString(tokenName);
    tokenNode->val = newString(tokenText);

    tokenNode->child = NULL;
    tokenNode->next = NULL;

    return tokenNode;
}

static inline void delNode(pNode* node) {
    if (node == NULL) return;
    pNode p = *node;
    while (p->child != NULL) {
        pNode temp = p->child;
        p->child = p->child->next;
        delNode(&temp);
    }
    free(p->name);
    free(p->val);
    free(p);
    p = NULL;
}

static inline void printTreeInfo(pNode curNode, int height) {
    if (curNode == NULL) {
        return;
    }

    for (int i = 0; i < height; i++) {
        printf("  ");
    }
    printf("%s", curNode->name);
    if (curNode->type == NOT_A_TOKEN) {
        printf(" (%d)", curNode->lineNo);
    } else if (curNode->type == TOKEN_TYPE || curNode->type == TOKEN_ID ||
               curNode->type == TOKEN_INT) {
        printf(": %s", curNode->val);
    } else if (curNode->type == TOKEN_FLOAT) {
        printf(": %lf", atof(curNode->val));
    }
    printf("\n");
    printTreeInfo(curNode->child, height + 1);
    printTreeInfo(curNode->next, height);
}

#endif