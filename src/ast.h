#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <vector>
#include <algorithm> 
using namespace std;
typedef enum element_Type{
    TOKEN,
    NOT_TOKEN
} Element_Type;

typedef struct node{
    Element_Type type;
    int line_number;
    char * name;
    char * value;
    struct node * child; // first child
    struct node * next; // next sibling
} Node;

typedef Node* pNode;


static inline pNode newNode(int line_number, Element_Type type, const char* name, int argc, ...){
    pNode new_element = (pNode)malloc(sizeof(Node));
    new_element->line_number = line_number;
    new_element->type = type;
    int nameLength = strlen(name) + 1;
    new_element->name = (char*)malloc(sizeof(char)*nameLength);
    strcpy(new_element->name , name);
    va_list nodeList;
    va_start(nodeList, argc);
    pNode tempPtr = va_arg(nodeList, pNode);
    new_element->child = tempPtr;
    for(int i = 0;i<argc-1; i++){
        tempPtr->next = va_arg(nodeList, pNode);
        tempPtr = tempPtr->next;
    }
    va_end(nodeList);
    return new_element;
}

static inline pNode newTokenNode(int line_number, Element_Type type,const char* name, char* text){
    pNode new_element = (pNode)malloc(sizeof(Node));
    new_element->line_number = line_number;
    new_element->type = type;
    int textLength = strlen(text) + 1;
    int nameLength = strlen(name) + 1;
    new_element->name = (char*)malloc(sizeof(char)*nameLength);
    strcpy(new_element->name , name);
    new_element->value = (char*)malloc(sizeof(char)*textLength);
    strcpy(new_element->value, text);
    new_element->child = NULL;
    new_element->next = NULL;
    return new_element;
}

static inline void printTreeInfo(pNode root, int depth){
    if(root == NULL) return;
    for(int i=0;i<depth;i++){
        printf("  ");
    }
    printf("%s", root->name);
    if(root->type == TOKEN){
        printf(": %s", root->value);
    }
    else{
        printf(" (%d)", root->line_number);
    }
    printf("\n");
    printTreeInfo(root->child, depth+1);
    printTreeInfo(root->next, depth);
}


static inline void delete_tree(pNode root, int depth){
    if(root->child==NULL&&root->next==NULL){
        free(root->name);
        free(root->value);
        free(root);
    } 
    printf("\n");
    delete_tree(root->child, depth+1);
    delete_tree(root->next, depth);
}

#endif