#include <stdio.h>
#include "ast.h"
#include "sysy.tab.hh"
#include "semantic.h"
#include <iostream>
#include "ir.h"
#include <vector>
#include <string>
#include <fstream>
using namespace std;
extern FILE* yyin;
extern int yyparse();
extern int semantic_analyse(pNode root);
extern void translate(pNode root);
extern int yylineno;
extern pNode root;
extern vector<string> ircode;
int yywrap();

int main(int argc, char **argv) {
    if(argc>0){
        if(!(yyin = fopen(argv[1],"r"))){
        perror(argv[1]);
        return 1;
        }
    }
    int x = yyparse();
    if(x){
        return 1;
    }else{
        printTreeInfo(root, 0);
        // x = semantic_analyse(root);
        // if(x==0){
        //     return 0;
        // }
        // else{
        //     return 1;
        // }
        translate(root);
        ofstream output(argv[2]);
        for(int i = 0; i < ircode.size(); i++)
        {
            output << ircode[i] << endl;
        //cout << translate_table.ircode[i] << endl;
        }
        // delete_tree(root,0);
    }
    return 0;
}
