#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "ast.h"
#include <unordered_map>
#include <string>
#include <vector>
typedef struct type* pType;
typedef struct funcparaList* pFuncparaList;
typedef enum _hashType {BASIC, ARRAY, FUNCTION,LABEL} HashType;
typedef enum _basicKind {INT_TYPE, VOID_TYPE} BasicKind;


typedef struct type{
    HashType kind;
    union{
        BasicKind basic;
        struct {
            pType elem;
            int size;
        } array;
        struct {
            int argc;
            pFuncparaList argv;
            pType returnType;
        }function;
        int label;
    } u;
} Type;

typedef struct funcparaList {
    char* name; 
    pType type; 
    pFuncparaList tail; 
} FuncparaList;

extern std::unordered_map <std::string,std::vector<pType>>hash_table;
extern vector<pNode>globelvar_func;

// 函数声明
pType newType(HashType kind, BasicKind b, ...);
bool checkType(pType type1, pType type2);
bool checkall(string s);
bool checkin(string s);
void add_element(string s, pType t);
void printtable();
void push_label();
void pop_label();
pType findkey(string s);
pFuncparaList newFuncparaList(char* newName, pType newType);
void add_func_para(pType func, pFuncparaList p);
pType analyseExps(pNode node);
pType analyseLVal(pNode node);
pType analysePrimaryExp(pNode node);
void analyseFuncRParams(pNode node, vector<pType> &v, vector<int> &temp1, vector<string> &temp2);
pType analyseUnaryExp(pNode node);
pType analyseMulExp(pNode node);
pType analyseAddExp(pNode node);
pType analyseRelExp(pNode node);
pType analyseEqExp(pNode node);
pType analyseLAndExp(pNode node);
pType analyseLOrExp(pNode node);
pType analyseExp(pNode node);
pType analyseArray(pNode node, pType p);
void analyseVarDef(pNode node);
void analyseVarDeflist(pNode node);
void analyseVarDecl(pNode node);
pType analyseFPDimension(pNode node);
pFuncparaList analyseFuncFParam(pNode node);
void analyseFuncFParams(pNode node, pType func);
void analyseStmt(pNode node, pType returnType);
void analyseBlockItem(pNode node, pType returnType);
void analyseBlockItems(pNode node, pType returnType);
void analyseBlock(pNode node, pType returnType, pType func);
void analyseDecl(pNode node);
void analyseFuncDef(pNode node);
void analyseCompUnit(pNode node);
void hash_init();
int semantic_analyse(pNode root);


#endif