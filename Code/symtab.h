#ifndef __SYMTAB_H__
#define __SYMTAB_H__
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 0x3fff
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct HashNode_* HashNode;

struct Type_ {
    enum { BASIC, ARRAY, STRUCTURE, FUNCTION} kind;
    union {
        enum { NUM_INT, NUM_FLOAT } basic;  // 基本类型
		struct {
			int arg_num;
            FieldList arg;
            Type ret;
        } function;
        struct {
            Type elem;
            int size;
        } array;              // 数组类型信息包括元素类型与数组大小构成
        FieldList structure;
    } u;
};

struct FieldList_ {
    char* name;      // 域的名字
    Type type;       // 域的类型
    FieldList tail;  // 下一个域
    int isArg;       // 是否是参数
};

struct HashNode_ {
    FieldList data;
    HashNode next;
};
void init();
void print_hashtab();
unsigned int hash(char* name);
void insert(FieldList field);
FieldList search(char* name);

#endif