#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include "intercode.h"
#include "node.h"


void translate_Program(Node* root);
void translate_ExtDefList(Node* root);
void translate_ExtDef(Node* root);
/**
 * 假设4 没有全局变量,无需考虑ExtDecList
 * 变量类型在语义分析部分全部完成,无需考虑Specifier
 */

Operand translate_VarDec(Node* root);  // 返回变量OP
void translate_FunDec(Node* root);
/**
 * 直接获取函数定义的Field,直接获取参数列表
 * 无需再取考虑VarList ParamDec
 */

void translate_CompSt(Node* root);
void translate_StmtList(Node* root);
void translate_Stmt(Node* root);

void translate_DefList(Node* root);
void translate_Def(Node* root);
void translate_DecList(Node* root);
void translate_Dec(Node* root);

void translate_Exp(Node* root, Operand place);
void translate_Args(Node* root, bool is_write);

void translate_Cond(Node* root, Operand true_label, Operand false_label);

Operand deep_copy_array(Operand op_left, Operand op_right);  // 数组深拷贝

Operand get_value(Operand addr);

Operand get_addr(Operand addr);  // 获取数组的地址，以及该数组是否需要作为实参区分处理

int get_size(Type type);  // 获取数组或者结构体的大小

void add_read_write_func();
#endif