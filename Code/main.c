#include <stdio.h>
#include "syntax.tab.h"
#include "semantic.h"
#include "translator.h"
                    
extern int yyparse();
extern void yyrestart(FILE*);
extern struct Node* root;  
extern int isError;
InterCodeList ir_list_head;   // IR循环双向链表头

int main(int argc, char** argv){
	if (argc <= 2){yyparse();return 1;}
	FILE* f = fopen(argv[1], "r");
	if (!f){
		fprintf(stderr, "Error: could not open file %s\n", argv[1]);
		perror(argv[1]);
		return 1;
	}
	FILE* ir_out = fopen(argv[2], "w");
	if (!ir_out){
		fprintf(stderr, "Error: could not open file %s\n", argv[2]);
		perror(argv[2]);
		return 1;
	}

	yyrestart(f);
	//yydebug = 1;
	yyparse();
	fclose(f);
	if(!isError){
		//tree_print(root, 0);
        //fflush(stdout);
        init();
		add_read_write_func();
        Program(root);
		translate_Program(root);
		print_irCodes(ir_list_head, ir_out);
    }
	fclose(ir_out);

	return 0;
}