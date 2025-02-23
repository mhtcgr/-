#ifndef NODE_H
#define NODE_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylineno;

typedef struct Node {
    enum {
        NODE_INT, 
        NODE_FLOAT, 
        NODE_ID, 
        NODE_TYPE,
        OCTAL_TOKEN,
        HEX_TOKEN,
        NODE_NOTHING
        // 添加更多类型
    } node_type;
    int isLexicalUnit;
    int line;
    char* name;
    char* data;

    struct Node** children; // 子节点数组
    int num_children;          // 子节点个数
} Node;


static inline void tree_print(Node* node, int depth){
    if (!node){
        return;
    } 
    for (int i = 0; i < depth; i++){
        printf("  ");
    }
        
    printf("%s", node->name);
    if(node->isLexicalUnit){
        switch(node->node_type){
        case NODE_ID:
            printf(": %s",node->data);
            break;
        case NODE_TYPE:
            printf(": %s",node->data);
            break;
        case NODE_INT:
            printf(": %d", (int)strtol(node->data, NULL, 10));
            break;
        case NODE_FLOAT:
            printf(": %lf", strtod(node->data, NULL));
            break;
        case OCTAL_TOKEN:
        	printf(": %ld", strtol(node->data, NULL, 8));
            break;
        case HEX_TOKEN:
        	printf(": %ld", strtol(node->data, NULL, 16));  
            break;
        default:
            break;
        }
        printf("\n");
    }
    else{
        printf(" (%d)\n", node->line);
    }
    for(int i=0;i<node->num_children;i++){
        tree_print(node->children[i], depth+1);
    }
}

static inline void add_child(Node* parent, Node* child) {
    // 重新分配内存，每次为新增的子节点分配空间
    parent->children = (Node**)realloc(parent->children, sizeof(Node*) * (parent->num_children + 1));
    if (parent->children == NULL) {
        // 如果内存分配失败，直接返回
        printf("内存分配失败\n");
        return;
    }

    // 将子节点添加到 children 数组中
    parent->children[parent->num_children] = child;
    parent->num_children++; // 更新子节点数量
}

static inline Node* ASTNodeCreator(int line, char* name, int node_type){
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        return NULL; 
    }
    node->line=line;
    int nameLength = strlen(name) + 1;
    node->name = (char*)malloc(sizeof(char) * nameLength);
    strncpy(node->name, name, nameLength);
    node->isLexicalUnit=0;
    node->node_type=node_type;
    node->num_children=0;
    node->children=NULL;
    return node;
}

static inline Node* newTokenNode(int line, int type, char* name,char* text) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        return NULL; 
    }

    int nameLength = strlen(name) + 1;
    int textLength = strlen(text) + 1;

    node->line = line;
    node->node_type = type;

    node->name = (char*)malloc(sizeof(char) * nameLength);
    node->data = (char*)malloc(sizeof(char) * textLength);

    strncpy(node->name, name, nameLength);
    strncpy(node->data, text, textLength);

    node->num_children=0;
    node->children=NULL;
    node->isLexicalUnit = 1;

    return node;
}

static inline void delNode(Node* node) {
    if (node == NULL) return;
    if(node->children != NULL) {
    	for(int i=0;i<node->num_children;i++){
    		Node* temp = node->children[i];
    		delNode(temp);
    	}
    }
    free(node->name);
    free(node->data);
    free(node);
    free(node->children);
    node->name = NULL;
    node->data = NULL;
    node = NULL;
    node->children=NULL;
}

static inline  Node* get_child(Node* node, int i) {
    // printf("node->num_children = %d\n", node->num_children);
    // printf("i = %d\n", i);
    assert(i < node->num_children);
    return node->children[i];
}
#endif