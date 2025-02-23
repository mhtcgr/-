该编译器实现了将C--源代码转换为MIPS32汇编代码的功能，支持完整的编译流程。项目的主要模块包括：

1.  **词法分析**（Lexical Analysis）：识别源代码中的基本单元，如关键字、标识符、操作符等，生成词法单元（tokens）。
2.  **语法分析**（Syntax Analysis）：根据C--语言的语法规则，分析源代码结构，构建语法树。
3.  **语义分析**（Semantic Analysis）：检查源代码的语义错误，如类型检查、作用域检查等。
4.  **中间代码生成**（Intermediate Code Generation）：将源代码转换成中间表示，便于后续优化和最终代码生成。
5.  **目标代码生成**（Code Generation）：生成MIPS32汇编代码，作为编译输出。

### 文件结构：

1.  **intercode.c**：负责中间代码的生成，将语法树转换为中间表示。
2.  **lexical.l**：定义了词法分析器的规则，利用lex工具生成词法单元。
3.  **main.c**：编译器的主程序，负责调用各个模块进行编译流程。
4.  **node.h**：定义了抽象语法树（AST）节点的结构。
5.  **parser**：包含语法分析的实现，利用yacc工具生成语法分析器。
6.  **semantic.c**：进行语义分析，检查代码的类型和作用域等。
7.  **syntax.y**：定义了语法规则，使用yacc工具生成语法分析器。
8.  **test.cmm**：存放测试用的C--源代码，用于验证编译器的正确性。
9.  **translator.c**：负责最终的MIPS32汇编代码生成。
10.  **Makefile**：编译项目的构建文件，指定如何编译和链接各个源文件。
11.  **syntax.tab.c**：yacc生成的语法分析器代码。
12.  **semantic.h**、**node.h**、**syntab.h**：头文件，分别用于语法分析、符号表管理和节点定义。
13.  **syntax.tab.h**：yacc工具生成的语法规则头文件。
