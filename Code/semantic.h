#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__
#include "symtab.h"
#include "node.h"

void Program(Node* root);
void ExtDefList(Node* root);
void ExtDef(Node* root);
void ExtDecList(Node* root, Type type);
FieldList VarDec(Node* root, Type type, int isStructure);
void FunDec(Node* root, Type type);
FieldList VarList(Node* root);
FieldList ParamDec(Node* root);
Type Specifier(Node* root);
Type StructSpecifier(Node* root);
char* OptTag(Node* root);
FieldList DefList(Node* root, int isStructure);
FieldList Def(Node* root, int isStructure);
FieldList DecList(Node* root, Type type, int isStructure);
FieldList Dec(Node* root, Type type, int isStructure);
Type Exp(Node* root);
FieldList Args(Node* root);
void CompSt(Node* root, Type type);
void Stmtlist(Node* root, Type type);
void Stmt(Node* root, Type type);
int isSameType(Type type1, Type type2);
void add_read_write_func();
#endif