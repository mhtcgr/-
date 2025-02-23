%{
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "node.h"
struct Node* root;
extern int isError;
//用户函数部分
#include "lex.yy.c"

int yyerror(char* msg) {
	fprintf(stderr, "Error type B at line %d: %s.\n", yylineno, msg);
	isError = 1;
}


%}


/* declared types */
%union{
    struct Node* node; 
}

// tokens

%token <node> INT
%token <node> FLOAT
%token <node> ID
%token <node> TYPE
%token <node> COMMA
%token <node> DOT
%token <node> SEMI
%token <node> RELOP
%token <node> ASSIGNOP
%token <node> PLUS MINUS STAR DIV
%token <node> AND OR NOT 
%token <node> LP RP LB RB LC RC
%token <node> IF
%token <node> ELSE
%token <node> WHILE
%token <node> STRUCT
%token <node> RETURN

// non-terminals

%type <node> Program ExtDefList ExtDef ExtDecList   //  High-level Definitions
%type <node> Specifier StructSpecifier OptTag Tag   //  Specifiers
%type <node> VarDec FunDec VarList ParamDec         //  Declarators
%type <node> CompSt StmtList Stmt                   //  Statements
%type <node> DefList Def Dec DecList                //  Local Definitions
%type <node> Exp Args                               //  Expressions

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT
%left LB RB
%left LP RP
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%
// 语法规则部分
//High-level Definitions
//一个程序可以产生一个ExtDefList
Program: ExtDefList
{
    $$=ASTNodeCreator(@$.first_line, "Program", NODE_NOTHING);
    root=$$;
    add_child($$, $1);
}
;

//ExtDefList表示零个或多个ExtDef
ExtDefList: ExtDef ExtDefList {
    $$=ASTNodeCreator(@$.first_line, "ExtDefList", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
}
| /* epsilon */ { $$=NULL; }
;

ExtDef: Specifier ExtDecList SEMI {
    $$=ASTNodeCreator(@$.first_line, "ExtDef", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
}
| Specifier SEMI {
    $$=ASTNodeCreator(@$.first_line, "ExtDef", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
}
|error SEMI                              { isError = 1; }
| Specifier FunDec CompSt {
    $$=ASTNodeCreator(@$.first_line, "ExtDef", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
}
;

ExtDecList: VarDec {
    $$=ASTNodeCreator(@$.first_line, "ExtDecList", NODE_NOTHING);
    add_child($$, $1);
}
| VarDec COMMA ExtDecList {
    $$=ASTNodeCreator(@$.first_line, "ExtDecList", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
}
;

Specifier: TYPE {
    $$=ASTNodeCreator(@$.first_line, "Specifier", NODE_NOTHING);
    add_child($$, $1);
}
| StructSpecifier {
    $$=ASTNodeCreator(@$.first_line, "Specifier", NODE_NOTHING);
    add_child($$, $1);
}
;

StructSpecifier: STRUCT OptTag LC DefList RC {
    $$=ASTNodeCreator(@$.first_line, "StructSpecifier", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
    add_child($$, $4);
    add_child($$, $5);
}
| STRUCT Tag {
    $$=ASTNodeCreator(@$.first_line, "StructSpecifier", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
}
;

OptTag: ID {
    $$=ASTNodeCreator(@$.first_line, "OptTag", NODE_NOTHING);
    add_child($$, $1);
}
| /* epsilon */ { $$=NULL; }
;

Tag: ID {
    $$=ASTNodeCreator(@$.first_line, "Tag", NODE_NOTHING);
    add_child($$, $1);
}
;

FunDec: ID LP VarList RP {
    $$=ASTNodeCreator(@$.first_line, "FunDec", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
    add_child($$, $4);
}
| ID LP RP {
    $$=ASTNodeCreator(@$.first_line, "FunDec", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
}
;

VarDec: ID {
    $$=ASTNodeCreator(@$.first_line, "VarDec", NODE_NOTHING);
    add_child($$, $1);
}
| VarDec LB INT RB {
    $$=ASTNodeCreator(@$.first_line, "VarDec", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
    add_child($$, $4);
}
;

VarList: ParamDec COMMA VarList {
    $$=ASTNodeCreator(@$.first_line, "VarList", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
}
| ParamDec {
    $$=ASTNodeCreator(@$.first_line, "VarList", NODE_NOTHING);
    add_child($$, $1);
}
;

ParamDec: Specifier VarDec {
    $$=ASTNodeCreator(@$.first_line, "ParamDec", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
}
;

CompSt: LC DefList StmtList RC {
    $$=ASTNodeCreator(@$.first_line, "CompSt", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
    add_child($$, $3);
    add_child($$, $4);
}
;

StmtList: Stmt StmtList {
    $$=ASTNodeCreator(@$.first_line, "StmtList", NODE_NOTHING);
    add_child($$, $1);
    add_child($$, $2);
}
| /* epsilon */ { $$=NULL; }
;

Stmt: Exp SEMI { $$=ASTNodeCreator(@$.first_line, "Stmt", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
    }
    | CompSt { $$=ASTNodeCreator(@$.first_line, "Stmt", NODE_NOTHING);
            add_child($$, $1);
    }
    | RETURN Exp SEMI { $$=ASTNodeCreator(@$.first_line, "Stmt", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{ $$=ASTNodeCreator(@$.first_line, "Stmt", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
            add_child($$, $4);
            add_child($$, $5);
    }
    | IF LP Exp RP Stmt ELSE Stmt { $$=ASTNodeCreator(@$.first_line, "Stmt", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
            add_child($$, $4);
            add_child($$, $5);
            add_child($$, $6);
            add_child($$, $7);
    }
    | WHILE LP Exp RP Stmt { $$=ASTNodeCreator(@$.first_line, "Stmt", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
            add_child($$, $4);
            add_child($$, $5);
    }
    | error SEMI { isError = 1; }
    | error RP { isError = 1; }
    | Exp error SEMI { isError = 1; }
    ;

DefList: Def DefList { $$=ASTNodeCreator(@$.first_line, "DefList", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
    }
    | /* epsilon */ { $$=NULL; }
    ;

Def: Specifier DecList SEMI { $$=ASTNodeCreator(@$.first_line, "Def", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
    }
    | Specifier DecList error SEMI { isError = 1; }
    | Specifier error SEMI { isError = 1; }
    ;

DecList: Dec { $$=ASTNodeCreator(@$.first_line, "DecList", NODE_NOTHING); add_child($$, $1); }
    | Dec COMMA DecList { $$=ASTNodeCreator(@$.first_line, "DecList", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
    }
    ;

Dec: VarDec { $$=ASTNodeCreator(@$.first_line, "Dec", NODE_NOTHING); add_child($$, $1); }
    | VarDec ASSIGNOP Exp { $$=ASTNodeCreator(@$.first_line, "Dec", NODE_NOTHING);
            add_child($$, $1);
            add_child($$, $2);
            add_child($$, $3);
    }
    ;
//Expressions
Exp: Exp ASSIGNOP Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp AND Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp OR Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp RELOP Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp PLUS Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp MINUS Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp STAR Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp DIV Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | LP Exp RP { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | MINUS Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
    }
    | NOT Exp { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
    }
    | ID LP Args RP { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
        add_child($$, $4);
    }
    | ID LP RP { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp LB Exp RB { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
        add_child($$, $4);
    }
    | Exp DOT ID { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | ID { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
    }
    | INT { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
    }
    | FLOAT { $$=ASTNodeCreator(@$.first_line, "Exp", NODE_NOTHING);
        add_child($$, $1);
    }
    ;

Args: Exp COMMA Args { $$=ASTNodeCreator(@$.first_line, "Args", NODE_NOTHING);
        add_child($$, $1);
        add_child($$, $2);
        add_child($$, $3);
    }
    | Exp { $$=ASTNodeCreator(@$.first_line, "Args", NODE_NOTHING);
        add_child($$, $1);
    }
    ;


%%



