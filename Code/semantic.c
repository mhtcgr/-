# include "semantic.h"

void Program(Node* root){
    if (root == NULL) return;
    assert(root->num_children == 1);
    // printf("into Program, root->num_children = %d\n", root->num_children);
    Node* e=get_child(root, 0);
    ExtDefList(e);
}

void ExtDefList(Node* root) {
    if (root == NULL) return;
    assert(root->num_children == 2);
    ExtDef(get_child(root, 0));
    ExtDefList(get_child(root, 1));
}

void ExtDef(Node* root){
    if (root == NULL) return;
    assert(root->num_children == 2||root->num_children == 3);
    Type ret_type = Specifier(get_child(root, 0));
    if(root->num_children == 2){
        assert(strcmp(get_child(root, 1)->name, "SEMI") == 0);   // ExtDef -> Specifier SEMI
    }
    else if(root->num_children == 3){
        if (strcmp(get_child(root, 1)->name, "ExtDecList") == 0) {  // ExtDef -> Specifier ExtDecList SEMI
            ExtDecList(get_child(root, 1), ret_type);
        } else if (strcmp(get_child(root, 2)->name, "CompSt") == 0) {  // ExtDef -> Specifier FunDec CompSt
            FunDec(get_child(root, 1), ret_type);
            CompSt(get_child(root, 2), ret_type);
        }
    }
}

void ExtDecList(Node* root, Type type){
    if (root == NULL) return;
    assert(root->num_children == 1||root->num_children == 3);
    if(root->num_children == 1){
        assert(strcmp(get_child(root, 0)->name, "VarDec") == 0);   // ExtDecList -> VarDec
        VarDec(get_child(root, 0), type, 0);
    }
    else if(root->num_children == 3){
        VarDec(get_child(root, 0), type, 0);
        ExtDecList(get_child(root, 2), type);
    }
}

FieldList VarDec(Node* root, Type type, int isStructure){
    //isStructure = 0说明需要写入符号表，反之说明只是结构体的变量列表
    if (root == NULL) return NULL;
    assert(root->num_children == 1||root->num_children == 4);
    if(root->num_children == 1){
        assert(strcmp(get_child(root, 0)->name, "ID") == 0);   // VarDec -> ID
        FieldList fl = (FieldList)malloc(sizeof(struct FieldList_));
        fl->name = get_child(root, 0)->data;
        fl->type = type;
        fl->tail = NULL;
        fl->isArg = 0;
        //printf("fl_name = %s, isStructure = %d\n",fl->name, isStructure);
        //print_hashtab();
        if (isStructure) {
            //如果是结构体类型，那么不需要插入符号表，只需要进行成员的匹配，没有重复的就行
        }
        else{
            if(search(fl->name)==NULL) insert(fl);
            else{
                printf("Error type 3 at Line %d:Redefined variable\n", root->line);
            }
        }
        return fl;
    }
    else if(root->num_children == 4){ // VarDec -> VarDec LB INT RB
        Type t = (Type)malloc(sizeof(struct Type_));
        t->kind = ARRAY;
        t->u.array.elem = type;
        t->u.array.size = atoi(get_child(root, 2)->data);
        return VarDec(get_child(root, 0), t, isStructure);
    }
    return NULL;
}

void FunDec(Node* root, Type type) {
    //print_hashtab();
    if (root == NULL) return;
    assert(root->num_children == 3 || root->num_children == 4);
    char* name = get_child(root, 0)->data;
    if(search(name)!=NULL){
        printf("Error type 4 at Line %d:Redefined function\n", root->line);
        return;
    }
    Type t = (Type)malloc(sizeof(struct Type_));
    t->kind = FUNCTION;
    t->u.function.ret = type;
    FieldList fl = (FieldList)malloc(sizeof(struct FieldList_));
    fl->type = t;
    fl->name = get_child(root, 0)->data;
    fl->tail = NULL;
    fl->isArg = 0;

    if (root->num_children == 3) {         // FunDec -> ID LP RP
        t->u.function.arg = NULL;
        t->u.function.arg_num = 0;
    } else if (root->num_children == 4) {  // FunDec -> ID LP VarList RP
        t->u.function.arg = VarList(get_child(root, 2));
        FieldList arg = t->u.function.arg;
        t->u.function.arg_num = 0;
        while(arg != NULL){
            t->u.function.arg_num++;
            arg = arg->tail;
        }
    }
    insert(fl);
}

FieldList VarList(Node* root) {
    if (root == NULL) return NULL;
    assert(root->num_children == 1 || root->num_children == 3);
    FieldList fl = ParamDec(get_child(root, 0));
    fl->isArg = 1;
    if (root->num_children == 1) {  // VarList -> ParamDec
    } else if (root->num_children == 3) {  // VarList -> ParamDec COMMA VarList
        fl->tail = VarList(get_child(root, 2));
    }
    return fl;
}

FieldList ParamDec(Node* root) {
    if (root == NULL) return NULL;
    assert(root->num_children == 2);// ParamDec -> Specifier VarDec
    Type type = Specifier(get_child(root, 0));
    return VarDec(get_child(root, 1), type, 0);
}

void CompSt(Node* root, Type type){//LC DefList StmtList RC
    if (root == NULL) return;
    assert(root->num_children == 4);
    DefList(get_child(root, 1), 0);
    Stmtlist(get_child(root, 2), type);
}

void Stmtlist(Node* root, Type type){//Stmt StmtList
    if (root == NULL) return;
    assert(root->num_children == 2);
    Stmt(get_child(root, 0), type);
    Stmtlist(get_child(root, 1), type);
}

void Stmt(Node* root, Type type){
    //type可以用来记录函数所需返回值的类型
    if (root == NULL) return;
    assert(root->num_children == 2||root->num_children == 1||root->num_children == 3||root->num_children == 5||root->num_children == 7);
    if(root->num_children == 1){//CompSt
        CompSt(get_child(root,0), type);
    }else if(root->num_children == 2){//Exp SEMI
        Exp(get_child(root, 0));
    }else if(root->num_children == 3){//RETURN Exp SEMI
        Type ret_type = Exp(get_child(root, 1));
        if(type != NULL && !isSameType(ret_type, type)){
            printf("Error type 8 at Line %d: Return type is not correct.\n", root->line);
        }
    }else if(root->num_children == 5){//IF LP Exp RP Stmt + WHILE LP Exp RP Stmt
        Exp(get_child(root, 2));
        Stmt(get_child(root, 4), type);
        if (strcmp(get_child(root, 0)->name, "IF") == 0){
            //
        }else if(strcmp(get_child(root, 0)->name, "WHILE") == 0){
            //
        }
    }else if(root->num_children == 7){//IF LP Exp RP Stmt ELSE Stmt
        Exp(get_child(root, 2));
        Stmt(get_child(root, 4), type);
        Stmt(get_child(root, 6), type);
    }
}

Type Specifier(Node* root){
    if (root == NULL) return NULL;
    assert(root->num_children == 1);
    Type type = NULL;
    if (strcmp(get_child(root, 0)->name, "TYPE") == 0){
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;
        if(strcmp(get_child(root, 0)->data, "int") == 0) type->u.basic = NUM_INT;
        if(strcmp(get_child(root, 0)->data, "float") == 0) type->u.basic = NUM_FLOAT;
    }
    if (strcmp(get_child(root, 0)->name, "StructSpecifier") == 0){
        type = StructSpecifier(get_child(root, 0));
    }
    return type;
}

Type StructSpecifier(Node* root){
    if (root == NULL) return NULL;
    assert(root->num_children == 5||root->num_children == 2);
    Type type = (Type)malloc(sizeof(struct Type_));
    FieldList fl = (FieldList)malloc(sizeof(struct FieldList_));
    fl->isArg = 0;
    if(root->num_children == 5){ //STRUCT OptTag LC DefList RC
        fl->name = OptTag(get_child(root, 1));
        
        if(fl->name != NULL){
            if(search(fl->name)!=NULL){
                printf("Error type 16 at Line %d: Duplicated name\n", root->line);
                return NULL;
            }
        }
        //type = (Type)malloc(sizeof(struct Type_));
        type->kind = STRUCTURE;
        type->u.structure = DefList(get_child(root, 3), 1);
        FieldList next_point = type->u.structure;
        //对type中的structure进行重复性检查，检查是否有重复的域内变量
        int i=1;
        while(next_point != NULL){
            FieldList first_point = type->u.structure;
            int j=i;
            while(first_point != next_point){
                if (strcmp(next_point->name, first_point->name) == 0) {
                    printf("Error type 15 at Line %d:Redefined member\n", root->line+j);
                    
                }
                first_point = first_point->tail;
                j++;
            }
            next_point = next_point->tail;
            i++;
        }
        fl->type = type;
        fl->tail = NULL;
        fl->isArg = 0;
        if(fl->name != NULL){
            // printf("insert Struct %s", fl->name);
            insert(fl);
        }

    }else if(root->num_children == 2){ //STRUCT Tag
        fl->name = OptTag(get_child(root, 1));
        FieldList f_st = search(fl->name);
        if(f_st==NULL){
            printf("Error type 17 at Line %d: Undefined structure\n", root->line);
            return NULL;
        }
        type = f_st->type;
    }
    return type;
}

char* OptTag(Node* root){
    if (root == NULL) return NULL;
    assert(root->num_children == 1);
    return get_child(root, 0)->data;
}

FieldList DefList(Node* root, int isStructure){
    //最后需要返回一个列表，里面有n个定义
    if (root == NULL) return NULL;
    assert(root->num_children == 2); //Def DefList
    FieldList fl = Def(get_child(root, 0), isStructure);
    FieldList point = fl;
    while(point->tail!=NULL){
        point = point->tail;
    }
    point->tail = DefList(get_child(root, 1), isStructure);
    return fl;
}

FieldList Def(Node* root, int isStructure){
    if (root == NULL) return NULL;
    assert(root->num_children == 3); // Def -> Specifier DecList SEMI
    Type type = Specifier(get_child(root, 0));
    FieldList fl = DecList(get_child(root, 1), type, isStructure);
    return fl;
}

FieldList DecList(Node* root, Type type, int isStructure) {
    if (root == NULL) return NULL;
    assert(root->num_children == 1 || root->num_children == 3);
    if (root->num_children == 1) {  // DecList -> Dec
        return Dec(get_child(root, 0), type, isStructure);
    } else if (root->num_children == 3) {  // DecList -> Dec COMMA DecList
        FieldList fl = Dec(get_child(root, 0), type, isStructure);
        FieldList point = fl;
        while(point->tail!=NULL){
            point = point->tail;
        }
        point->tail = DecList(get_child(root, 2), type, isStructure);
        return fl;
    }
    return NULL;
}

FieldList Dec(Node* root, Type type, int isStructure) {
    if (root == NULL) return NULL;
    assert(root->num_children == 1 || root->num_children == 3);
    if (root->num_children == 1) {  // Dec -> VarDec
        return VarDec(get_child(root, 0), type, isStructure);
    } else if (root->num_children == 3) {  // VarDec ASSIGNOP Exp
        if(isStructure){
            printf("Error type 15 at Line %d: can't Initialize in definition\n", root->line);
            return VarDec(get_child(root, 0), type, isStructure);
        }
        FieldList fl = VarDec(get_child(root, 0), type, isStructure);
        Type expType = Exp(get_child(root, 2));
        if(!isSameType(expType, type)){
            printf("Error type 5 at Line %d: Types don't match\n", root->line);
        }
        return fl;
    }
    return NULL;
}

Type Exp(Node* root){
    if (root == NULL) return NULL;
    //printf("into EXP, root->num_children = %d\n", root->num_children);
    assert(root->num_children == 1 || root->num_children == 2 || root->num_children == 3 || root->num_children == 4);
    Type type = NULL;
    if(root->num_children == 1){
        if(strcmp(get_child(root, 0)->name, "ID") == 0) {
            //printf("into EXP, root->num_children = %d, name = %s\n", root->num_children, get_child(root, 0)->data);
            FieldList fl = search(get_child(root, 0)->data);
            //printf("fl==NULL is %d, %s\n", fl==NULL, get_child(root, 0)->data);
            if (fl == NULL|| fl->type->kind == FUNCTION) {
                //好像不需要 || fl->type->kind == STRUCTURE？？？ID也可以是structure类型
                printf("Error type 1 at Line %d: undefined variable.\n", root->line);
            } else {
                type = fl->type;
            }
        }else if(strcmp(get_child(root, 0)->name, "INT") == 0){
            type = (Type)malloc(sizeof(struct Type_));
            type->kind = BASIC;
            type->u.basic = NUM_INT;
        }else if(strcmp(get_child(root, 0)->name, "FLOAT") == 0){
            type = (Type)malloc(sizeof(struct Type_));
            type->kind = BASIC;
            type->u.basic = NUM_FLOAT;
        }
    }
    if(root->num_children == 2){
        if (strcmp(get_child(root, 0)->name, "NOT") == 0) {
            type = Exp(get_child(root, 1));
            if (type != NULL && (type->kind != BASIC || type->u.basic != NUM_INT)) {
                printf("Error type 7 at Line %d: Non-int type cannot perform logical operations \n", root->line);
            }
        } else if (strcmp(get_child(root, 0)->name, "MINUS") == 0) {
            type = Exp(get_child(root, 1));
        }
    }
    if(root->num_children == 3){
        if (strcmp(get_child(root, 0)->name, "LP") == 0) {  // Exp -> LP Exp RP
            type = Exp(get_child(root, 1));
        }
        else if(strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP RP
            FieldList fl = search(get_child(root, 0)->data);
            if(fl == NULL){
                printf("Error type 2 at Line %d: undefined function\n", root->line);
            }else if(fl->type->kind != FUNCTION){
                printf("Error type 11 at Line %d: this is not a function\n", root->line);
            }else if(fl->type->u.function.arg_num != 0){
                printf("Error type 9 at Line %d: quality of arguments is not matched\n", root->line);
            }
            else if(fl->type->kind == FUNCTION){
                type = fl->type->u.function.ret;
            }
        }else if (strcmp(get_child(root, 1)->name, "DOT") == 0) {  // Exp -> Exp DOT ID
            Type structType = Exp(get_child(root, 0));
            if (structType == NULL) {
                return NULL;
            } else if(structType->kind != STRUCTURE) {
                printf("Error type 13 at Line %d: Illegal use of ','\n", root->line);
            } else {
                char* name = get_child(root, 2)->data;
                FieldList fl = structType->u.structure;
                int flag = 0;
                while(fl != NULL){
                    if(strcmp(fl->name, name) == 0){
                        flag = 1;
                        type = fl->type;
                        break;
                    }
                    fl = fl->tail;
                }
                if (flag == 0) {
                    printf("Error type 14 at Line %d: do not have this member variable %s\n", root->line, name);
                }
            }
        }
        else {
            type = Exp(get_child(root, 0));
            Type type2 = Exp(get_child(root, 2));
            if (!isSameType(type, type2)) {
                if(type2!=NULL&&strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0){
                    //type2等于NULL，说明已经有其他报错，不需要继续报错
                    printf("Error type 5 at Line %d: Types don't match for assignment\n", root->line);
                }else if(type2!=NULL) printf("Error type 7 at Line %d: Types don't match for operators\n", root->line);
            }
            if(strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0){
                Node *left = get_child(root, 0);
                if (!(( left->num_children == 1  && strcmp(get_child(left, 0)->name, "ID") == 0) ||
                      (left->num_children == 4 && strcmp(get_child(left, 1)->name, "LB") == 0 ) ||
                      (left->num_children == 3  && strcmp(get_child(left, 1)->name, "DOT") == 0))) {
                    printf("Error type 6 at Line %d: left value must be a variable\n", root->line);
                }
            }else if(strcmp(get_child(root, 1)->name, "AND") == 0 || strcmp(get_child(root, 1)->name, "OR") == 0){
                if(type != NULL && (type->kind != BASIC || type->u.basic != NUM_INT)){
                    printf("Error type 7 at Line %d: Non-int type cannot perform logical operations \n", root->line);
                }
            }else if(strcmp(get_child(root, 1)->name, "RELOP") == 0){
                //比较之后返回int值
                type = (Type) malloc(sizeof(struct Type_));
                type->kind = BASIC;
                type->u.basic = NUM_INT;
            }
            else if(strcmp(get_child(root, 1)->name, "PLUS") == 0
                    ||strcmp(get_child(root, 1)->name, "MINUS") == 0
                    ||strcmp(get_child(root, 1)->name, "STAR") == 0
                    ||strcmp(get_child(root, 1)->name, "DIV") == 0){

            }
        }
    }else if(root->num_children == 4){
        if(strcmp(get_child(root, 1)->name, "LB") == 0){ // Exp -> Exp LB Exp RB
            //数组访问
            Node* array = get_child(root, 0);
            if(strcmp(get_child(array, 0)->name, "ID") != 0){
                //左边不是一个变量，直接报错，对非变量使用[]
            }
            Type type1 = Exp(get_child(root, 0));
            if(type1 == NULL || type1->kind != ARRAY){
                printf("Error type 10 at Line %d: This is not a array \n", root->line);
            }else{
                type = type1->u.array.elem;
            }
            Type type2 = Exp(get_child(root, 2));
            if (type2 != NULL && (type2->kind != BASIC || type2->u.basic != NUM_INT)) {
                printf("Error type 12 at Line %d: This is not an integer \n", root->line);
            }
        }else if(strcmp(get_child(root, 0)->name, "ID") == 0){ // Exp -> ID LP Args RP
            //函数调用表达式，返回的type是函数的返回值
            char* name = get_child(root, 0)->data;
            FieldList fl = search(name);
            if(fl == NULL){
                printf("Error type 2 at Line %d: Undefined function \n", root->line);
                type = NULL;
            }else if(fl->type->kind != FUNCTION){
                printf("Error type 11 at Line %d: This is not a function \n", root->line);
                type = NULL;
            }else{
                FieldList needArg = fl->type->u.function.arg;//函数需要的参数
                FieldList realArg = Args(get_child(root, 2));//真实传入的参数
                int isMatch = 1;
                while(needArg != NULL || realArg != NULL){
                    if(needArg == NULL || realArg == NULL){
                        isMatch = 0;
                        break;
                    }
                    if(!isSameType(needArg->type, realArg->type)){
                        isMatch = 0;
                        break;
                    }
                    needArg = needArg->tail;
                    realArg = realArg->tail;
                }
                if(!isMatch){
                    printf("Error type 9 at Line %d: quality of arguments is not matched\n", root->line);
                }
                if(fl->type->kind == FUNCTION){
                    type = fl->type->u.function.ret;
                }
            }
        }

    }
    return type;
}

FieldList Args(Node* root){
    if(root == NULL) return NULL;
    assert(root->num_children == 1 || root->num_children == 3);

    Type type1 = Exp(get_child(root, 0));
    FieldList fl = (FieldList)malloc(sizeof(struct FieldList_));
    fl->isArg = 0;
    if(root->num_children == 1){  // Args->Exp
        fl->type = type1;
        fl->tail = NULL;
        fl->name = "arg";
    }else if(root->num_children == 3){  // Exp COMMA Args
        fl->type = type1;
        fl->tail = Args(get_child(root, 2));
        fl->name = "arg";
    }
    return fl;
}

int isSameType(Type type1, Type type2){
    //如果是数组递归对比到最后可能会出现一边NULL，一边不是NULL，这个时候说明维度不同
    if (type1 == NULL || type2 == NULL) return 0;
    if (type1->kind != type2->kind) return 0;
    if (type1 == type2) return 1;
    if(type1->kind == BASIC){
        return type1->u.basic == type2->u.basic;
    }
    if(type1->kind == ARRAY){
        return isSameType(type1->u.array.elem, type2->u.array.elem);
    }
    if(type1->kind == FUNCTION){
        //函数进行比较，还不知道会发生什么
        return 0;
    }
    if(type1->kind == STRUCTURE){
        FieldList member1 = type1->u.structure;
        FieldList member2 = type2->u.structure;
        while(member1 != NULL || member2 != NULL){
            if (member1 == NULL || member2 == NULL) return 0;
            if (isSameType(member1->type, member2->type) == 0) return 0;
            member1 = member1->tail;
            member2 = member2->tail;
        }
        return 1;
    }
    return 0;
}

void add_read_write_func(){
    // 其中read函数没有任何参数，返回值为int型（即读入的整数值），write函数包含一个int类型的参数（即要输出的整数值），返回值也为int型（固定返回0）
    char* read_name = "read";
    char* write_name = "write";
    Type read_type = (Type)malloc(sizeof(struct Type_));
    read_type->kind = FUNCTION;
    read_type->u.function.ret = (Type)malloc(sizeof(struct Type_));
    read_type->u.function.ret->kind = BASIC;
    read_type->u.function.ret->u.basic = NUM_INT;
    read_type->u.function.arg = NULL;
    read_type->u.function.arg_num = 0;
    FieldList read_fl = (FieldList)malloc(sizeof(struct FieldList_));
    read_fl->type = read_type;
    read_fl->name = read_name;
    read_fl->tail = NULL;
    read_fl->isArg = 0;
    insert(read_fl);

    Type write_type = (Type)malloc(sizeof(struct Type_));
    write_type->kind = FUNCTION;
    write_type->u.function.ret = (Type)malloc(sizeof(struct Type_));
    write_type->u.function.ret->kind = BASIC;
    write_type->u.function.ret->u.basic = NUM_INT;
    write_type->u.function.arg = (FieldList)malloc(sizeof(struct FieldList_));
    write_type->u.function.arg->type = (Type)malloc(sizeof(struct Type_));
    write_type->u.function.arg->type->kind = BASIC;
    write_type->u.function.arg->type->u.basic = NUM_INT;
    write_type->u.function.arg->tail = NULL;
    write_type->u.function.arg->name = "output_int";
    write_type->u.function.arg->isArg = 1;
    write_type->u.function.arg_num = 1;
    FieldList write_fl = (FieldList)malloc(sizeof(struct FieldList_));
    write_fl->type = write_type;
    write_fl->name = write_name;
    write_fl->tail = NULL;
    write_fl->isArg = 0;
    insert(write_fl);
}
