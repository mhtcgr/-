# include "symtab.h"

HashNode hashtable[TABLE_SIZE + 1];

void init(){
	for(int i=0;i<TABLE_SIZE;i++){
		hashtable[i] = NULL;
	}
}

void print_hashtab(){
    for(int i=0;i<TABLE_SIZE;i++){
        if(hashtable[i]!=NULL){
            printf("%s i=%d\n", hashtable[i]->data->name, i);
            if(hashtable[i]->data->type->kind == STRUCTURE){
            	FieldList fl = hashtable[i]->data->type->u.structure;
                while(fl!=NULL){
                    printf("%s\n", fl->name);
                    fl = fl->tail;
                }
                
            }
        }
    }
    printf("\n");
}

unsigned int hash(char* name) {
    unsigned int val = 0, i;
    for (; *name; name++) {
        val = (val << 2) + *name;
        if ((i = val & ~TABLE_SIZE)) val = (val ^ (i >> 12)) & TABLE_SIZE;
    }
    assert(val <= TABLE_SIZE);
    return val;
}

void insert(FieldList field){
	int hashnum = (int)hash(field->name);
	HashNode h = (HashNode)malloc(sizeof(struct HashNode_));
	assert(h!=NULL);
	h->data=field;
	h->next=NULL;
	if(hashtable[hashnum]==NULL){
		hashtable[hashnum] = h;
	}
	else{
		HashNode n = hashtable[hashnum]->next;
		while(n->next!=NULL){
			n=n->next;
		}
		n->next=h;
	}
}

FieldList search(char* name){
	int hashnum = (int)hash(name);
	if(hashtable[hashnum]==NULL){
		return NULL;
	}
	else{
		HashNode n = hashtable[hashnum];
		while(n!=NULL){
			if(strcmp(n->data->name, name)==0){
			return n->data;
			}
			n=n->next;
		}
		return NULL;
	}
}

