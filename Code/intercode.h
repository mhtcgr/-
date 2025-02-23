#ifndef __INTERCODE_H__
#define __INTERCODE_H__
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "symtab.h"

typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct InterCodeList_* InterCodeList;

struct Operand_ {
    enum {
        OP_VARIABLE,   
        OP_ADDRESS,    
        OP_FUNCTION,   
        OP_ARRAY,      
        OP_STRUCTURE,  
        OP_LABEL,      
        OP_TEMP,       
        OP_CONSTANT,   
    } kind;
    union {
        char* var_name;       
        char* array_name;     
        int addr_no;          
        char* func_name;      
        int label_no;         
        int temp_no;          
        long long const_val; 
    } u;
    Type type;  
    int size;   
};

struct InterCode_ {
    enum {
        IR_LABEL,    // LABEL x :
        IR_FUNC,     // FUNCTION f :
        IR_GOTO,     // GOTO x
        IR_RETURN,   // RETURN x
        IR_ARG,      // ARG x
        IR_PARAM,    // PARAM x
        IR_READ,     // READ x
        IR_WRITE,    // WRITE x
        IR_DEC,      // DEC x [size]
        IR_ASSIGN,   // x := y
        IR_ADDR,     // x := &y
        IR_LOAD,     // x := *y
        IR_STORE,    // *x := y
        IR_CALL,     // x := CALL f
        IR_ADD,      // x := y+z
        IR_SUB,      // x := y-z
        IR_MUL,      // x := y*z
        IR_DIV,      // x := y/z
        IR_IF_GOTO,  // IF x [relop] y GOTO z
    } kind;
    union {
        // IR_LABEL IR_FUNC IR_GOTO IR_RETURN IR_ARG IR_PARAM IR_READ IR_WRITE
        struct {
            Operand op;
        } single_ir;  

        // IR_ASSIGN IR_ADDR IR_LOAD IR_STORE IR_CALL
        struct {
            Operand right, left;
        } binary_ir;  

        // IR_ADD IR_SUB IR_MUL IR_DIV
        struct {
            Operand res, op1, op2;
        } ternary_ir;  

        // IR_DEC x [size]
        struct {
            Operand op;
            int size;
        } dec;  

        // IR_IF_GOTO
        struct {
            Operand x, y, z;
            char relop[64];
        } if_goto;  
    } u;
};

struct InterCodeList_ {
    InterCode code;
    InterCodeList prev, next;
};

InterCodeList init_ir_list_head();                                 
void add_irCode(InterCodeList ir_list_head, InterCode ir);        
void del_irCode(InterCodeList ir);                            
void print_irCodes(InterCodeList ir_list_head, FILE* ir_out);  
void print_irCode(InterCode ir, FILE* ir_out);                
void print_op(Operand op, FILE* ir_out);                       
InterCode new_irCode(InterCodeList ir_list_head, int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size, char* relop);                                                 // 生成IR
Operand new_op(int operand_kind ,int val,char* name);
void struct_err();
#endif