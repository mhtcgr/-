#include "intercode.h"

int label_id = 0;
int temp_id = 0;
int addr_id = 0;
 
InterCodeList init_ir_list_head() {
    InterCodeList ir_list_head = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    assert(ir_list_head != NULL);
    ir_list_head->code = NULL;
    ir_list_head->prev = ir_list_head->next = ir_list_head;
    return ir_list_head;
}
//
void add_irCode(InterCodeList ir_list_head, InterCode ir) {
    if (ir == NULL) return;
    InterCodeList new_ir = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    new_ir->code = ir;
    InterCodeList tail = ir_list_head->prev;
    new_ir->prev = tail;
    tail->next = new_ir;
    new_ir->next = ir_list_head;
    ir_list_head->prev = new_ir;
}
//
void del_irCode(InterCodeList ir) {
    if (ir == NULL) return;
    InterCodeList prev = ir->prev;
    InterCodeList next = ir->next;
    prev->next = next;
    next->prev = prev;
}
//
void print_irCodes(InterCodeList ir_list_head, FILE* ir_out) {
    InterCodeList cur = ir_list_head->next;
    while (cur != ir_list_head) {
        assert(cur->code != NULL);
        print_irCode(cur->code, ir_out);
        cur = cur->next;
    }
}
//
void print_irCode(InterCode ir, FILE* ir_out) {
    if (ir == NULL || ir_out == NULL) return;
    
    // 控制流类
    switch (ir->kind) {
        case IR_LABEL:
            fprintf(ir_out, "LABEL ");
            print_op(ir->u.single_ir.op, ir_out);
            fprintf(ir_out, ": ");
            break;
        case IR_FUNC:
            fprintf(ir_out, "FUNCTION ");
            print_op(ir->u.single_ir.op, ir_out);
            fprintf(ir_out, ": ");
            break;
        case IR_GOTO:
            fprintf(ir_out, "GOTO ");
            print_op(ir->u.single_ir.op, ir_out);
            break;
        case IR_IF_GOTO:
            fprintf(ir_out, "IF ");
            print_op(ir->u.if_goto.x, ir_out);
            fprintf(ir_out, "%s ", ir->u.if_goto.relop);
            print_op(ir->u.if_goto.y, ir_out);
            fprintf(ir_out, "GOTO ");
            print_op(ir->u.if_goto.z, ir_out);
            break;
        case IR_RETURN:
            fprintf(ir_out, "RETURN ");
            print_op(ir->u.single_ir.op, ir_out);
            break;
        case IR_ARG:
            fprintf(ir_out, "ARG ");
            print_op(ir->u.single_ir.op, ir_out);
            break;
        case IR_PARAM:
            fprintf(ir_out, "PARAM ");
            print_op(ir->u.single_ir.op, ir_out);
            break;
        case IR_READ:
            fprintf(ir_out, "READ ");
            print_op(ir->u.single_ir.op, ir_out);
            break;
        case IR_WRITE:
            fprintf(ir_out, "WRITE ");
            print_op(ir->u.single_ir.op, ir_out);
            break;

        // 数据操作类
        case IR_ASSIGN:
            print_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= ");
            print_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_ADDR:
            print_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= &");
            print_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_LOAD:
            print_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= *");
            print_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_STORE:
            fprintf(ir_out, "*");
            print_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= ");
            print_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_ADD:
            print_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            print_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "+ ");
            print_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_SUB:
            print_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            print_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "- ");
            print_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_MUL:
            print_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            print_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "* ");
            print_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_DIV:
            print_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            print_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "/ ");
            print_op(ir->u.ternary_ir.op2, ir_out);
            break;

        // 函数调用与参数类
        case IR_CALL:
            print_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= CALL ");
            print_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_DEC:
            fprintf(ir_out, "DEC ");
            print_op(ir->u.dec.op, ir_out);
            fprintf(ir_out, "%d ", ir->u.dec.size);
            break;

        default:
            assert(0);
            break;
    }
    
    fprintf(ir_out, "\n");
}
//
void print_op(Operand op, FILE* ir_out) {
    if (op == NULL) return;
    switch (op->kind) {
        case OP_TEMP:
            fprintf(ir_out, "t%d ", op->u.temp_no);
            break;
        case OP_LABEL:
            fprintf(ir_out, "label%d ", op->u.label_no);
            break;
        case OP_ARRAY:
            fprintf(ir_out, "array_%s ", op->u.array_name);
            break;
        case OP_ADDRESS:
            fprintf(ir_out, "addr%d ", op->u.addr_no);
            break;
        case OP_FUNCTION:
            fprintf(ir_out, "%s ", op->u.func_name);
            break;
        case OP_VARIABLE:
            fprintf(ir_out, "var_%s ", op->u.var_name);
            break;
        case OP_CONSTANT:
            fprintf(ir_out, "#%lld ", op->u.const_val);
            break;
        case OP_STRUCTURE:
            struct_err();
            break;
        default:
            printf("%s\n",op->u.func_name);
            printf("Error: Invalid operand kind%d.\n", op->kind);
            assert(0);
            break;
    }
}
//
InterCode new_irCode(InterCodeList ir_list_head, int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size, char* relop) {
    InterCode ir = NULL;
    
    switch (ir_kind) {
        case IR_LABEL:
        case IR_FUNC:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_PARAM:
        case IR_READ:
        case IR_WRITE:
            if (op1 != NULL) {
                ir = (InterCode)malloc(sizeof(struct InterCode_));
                ir->kind = ir_kind;
                ir->u.single_ir.op = op1;
            }
            break;
        case IR_DEC:
            if (op1 != NULL) {
                ir = (InterCode)malloc(sizeof(struct InterCode_));
                ir->kind = ir_kind;
                ir->u.dec.op = op1;
                ir->u.dec.size = dec_size;
            }
            break;
        case IR_ASSIGN:
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
        case IR_CALL:
            if (op1 != NULL && op2 != NULL) {
                ir = (InterCode)malloc(sizeof(struct InterCode_));
                ir->kind = ir_kind;
                ir->u.binary_ir.left = op1;
                ir->u.binary_ir.right = op2;
            }
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            if (op1 != NULL && op2 != NULL && op3 != NULL) {
                ir = (InterCode)malloc(sizeof(struct InterCode_));
                ir->kind = ir_kind;
                ir->u.ternary_ir.res = op1;
                ir->u.ternary_ir.op1 = op2;
                ir->u.ternary_ir.op2 = op3;
            }
            break;
        case IR_IF_GOTO:
            if (op1 != NULL && op2 != NULL && op3 != NULL) {
                ir = (InterCode)malloc(sizeof(struct InterCode_));
                ir->kind = ir_kind;
                ir->u.if_goto.x = op1;
                ir->u.if_goto.y = op2;
                ir->u.if_goto.z = op3;
                strcpy(ir->u.if_goto.relop, relop);
            }
            break;
        default:
            assert(0);
            break;
    }

    if (ir_list_head) add_irCode(ir_list_head, ir);
    return ir;
}
//
Operand new_op(int operand_kind, int val,char* name) {
    Operand op = (Operand)malloc(sizeof(struct Operand_));
    op->kind = operand_kind;
    FieldList result;
    switch (operand_kind) {
        case OP_CONSTANT:
            op->u.const_val = val;
            break;
        case OP_VARIABLE:
            result = search(name);
            op->u.var_name = result->name;
            break;
        case OP_ARRAY:
            result = search(name);
            op->u.array_name = result->name;
            break;
        case OP_STRUCTURE:
            struct_err();
            break;            
        case OP_ADDRESS:
            op->u.addr_no = addr_id++;
            break;
        case OP_FUNCTION:
            op->u.func_name = name;
            break;
        case OP_LABEL:
            op->u.label_no = label_id++;
            break;
        case OP_TEMP:
            op->u.temp_no = temp_id++;
            break;
        default:
            assert(0);
            break;
    }
    return op;
}
//
void struct_err(){
    fprintf(stderr, "Error: Structure type is not supported in this version.\n");
    exit(1);
}