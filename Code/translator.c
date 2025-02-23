#include "translator.h"
#include <stdlib.h>

extern InterCodeList ir_list_head;

//
void translate_Program(Node* root) {
    //printf("translate_Program\n");
    ir_list_head = init_ir_list_head();
    assert(ir_list_head != NULL);
    if (root == NULL) return;
    // Program -> ExtDefList
    assert(root->num_children == 1);
    translate_ExtDefList(get_child(root, 0));
}
//
void translate_ExtDefList(Node* root) {
    //printf("translate_ExtDefList\n");
    if (root == NULL) return;
    // ExtDefList -> ExtDef ExtDefList
    assert(root->num_children == 2);
    translate_ExtDef(get_child(root, 0));
    translate_ExtDefList(get_child(root, 1));
}
//
void translate_ExtDef(Node* root) {
    //printf("translate_ExtDef\n");
    if (root == NULL) return;
    assert(root->num_children == 2 || root->num_children == 3);
    if (root->num_children == 3) {
        if (strcmp(get_child(root, 1)->name, "ExtDecList") == 0) {  // ExtDef -> Specifier ExtDecList SEMI
            // 没有全局变量, 忽略
        } else if (strcmp(get_child(root, 2)->name, "CompSt") == 0) {  // ExtDef -> Specifier FunDec CompSt
            translate_FunDec(get_child(root, 1));
            translate_CompSt(get_child(root, 2));
        }
    }
}
//
Operand translate_VarDec(Node* root) {
    if (root == NULL) return NULL;
    assert(root->num_children == 1 || root->num_children == 4);
    Operand var_op;
    if (root->num_children == 1) {  // VarDec -> ID
        char* var_name = get_child(root, 0)->data;
        FieldList res = search(var_name);
        assert(res != NULL);
        if (res->type->kind == ARRAY) {  
            var_op = new_op(OP_ARRAY, 0, var_name);
            var_op->type = res->type->u.array.elem;
            var_op->size = res->type->u.array.size;
            int dec_size = get_size(res->type);
            // DEC variable.name size
            new_irCode(ir_list_head, IR_DEC, var_op, NULL, NULL, dec_size, NULL);
        } else if (res->type->kind == BASIC) { 
            var_op = new_op(OP_VARIABLE, 0, var_name);
        } else if (res->type->kind == STRUCTURE) {  // 结构体报错
            struct_err();
        }
    } else if (root->num_children == 4) {  // VarDec -> VarDec LB INT RB
        return translate_VarDec(get_child(root, 0));
    }
    return var_op;
}
//
void translate_FunDec(Node* root) {
    //printf("translate_FunDec\n");
    if (root == NULL) return;
    assert(root->num_children == 3 || root->num_children == 4);
    char* func_name = get_child(root, 0)->data;
    // printf("function name: %s\n", func_name);
    FieldList func = search(func_name);
    assert(func != NULL);
    if (root->num_children == 3) {         // FunDec -> ID LP RP
        Operand func_op = new_op(OP_FUNCTION, 0,  func_name);
        // FUNCTION func.name
        new_irCode(ir_list_head, IR_FUNC, func_op, NULL, NULL, -1, NULL);
    } 
    else if (root->num_children == 4) {  // FunDec -> ID LP VarList RP
        Operand func_op = new_op(OP_FUNCTION, 0,  func_name);
        // FUNCTION func.name
        new_irCode(ir_list_head, IR_FUNC, func_op, NULL, NULL, -1, NULL);
        FieldList args = func->type->u.function.arg;
        while (args) {
            // PARAM arg.name
            Operand arg_op = NULL;
            switch (args->type->kind) {
                case STRUCTURE:  // 假设不存在结构变量
                    struct_err();
                    break;
                case BASIC:  // 参数是普通变量
                    arg_op = new_op(OP_VARIABLE, 0,  args->name);
                    break;
                case ARRAY:  // 参数是数组
                    arg_op = new_op(OP_ARRAY, 0,  args->name);
                    break;
                default:
                    break;
            }
            new_irCode(ir_list_head, IR_PARAM, arg_op, NULL, NULL, -1, NULL);
            args = args->tail;
        }
    }
}
//
void translate_CompSt(Node* root) {
    //printf("translate_CompSt\n");
    if (root == NULL) return;
    // CompSt -> LC DefList StmtList RC
    assert(root->num_children == 4);
    translate_DefList(get_child(root, 1));
    translate_StmtList(get_child(root, 2));
}
//
void translate_StmtList(Node* root) {
    if (root == NULL) return;
    // Stmtlist -> Stmt Stmtlist
    assert(root->num_children == 2);
    translate_Stmt(get_child(root, 0));
    translate_StmtList(get_child(root, 1));
}
//
void translate_Stmt(Node* root) {
    if (root == NULL) return;
    assert(root->num_children == 1 || root->num_children == 2 || root->num_children == 3 || root->num_children == 5 || root->num_children == 7);
    if (root->num_children == 1) {  // Stmt -> CompSt
        translate_CompSt(get_child(root, 0));
    } else if (root->num_children == 2) {  // Stmt -> Exp SEMI
        Operand t1= new_op(OP_TEMP, 0, NULL);
        translate_Exp(get_child(root, 0), t1);
    } else if (root->num_children == 3) {  // Stmt -> RETURN Exp SEMI
        Operand t1 = new_op(OP_TEMP, 0, NULL);
        translate_Exp(get_child(root, 1), t1);
        t1 = get_value(t1);
        // RETURN t1
        new_irCode(ir_list_head, IR_RETURN, t1, NULL, NULL, -1, NULL);
    } else if (root->num_children == 5) {
        if (strcmp(get_child(root, 0)->name, "IF") == 0) {  // Stmt -> IF LP Exp RP Stmt
            Operand label1 = new_op(OP_LABEL, 0, NULL);
            Operand label2 = new_op(OP_LABEL, 0, NULL);
            translate_Cond(get_child(root, 2), label1, label2);
            // LABEL label1
            new_irCode(ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
            translate_Stmt(get_child(root, 4));
            // LABEL label2
            new_irCode(ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
        } else if (strcmp(get_child(root, 0)->name, "WHILE") == 0) {  // Stmt -> WHILE LP Exp RP Stmt
            Operand label1 = new_op(OP_LABEL, 0, NULL);
            Operand label2 = new_op(OP_LABEL, 0, NULL);
            Operand label3 = new_op(OP_LABEL, 0, NULL);
            // LABEL label1
            new_irCode(ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
            translate_Cond(get_child(root, 2), label2, label3);
            // LABEL label2
            new_irCode(ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
            translate_Stmt(get_child(root, 4));
            // GOTO label1
            new_irCode(ir_list_head, IR_GOTO, label1, NULL, NULL, -1, NULL);
            // LABEL label3
            new_irCode(ir_list_head, IR_LABEL, label3, NULL, NULL, -1, NULL);
        }
    } else if (root->num_children == 7) {  // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Operand label1 = new_op(OP_LABEL, 0, NULL);
        Operand label2 = new_op(OP_LABEL, 0, NULL);
        Operand label3 = new_op(OP_LABEL, 0, NULL);
        // print_tree(get_child(root, 2), 0);
        translate_Cond(get_child(root, 2), label1, label2);
        // LABEL label1
        new_irCode(ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        translate_Stmt(get_child(root, 4));
        // GOTO label3
        new_irCode(ir_list_head, IR_GOTO, label3, NULL, NULL, -1, NULL);
        // LABEL label2
        new_irCode(ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
        translate_Stmt(get_child(root, 6));
        // LABEL label3
        new_irCode(ir_list_head, IR_LABEL, label3, NULL, NULL, -1, NULL);
    }
}
//
void translate_DefList(Node* root) { // DefList -> Def DefList
    //printf("translate_DefList\n");
    if (root == NULL) return;
    
    assert(root->num_children == 2);
    translate_Def(get_child(root, 0));
    translate_DefList(get_child(root, 1));
}
//
void translate_Def(Node* root) { // Def -> Specifier DecList SEMI
//printf("translate_Def\n");
    if (root == NULL) return;
    
    assert(root->num_children == 3);
    translate_DecList(get_child(root, 1));
}
//
void translate_DecList(Node* root) {
    //printf("translate_DecList\n");
    if (root == NULL) return;
    assert(root->num_children == 1 || root->num_children == 3);
    if (root->num_children == 1) {         // DecList -> Dec
        translate_Dec(get_child(root, 0));
    } else if (root->num_children == 3) {  // DecList -> Dec COMMA DecList
        translate_Dec(get_child(root, 0));
        translate_DecList(get_child(root, 2));
    }
}
//
void translate_Dec(Node* root) {
    //printf("translate_Dec\n");
    if (root == NULL) return;
    assert(root->num_children == 1 || root->num_children == 3);
    if (root->num_children == 1) {         // Dec -> VarDec
        Operand var = translate_VarDec(get_child(root, 0));
    } else if (root->num_children == 3) {  // Dec -> VarDec ASSIGNOP Exp
        Operand var = translate_VarDec(get_child(root, 0));
        //printf("get vardec\n");
        Operand t1 = new_op(OP_TEMP, 0, NULL);
        //printf("rootchildrennum:%d\n", root->num_children);
        translate_Exp(get_child(root, 2), t1);
        //printf("get exp\n");
        if (var->kind == OP_VARIABLE) {  
            t1 = get_value(t1);
            // variable.name := t1
            new_irCode(ir_list_head, IR_ASSIGN, var, t1, NULL, -1, NULL);
        } else if (var->kind == OP_ARRAY) {  
            deep_copy_array(var, t1);
        }
    }
}

void translate_Exp(Node* root, Operand place) {
    if (root == NULL) return;
    assert(root->num_children == 1 || root->num_children == 2 || root->num_children == 3 || root->num_children == 4);
    /**
         * Exp -> NOT Exp
         * Exp -> Exp RELOP Exp
         * Exp -> Exp AND Exp
         * Exp -> Exp OR Exp
         */
    //printf("666666");
    if ((root->num_children == 3 && 
        (strcmp(get_child(root, 1)->name, "AND") == 0 || 
         strcmp(get_child(root, 1)->name, "OR") == 0 ||
          strcmp(get_child(root, 1)->name, "RELOP") == 0))  
         || (root->num_children == 2 && strcmp(get_child(root, 0)->name, "NOT") == 0)) {
        //printf("555555");
        
        Operand label0 = new_op(OP_LABEL, 0, NULL);
        Operand label1 = new_op(OP_LABEL, 0, NULL);
        //printf("4");

        // place := #0
        Operand zero = new_op(OP_CONSTANT, 0, NULL);
        new_irCode(ir_list_head, IR_ASSIGN, place, zero, NULL, -1, NULL);
        translate_Cond(root, label0, label1);
        // LABEL label0
        new_irCode(ir_list_head, IR_LABEL, label0, NULL, NULL, -1, NULL);
        // place := #1
        Operand one = new_op(OP_CONSTANT, 1, NULL);
        new_irCode(ir_list_head, IR_ASSIGN, place, one, NULL, -1, NULL);
        // LABEL label1
        new_irCode(ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        } 
    else if (root->num_children == 1) {
        // Exp -> ID
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID
            FieldList result = search(get_child(root, 0)->data);
            assert(result != NULL);
            if (result->type->kind == BASIC) {
                // place := variable.name
                place->kind = OP_VARIABLE;
                place->u.var_name = result->name;
            } 
            else if (result->type->kind == ARRAY) {
                //printf("数组变量\n");
                // 如果是函数的形式参数，则ID代表的是其数组的基地址
                if (result->isArg == 1) {
                    //printf("函数形式参数\n");
                    Operand array = new_op(OP_ARRAY, 0, result->name);
                    Operand array_base = new_op(OP_ADDRESS, 0, NULL);
                    place->kind = OP_ADDRESS;
                    place->u.addr_no=array_base->u.addr_no;
                    // place := array
                    new_irCode(ir_list_head, IR_ASSIGN, place, array, NULL, -1, NULL);
                } else {
                    //printf("普通数组\n");
                    place->kind = OP_ARRAY;
                    place->u.array_name = result->name;
                    //Operand array = new_op(OP_ARRAY, 0,  result->name);
                }

                place->type = result->type->u.array.elem;
                place->size = result->type->u.array.size;
            } 
            else if (result->type->kind == STRUCTURE) {  // 不存在结构体变量
                struct_err();
            }
        } 
        else if (strcmp(get_child(root, 0)->name, "INT") == 0) {  // Exp -> INT
            // place := #value
            place->kind = OP_CONSTANT;
            char *endptr;  // 用于指向解析后的字符串的剩余部分
            place->u.const_val = strtoll(get_child(root, 0)->data,&endptr, 10);
        } 
        else {  // 不存在浮点型常量
            assert(0);
        }
    } 
    else if (root->num_children == 2) {
        if (strcmp(get_child(root, 0)->name, "MINUS") == 0) {  // Exp -> MINUS Exp
            Operand t1 = new_op(OP_TEMP, 0, NULL);
            translate_Exp(get_child(root, 1), t1);
            t1 = get_value(t1);
            if (t1->kind == OP_CONSTANT) {
                place->kind = OP_CONSTANT;
                place->u.const_val = 0 - t1->u.const_val;
            } else {  // place := #0 - t1
                Operand zero = new_op(OP_CONSTANT, 0, NULL);
                new_irCode(ir_list_head, IR_SUB, place, zero, t1, -1, NULL);
            }
        }
    } 
    else if (root->num_children == 3) {
        if (strcmp(get_child(root, 0)->name, "LP") == 0) {  // Exp -> LP Exp RP
            translate_Exp(get_child(root, 1), place);
        } else if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP RP
            FieldList function = search(get_child(root, 0)->data);
            assert(function != NULL);
            if (strcmp(function->name, "read") == 0) {
                // READ place
                new_irCode(ir_list_head, IR_READ, place, NULL, NULL, -1, NULL);
            } else {
                // place := CALL function.name
                Operand func_op = new_op(OP_FUNCTION, 0,  function->name);
                new_irCode(ir_list_head, IR_CALL, place, func_op, NULL, -1, NULL);
            }
        } else if (strcmp(get_child(root, 1)->name, "DOT") == 0) {  // Exp -> Exp DOT ID
            // 结构体变量
            struct_err();
        } else if (strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0) {  // Exp ASSIGNOP Exp
            Operand left_t = new_op(OP_TEMP, 0, NULL);
            Operand right_t = new_op(OP_TEMP, 0, NULL);
            translate_Exp(get_child(root, 0), left_t);
            translate_Exp(get_child(root, 2), right_t);

            if (left_t->kind == OP_ARRAY || left_t->kind == OP_ADDRESS) {
                
                if (right_t->kind == OP_ARRAY || right_t->kind == OP_ADDRESS) {  // 数组赋值
                    deep_copy_array(left_t, right_t);
                } else {  
                    assert(left_t->kind == OP_ADDRESS);
                    // *left_t := right_t
                    new_irCode(ir_list_head, IR_STORE, left_t, right_t, NULL, -1, NULL);
                }
            } else {  // 左为普通变量
                right_t = get_value(right_t);
                // left_t := right_t
                new_irCode(ir_list_head, IR_ASSIGN, left_t, right_t, NULL, -1, NULL);
            }
            // place := right_t
            place->kind = right_t->kind;
            place->u = right_t->u;
        } else {
            /**
             * Exp -> Exp PLUS Exp
             * Exp -> Exp MINUS Exp
             * Exp -> Exp STAR Exp
             * Exp -> Exp DIV Exp
             */
            Operand t1 = new_op(OP_TEMP, 0, NULL);
            translate_Exp(get_child(root, 0), t1);
            t1 = get_value(t1);
            Operand t2 = new_op(OP_TEMP, 0, NULL);
            translate_Exp(get_child(root, 2), t2);
            t2 = get_value(t2);
            int ir_kind = -1;
            long long val;
            if (strcmp(get_child(root, 1)->name, "PLUS") == 0) {
                ir_kind = IR_ADD;
                val = t1->u.const_val + t2->u.const_val;
            } else if (strcmp(get_child(root, 1)->name, "MINUS") == 0) {
                ir_kind = IR_SUB;
                val = t1->u.const_val - t2->u.const_val;
            } else if (strcmp(get_child(root, 1)->name, "STAR") == 0) {
                ir_kind = IR_MUL;
                val = t1->u.const_val * t2->u.const_val;
            } else if (strcmp(get_child(root, 1)->name, "DIV") == 0) {
                ir_kind = IR_DIV;
                long long v1 = t1->u.const_val;
                long long v2 = t2->u.const_val;
                if (v1 > 0 && v2 < 0) {
                    val = (v1 - v2 - 1) / v2;
                } else if (v1 < 0 && v2 > 0) {
                    val = (v1 - v2 + 1) / v2;
                } else {
                    if (v2 == 0) {
                        val = 0;
                    } else {
                        //printf("v1:%lld, v2:%lld\n", v1, v2);
                        val = v1 / v2;
                    }
                }
            } else {
                assert(0);
            }
            if (t1->kind == OP_CONSTANT && t2->kind == OP_CONSTANT) {
                place->kind = OP_CONSTANT;
                place->u.const_val = val;
            } else {
                // place := t1 op t2
                new_irCode(ir_list_head, ir_kind, place, t1, t2, -1, NULL);
            }
        }
    } 
    else if (root->num_children == 4) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP Args RP
            FieldList function = search(get_child(root, 0)->data);
            assert(function != NULL);
            if (strcmp(function->name, "write") == 0) {
                translate_Args(get_child(root, 2), true);
                // place := #0
                place->kind = OP_CONSTANT;
                place->u.const_val = 0;
            } else {
                translate_Args(get_child(root, 2), false);
                // place := CALL function.name
                Operand func_op = new_op(OP_FUNCTION, 0,  function->name);
                new_irCode(ir_list_head, IR_CALL, place, func_op, NULL, -1, NULL);
            }
        } 
        else if (strcmp(get_child(root, 0)->name, "Exp") == 0) {  // Exp -> Exp LB Exp RB
            Operand t1 = new_op(OP_TEMP, 0, NULL);
            translate_Exp(get_child(root, 0), t1);
            assert(t1->kind == OP_ARRAY || t1->kind == OP_ADDRESS);
            Operand t2 = new_op(OP_TEMP, 0, NULL);
            translate_Exp(get_child(root, 2), t2);
            t2 = get_value(t2);
            Operand offset = new_op(OP_TEMP, 0, NULL);
            int width = get_size(t1->type);
            if (t2->kind == OP_CONSTANT) {
                offset->kind = OP_CONSTANT;
                offset->u.const_val = t2->u.const_val * width;
            } 
            else {
                // offset :=  t2 * width
                Operand width_op = new_op(OP_CONSTANT, width, NULL);
                new_irCode(ir_list_head, IR_MUL,offset,  t2,  width_op, -1, NULL);
            }

            // 将place设置为ADDRESS, 并赋值编号
            place->kind = OP_ADDRESS;
            Operand new_place = new_op(OP_ADDRESS, 0, NULL);
            place->u.addr_no = new_place->u.addr_no;

            if (t1->kind == OP_ARRAY) {  // Exp1-> ID
                
                Operand base = new_op(OP_ADDRESS, 0, NULL);
                // base := &addr
                new_irCode(ir_list_head, IR_ADDR, base, t1, NULL, -1, NULL);
                // place := base + offset
                new_irCode(ir_list_head, IR_ADD, place, base, offset, -1, NULL);
                
            } 
            else if (t1->kind == OP_ADDRESS) {  // Exp1 -> Exp LB Exp RB
                // place := t1 + offset
                new_irCode(ir_list_head, IR_ADD, place, t1, offset, -1, NULL);
            } 
            else {
                assert(0);
            }

            if (t1->type->kind == BASIC) {
                place->type = NULL;
                place->size = 0;
            } else if (t1->type->kind == ARRAY) {
                place->type = t1->type->u.array.elem;
                place->size = t1->type->u.array.size;
            }
        }
    }
}

void translate_Args(Node* root, bool is_write) {
    if (root == NULL) return;
    assert(root->num_children == 1 || root->num_children == 3);
    Operand arg = new_op(OP_TEMP, 0, NULL);
    if (root->num_children == 1) {         // Args -> Exp
        translate_Exp(get_child(root, 0), arg);
    } 
    else if (root->num_children == 3) {  // Args -> Exp COMMA Args
        assert(!is_write);            // WRITE 只有一个参数
        translate_Exp(get_child(root, 0), arg);
        translate_Args(get_child(root, 2), is_write);
    }
    
    
    if (arg->kind == OP_STRUCTURE) {  // 结构体不作为参数
        struct_err();
    }
    if (is_write) {  // WRITE arg
        arg = get_value(arg);
        new_irCode(ir_list_head, IR_WRITE, arg, NULL, NULL, -1, NULL);
    } else {
        if (arg->kind == OP_ARRAY) {
            arg = get_addr(arg);
        } 
        else if (arg->kind == OP_ADDRESS) {
            if (arg->type == NULL) {  // 数值
                arg = get_value(arg);
            } 
            else if (arg->type->kind == BASIC) {  // 一维数组
            } 
            else {                                
                assert(0);
            }
        }
        new_irCode(ir_list_head, IR_ARG, arg, NULL, NULL, -1, NULL);
    }
}
//
void translate_Cond(Node* root, Operand label_true, Operand label_false) {
    if (root == NULL) return;
    assert(root->num_children == 1 || root->num_children == 2 || root->num_children == 3 || root->num_children == 4);

    if (root->num_children>=2){
        if (strcmp(get_child(root, 1)->name, "AND") == 0)  {  // Exp AND Exp
        //printf("11111111111111");
        Operand label1 = new_op(OP_LABEL, 0, NULL);
        translate_Cond(get_child(root, 0), label1, label_false);
        //printf("2");
        new_irCode(ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        translate_Cond(get_child(root, 2), label_true, label_false);
        //printf("3");
    } 
    else if (strcmp(get_child(root, 1)->name, "OR") == 0) {  // Exp OR Exp
        Operand label1 = new_op(OP_LABEL, 0, NULL);
        translate_Cond(get_child(root, 0), label_true, label1);
        new_irCode(ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        translate_Cond(get_child(root, 2), label_true, label_false);
    }  
    else if (strcmp(get_child(root, 0)->name, "NOT") == 0) {  // NOT Exp
        translate_Cond(get_child(root, 1), label_false, label_true); 
    } 
    else if (strcmp(get_child(root, 1)->name, "RELOP") == 0) {  // Exp RELOP Exp
        Operand t1 = new_op(OP_TEMP, 0, NULL);
        translate_Exp(get_child(root, 0), t1);
        t1 = get_value(t1);
        Operand t2 = new_op(OP_TEMP, 0, NULL);
        translate_Exp(get_child(root, 2), t2);
        t2 = get_value(t2);
        char* relop = get_child(root, 1)->data;
        // IF t1 op t2 GOTO label_true
        new_irCode(ir_list_head, IR_IF_GOTO, t1, t2, label_true, -1, relop);
        // GOTO label_false
        new_irCode(ir_list_head, IR_GOTO, label_false, NULL, NULL, -1, NULL);
    }
    else {
        //printf("111111111\n");
        Operand t1 = new_op(OP_TEMP, 0, NULL);
        translate_Exp(root, t1);
        t1 = get_value(t1);
        
        // IF t1 != #0 GOTO label_true
        Operand zero = new_op(OP_CONSTANT, 0, NULL);
        new_irCode(ir_list_head, IR_IF_GOTO, t1, zero, label_true, -1, "!=");
        // GOTO label_false
        new_irCode(ir_list_head, IR_GOTO, label_false, NULL, NULL, -1, NULL);
    }
    }
     else {
        //printf("111111111\n");
        Operand t1 = new_op(OP_TEMP, 0, NULL);
        translate_Exp(root, t1);
        t1 = get_value(t1);
        
        // IF t1 != #0 GOTO label_true
        Operand zero = new_op(OP_CONSTANT, 0, NULL);
        new_irCode(ir_list_head, IR_IF_GOTO, t1, zero, label_true, -1, "!=");
        // GOTO label_false
        new_irCode(ir_list_head, IR_GOTO, label_false, NULL, NULL, -1, NULL);
    }
}
//
Operand deep_copy_array(Operand op_left, Operand op_right) {
    Operand left_base = get_addr(op_left);
    Operand right_base = get_addr(op_right);
    int size_left = get_size(op_left->type) * op_left->size;
    int size_right = get_size(op_right->type) * op_right->size;
    int size = 0;
    if (size_left < size_right) {
        size = size_left;
    } else {
        size = size_right;
    }
    Operand left = new_op(OP_ADDRESS, 0, NULL);
    Operand right = new_op(OP_ADDRESS, 0, NULL);
    Operand val = new_op(OP_TEMP, 0, NULL);
    //printf("size:%d\n", size);
     // val := *right
     new_irCode(ir_list_head, IR_LOAD, val, right_base, NULL, -1, NULL);
     // *left := val
     new_irCode(ir_list_head, IR_STORE, left_base, val, NULL, -1, NULL);
    for (int i = 4; i < size; i += 4) {
        Operand offset = new_op(OP_CONSTANT, i, NULL);
        // left := base + offset
        new_irCode(ir_list_head, IR_ADD, left, left_base, offset, -1, NULL);
        // right := base + offset
        new_irCode(ir_list_head, IR_ADD, right, right_base, offset, -1, NULL);
        // val := *right
        new_irCode(ir_list_head, IR_LOAD, val, right, NULL, -1, NULL);
        // *left := val
        new_irCode(ir_list_head, IR_STORE, left, val, NULL, -1, NULL);
    }
    return left_base;
}
//
Operand get_value(Operand op) {
    if (op->kind == OP_ADDRESS){
        Operand t = new_op(OP_TEMP, 0, NULL);
        // t := *addr
        new_irCode(ir_list_head, IR_LOAD, t, op, NULL, -1, NULL);
        return t;
    }
    return op;
}
//
Operand get_addr(Operand op) {
    if (op->kind == OP_ARRAY) {
        Operand t = new_op(OP_ADDRESS, 0, NULL);
        // t := &op
        new_irCode(ir_list_head, IR_ADDR, t, op, NULL, -1, NULL);
        return t;
    }
    return op;
}
//
int get_size(Type type) {
    if (type == NULL) return 0;
    if (type->kind == BASIC) {
        return 4;
    } else if (type->kind == ARRAY) {
        return type->u.array.size * get_size(type->u.array.elem);
    } else if (type->kind == STRUCTURE) {
        struct_err();
        int size = 0;
        FieldList iter = type->u.structure;
        while (iter) {
            size += get_size(iter->type);
            iter = iter->tail;
        }
        return size;

    } else {
        assert(0);
    }
    return 0;
}


