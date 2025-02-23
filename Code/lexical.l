%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "syntax.tab.h" 
#include "node.h"
int num=1; 
int line=1;
int col=1; 
int yycolumn = 1; 
int isError=0;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

%}

%option yylineno

keywords int|float|if|else|while|return|struct
identifier ({latter}|_)({latter}|_|{digit})*
latter [a-zA-Z]
digit [0-9]
decimal_constant ({nonzero_digit}{digit}*)|0
octal_constant 0{octal_digit}*
hexadecimal_constant {hexadecimal_prefix}{hexadecimal_digit}+
hexadecimal_prefix (0x|0X)
nonzero_digit [1-9]
octal_digit [0-7]
hexadecimal_digit [0-9a-fA-F]
float_constant (0|({nonzero_digit}{digit}*))\.{digit}*
RELOP 	>|<|>=|<=|==|!=
bad_octal_constant 0([0-7]*[8|9]+)+
bad_hexadecimal_constant {hexadecimal_prefix}([1-9a-fA-F]*[g-zG-Z]+)+

%%
\n {++line;}
{keywords} {
    // printf("keywords %s\n", yytext);
    if (strcmp(yytext,"int")==0) {
        yylval.node = newTokenNode(yylineno, NODE_TYPE, "TYPE", yytext); 
        return TYPE;
    }
    if (strcmp(yytext,"float")==0) {
        yylval.node = newTokenNode(yylineno, NODE_TYPE, "TYPE", yytext); 
        return TYPE;
    }
    if (strcmp(yytext,"if")==0) {
        yylval.node = newTokenNode(yylineno, NODE_NOTHING, "IF", yytext); 
        return IF;
    }
    if (strcmp(yytext,"else")==0) {
        yylval.node = newTokenNode(yylineno, NODE_NOTHING, "ELSE", yytext); 
        return ELSE;
    }
    if (strcmp(yytext,"while")==0) {
        yylval.node = newTokenNode(yylineno, NODE_NOTHING, "WHILE", yytext); 
        return WHILE;
    }
    if (strcmp(yytext,"return")==0) {
        yylval.node = newTokenNode(yylineno, NODE_NOTHING, "RETURN", yytext); 
        return RETURN;
    }
    if (strcmp(yytext,"struct")==0) {
        yylval.node = newTokenNode(yylineno, NODE_NOTHING, "STRUCT", yytext); 
        return STRUCT;
    }
}

{bad_octal_constant} {
    printf("Error type A at Line %d: Bad octal constant \'%s\'\n",yylineno,yytext);
    isError = 1;
}

{bad_hexadecimal_constant} {
    printf("Error type A at Line %d: Bad hexadecimal constant \'%s\'\n",yylineno,yytext);
    isError = 1;
}

{decimal_constant} {
    // printf("integer_constant %s\n", yytext);
    yylval.node = newTokenNode(yylineno, NODE_INT, "INT", yytext);
    return INT;
}

"//".* {isError=1; printf("Error type B at Line %d: Comment not allowed\n",yylineno); }

"/*"([^*]|\*+[^*/])*\*+"/" {
    isError=1;
    int count = 0;
    while (*yytext) {
        if (*yytext == '\n') {
            count++;
        }
        yytext++;
    }
    for(int i=count;i>=0;i--) {
        printf("Error type B at Line %d: Comment not allowed\n",yylineno-i);
    }
    
}

{octal_constant} {
    // printf("octal_constant %s\n", yytext);
    yylval.node = newTokenNode(yylineno, OCTAL_TOKEN, "INT", yytext);
    return INT;
}

{hexadecimal_constant} {
    // printf("hexadecimal_constant %s\n", yytext);
    yylval.node = newTokenNode(yylineno, HEX_TOKEN, "INT", yytext);
    return INT;
}

{float_constant} {
    // printf("floating_constant %s\n", yytext);
    yylval.node = newTokenNode(yylineno, NODE_FLOAT, "FLOAT", yytext);
    return FLOAT;
}

{identifier} {
    // printf("identifier %s\n", yytext);
    yylval.node = newTokenNode(yylineno, NODE_ID, "ID", yytext);
    return ID;
}

";" {
    // printf(";\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "SEMI", yytext);
    return SEMI;
}

"," {
    // printf(",\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "COMMA", yytext);
    return COMMA;
}

"=" {
    // printf("=\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "ASSIGNOP", yytext);
    return ASSIGNOP;
}

{RELOP} {
    // printf("%d,%s\n", yylineno, yytext);
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "RELOP", yytext);
    return RELOP;
}

"+" {
    // printf("+\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "PLUS", yytext);
    return PLUS;
}

"-" {
    // printf("-\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "MINUS", yytext);
    return MINUS;
}

"*" {
    // printf("*\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "STAR", yytext);
    return STAR;
}

"/" {
    // printf("/\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "DIV", yytext);
    return DIV;
}

"&&" {
    // printf("&&\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "AND", yytext);
    return AND;
}

"||" {
    // printf("||\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "OR", yytext);
    return OR;
}

"." {
    // printf(".\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "DOT", yytext);
    return DOT;
}

"!" {
    // printf("!\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "NOT", yytext);
    return NOT;
}

"(" {
    // printf("(\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "LP", yytext);
    return LP;
}

")" {
    // printf(")\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "RP", yytext);
    return RP;
}

"{" {
    // printf("{\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "LC", yytext);
    return LC;
}

"}" {
    // printf("}\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "RC", yytext);
    return RC;
}

"[" {
    // printf("[\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "LB", yytext);
    return LB;
}

"]" {
    // printf("]\n");
    yylval.node = newTokenNode(yylineno, NODE_NOTHING, "RB", yytext);
    return RB;
}


[ \t\r]+ {}
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno,yytext); isError=1;}
%%
