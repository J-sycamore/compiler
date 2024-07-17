#include<vector>
#include<unordered_map>
#include<stdio.h>
#include<string>
#include<iostream>
#include <stdarg.h>
#include "ast.h"
#include "semantic.h"
using namespace std;
#define DEBUG 0
unordered_map<string, vector<pType>> hash_table;
vector<pNode>globelvar_func;
int res = 0;
int paralevel;
string temp_ID;
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

// 初始化新元素
pType newType(HashType kind,BasicKind b, ...){
    pType p = (pType)malloc(sizeof(Type));
    p->kind = kind;
    va_list vaList;
    assert(kind == BASIC || kind == ARRAY || kind == FUNCTION||kind == LABEL);
    if(kind==BASIC){
        p->u.basic = b;
    }
    else if(kind==ARRAY){
        va_start(vaList, 3);
        p->u.array.elem = va_arg(vaList, pType);
        p->u.array.size = va_arg(vaList, int);
    }
    else if(kind==FUNCTION){
        va_start(vaList, 3);
        p->u.function.argc = va_arg(vaList, int);
        p->u.function.argv = va_arg(vaList, pFuncparaList);
        p->u.function.returnType = va_arg(vaList, pType);
    }
    else if(kind==LABEL){
        p->u.label = -1;
    }
    va_end(vaList);
    return p;
}

// 比较两个pType是否相等
bool checkType(pType type1, pType type2){
    if(type1==NULL||type2==NULL){
        return true;
    }
    else if(type1->kind!=type2->kind){
        return false;
    }
    if(type1->kind==FUNCTION||type2->kind==FUNCTION){
        return false;
    }
    else{
        if(type1->kind==BASIC){
            return type1->u.basic==type2->u.basic;
        }
        else{
            // 数组需要递归check，elem是下一层的信息
            // 等于0需要单独判断，因为函数形参会有a[][1][1]之类的
            if(type1->u.array.size == 0 || type2->u.array.size == 0){
                return checkType(type1->u.array.elem, type2->u.array.elem);
            }
            else{
                if(type1->u.array.size!=type2->u.array.size){
                    return false;
                }
                return checkType(type1->u.array.elem, type2->u.array.elem);
            }
        }
    }
}

// 检查当前域的符号表是否有这个符号
bool checkall(string s){
    if(hash_table.find(s)==hash_table.end()){
        return false;
    }
    else{
        if((hash_table[s]).back()->kind==LABEL){
            return false;
        }
        else{
            return true;
        }
    }
}

// 检查所有域是否有这个符号
bool checkin(string s){
    if(hash_table.find(s)!=hash_table.end()){
        return true;
    }
    else{
        return false;
    }
}

// 向符号表加入一个元素
void add_element(string s,pType t){
    bool p = checkin(s);
    if(p){
        hash_table[s].push_back(t);
    }
    else{
        vector<pType> temp;
        temp.push_back(t);
        hash_table[s] = temp;
    }
}

// 简单打印符号表，debug使用
void printtable(){
    cout<<"\nTABLE:\n";
    for (auto it = hash_table.begin(); it != hash_table.end(); ++it) {
        cout<<"key:"<<it->first<<" ";
        for(int i = 0;i<(it->second).size();i++){
            cout<<(it->second)[i]->kind<<" ";
        }
        cout<<endl;
    }
    cout<<"TABLE:\n";
}

// 进入一个新的域，将LABEL push进符号表
// LABEL是新域和旧域之间的一个mask
void push_label(){
    for (auto it = hash_table.begin(); it != hash_table.end(); ++it) {
        pType p = newType(LABEL, INT_TYPE);
        it->second.push_back(p);
    }
}

// 离开一个域，将域中定义的符号和LABEL删除
void pop_label(){
    vector<string> temp;
    for (auto it = hash_table.begin(); it != hash_table.end(); ++it) {
        pType p = it->second.back();
        while(p->kind!=LABEL&&it->second.size()>1){
            it->second.pop_back();
            p = it->second.back();
        }
        it->second.pop_back();
        if(it->second.size()==0){
            temp.push_back(it->first);
        }
    }
    for(int i=0;i<temp.size();i++){
        hash_table.erase(temp[i]);
    }
}

// 通过key查找符号表对应元素
pType findkey(string s){
    if(checkin(s)){
        int len = hash_table[s].size()-1;
        pType ans = hash_table[s][len];
        while(ans->kind==LABEL&&len>0){
            len--;
            ans = hash_table[s][len];
        }
        return ans;
    }
    else{
        return NULL;
    }
}

// 初始化一个参数表
pFuncparaList newFuncparaList(char* Name, pType newType){
    pFuncparaList p = (pFuncparaList)malloc(sizeof(FuncparaList));
    p->name = Name;
    p->type = newType;
    p->tail = NULL;
    return p;
}

// 向当前参数表加入一个参数
void add_func_para(pType func,pFuncparaList p){
    if(func->u.function.argc==0){
        func->u.function.argv = p;
    }
    else{
        pFuncparaList temp = func->u.function.argv;
        while(temp->tail!=NULL){
            temp = temp->tail;
        }
        temp->tail = p;
    }
    func->u.function.argc += 1;
}

// Exps:Exps LB Exp RB                                 {$$ = newNode(@$.first_line,NOT_TOKEN,"Exps1",4,$1,$2,$3,$4);}
//     |LB Exp RB                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"Exps2",3,$1,$2,$3);}
//     ;
pType analyseExps(pNode node){
    pType p,q;
    if(strcmp(node->name,"Exps1")==0){
        if(DEBUG) printf("Exps1\n");
        p = analyseExps(node->child);
        q = analyseExp(node->child->next->next);
    }
    else if(strcmp(node->name,"Exps2")==0){
        if(DEBUG) printf("Exps2\n");
        p = analyseExp(node->child->next);
        if(p->kind==ARRAY){
            res = 1;
            cout<<"220"<<endl;
        }
    }
    return p;
}

// LVal:ID                                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LVal1",1,$1);}
//     |ID Exps                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"LVal2",2,$1,$2);}
//     ;
pType analyseLVal(pNode node){
    pType p;
    string ss(node->child->value);
    temp_ID = ss;
    if(strcmp(node->name,"LVal1")==0){
        if(DEBUG) printf("LVal1\n");
        if(checkin(node->child->value)){
            p = hash_table[node->child->value].back();
            int len = hash_table[node->child->value].size();
            while(p->kind==LABEL&&len>0){
                len--;
                p = hash_table[node->child->value][len];
            }
        }
        else{
            p = NULL;
            res = 1;
            cout<<"241"<<endl;
        }
    }
    else if(strcmp(node->name,"LVal2")==0){
        if(DEBUG) printf("LVal2\n");
        string s(node->child->value);
        pType temp = findkey(s);
        if(temp->kind==BASIC){
            res = 1;
            cout<<"250"<<endl;
        }
        p = analyseExps(node->child->next);
    }
    return p;
}

// PrimaryExp: LP Exp RP                               {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp1",3,$1,$2,$3);}
//     | LVal                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp2",1,$1);}
//     | Number                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp3",1,$1);}
//     ;
pType analysePrimaryExp(pNode node){
    pType p;
    if(strcmp(node->name,"PrimaryExp1")==0){
        if(DEBUG) printf("PrimaryExp1\n");
        p = analyseExp(node->child->next);
    }
    else if(strcmp(node->name,"PrimaryExp2")==0){
        if(DEBUG) printf("PrimaryExp2\n");
        p = analyseLVal(node->child);
    }
    else if(strcmp(node->name,"PrimaryExp3")==0){
        if(DEBUG) printf("PrimaryExp3\n");
        p = newType(BASIC, INT_TYPE);
    }
    return p;
}

// FuncRParams:Exp                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncRParams1",1,$1);}
//     |Exp COMMA FuncRParams                          {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncRParams2",3,$1,$2,$3);}
//     ;
void analyseFuncRParams(pNode node,vector<pType> &v,vector<int> &temp1,vector<string>& temp2){
    paralevel = 0;
    if(strcmp(node->name,"FuncRParams1")==0){
        if(DEBUG) printf("FuncRParams1\n");
        v.push_back(analyseExp(node->child));
        temp1.push_back(paralevel);
        temp2.push_back(temp_ID);
    }
    else if(strcmp(node->name,"FuncRParams2")==0){
        if(DEBUG) printf("FuncRParams2\n");
        v.push_back(analyseExp(node->child));
        temp1.push_back(paralevel);
        temp2.push_back(temp_ID);
        analyseFuncRParams(node->child->next->next,v,temp1,temp2);
    }
}

// UnaryExp:PrimaryExp                                 {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp1",1,$1);}
//     | ID LP FuncRParams RP                          {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp2",4,$1,$2,$3,$4);}
//     | ID LP RP                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp3",3,$1,$2,$3);}
//     | UnaryOp UnaryExp                              {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp4",2,$1,$2);}
//     ;
pType analyseUnaryExp(pNode node){
    pType p = NULL;
    if(strcmp(node->name,"UnaryExp1")==0){
        if(DEBUG) printf("UnaryExp1\n");
        p = analysePrimaryExp(node->child);
    }
    else if(strcmp(node->name,"UnaryExp2")==0){
        if(DEBUG) printf("UnaryExp2\n");
        string ID(node->child->value);
        if(!checkin(ID)){
            res = 1;
            cout<<"302"<<endl;
        }
        if(findkey(ID)->kind!=FUNCTION){
            res = 1;
            cout<<"306"<<endl;
        }
        vector<pType> temp;
        vector<int> temp1;
        vector<string> temp2;
        analyseFuncRParams(node->child->next->next,temp,temp1,temp2);
        int temp_label = 0;
        int len = temp.size();
        int avgc = findkey(ID)->u.function.argc;
        if(len!=avgc){
            res = 1;
            cout<<"326"<<endl;
        }
        else{            
            pFuncparaList x = findkey(ID)->u.function.argv;
            for(int i=0;i<len;i++){
                if(temp[i]==NULL){
                    temp_label = 1;
                    break;
                }
                if(temp[i]->kind==FUNCTION){
                    pType xx = temp[i]->u.function.returnType;
                    if(xx->kind!=x->type->kind){
                        cout<<"360"<<endl;
                        res = 1;
                    }
                    x =x->tail;
                    continue;
                }
                pType tt = findkey(temp2[i]);
                if(tt->kind==ARRAY&&x->type->kind==ARRAY){
                    pType pp = x->type;
                    for(int j=0;j<temp1[i]-1;j++){
                        
                        tt = tt->u.array.elem;
                        pp = pp->u.array.elem;
                    }
                    if(!checkType(tt,x->type)){
                        res = 1;
                        cout<<"331"<<endl;
                    }
                }
                
                x =x->tail;
            }
        }
        
    }
    else if(strcmp(node->name,"UnaryExp3")==0){
        if(DEBUG) printf("UnaryExp3\n");
        pType ans = newType(FUNCTION,VOID_TYPE,0,newFuncparaList("temp",newType(BASIC,VOID_TYPE)),newType(BASIC,VOID_TYPE));
        return ans;
    }
    else if(strcmp(node->name,"UnaryExp4")==0){
        if(DEBUG) printf("UnaryExp4\n");
        p = analyseUnaryExp(node->child->next);
    }
    return p;
}

// MulExp: UnaryExp                                    {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp1",1,$1);}
//     | MulExp STAR UnaryExp                          {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp2",3,$1,$2,$3);}
//     | MulExp DIV UnaryExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp3",3,$1,$2,$3);}
//     | MulExp MOD UnaryExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp4",3,$1,$2,$3);}
//     ;
pType analyseMulExp(pNode node){
    pType p,q;
    if(strcmp(node->name,"MulExp1")==0){
        if(DEBUG) printf("MulExp1\n");
        p = analyseUnaryExp(node->child);
    }
    else if(strcmp(node->name,"MulExp2")==0){
        if(DEBUG) printf("MulExp2\n");
        p = analyseMulExp(node->child);
        q = analyseUnaryExp(node->child->next->next);
        if(!checkType(p,q)){
            res = 1;
            cout<<"340"<<endl;
        }
    }
    else if(strcmp(node->name,"MulExp3")==0){
        if(DEBUG) printf("MulExp3\n");
        p = analyseMulExp(node->child);
        q = analyseUnaryExp(node->child->next->next);
    }
    else if(strcmp(node->name,"MulExp4")==0){
        if(DEBUG) printf("MulExp4\n");
        p = analyseMulExp(node->child);
        q = analyseUnaryExp(node->child->next->next);
    }
    return p;
}

// AddExp:MulExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp1",1,$1);}
//     | AddExp PLUS MulExp                            {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp2",3,$1,$2,$3);}
//     | AddExp MINUS MulExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp3",3,$1,$2,$3);}
//     ;
pType analyseAddExp(pNode node){
    pType p,q;
    if(strcmp(node->name,"AddExp1")==0){
        if(DEBUG) printf("AddExp1\n");
        p = analyseMulExp(node->child);
    }
    else if(strcmp(node->name,"AddExp2")==0){
        if(DEBUG) printf("AddExp2\n");
        p = analyseAddExp(node->child);
        q = analyseMulExp(node->child->next->next);
    }
    else if(strcmp(node->name,"AddExp3")==0){
        if(DEBUG) printf("AddExp3\n");
        p = analyseAddExp(node->child);
        q = analyseMulExp(node->child->next->next);
    }
    return p;
}

// RelExp:AddExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"RelExp1",1,$1);}
//     | RelExp RELOP AddExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"RelExp2",3,$1,$2,$3);}
//     ;
pType analyseRelExp(pNode node){
    pType p,q;
    if(strcmp(node->name,"RelExp1")==0){
        if(DEBUG) printf("RelExp1\n");
        p = analyseAddExp(node->child);
    }
    else if(strcmp(node->name,"RelExp2")==0){
        if(DEBUG) printf("RelExp2\n");
        p = analyseRelExp(node->child);
        q = analyseAddExp(node->child->next->next);
    }
    return p;
}

// EqExp:RelExp                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"EqExp1",1,$1);}
//     | EqExp EQOP RelExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"EqExp2",3,$1,$2,$3);}
//     ; 
pType analyseEqExp(pNode node){
    pType p,q;
    if(strcmp(node->name,"EqExp1")==0){
        if(DEBUG) printf("EqExp1\n");
        p = analyseRelExp(node->child);
    }
    else if(strcmp(node->name,"EqExp2")==0){
        if(DEBUG) printf("EqExp2\n");
        p = analyseEqExp(node->child);
        q = analyseRelExp(node->child->next->next);
    }
    return p;
}

// LAndExp:EqExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"LAndExp1",1,$1);}
//     | LAndExp AND EqExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LAndExp2",3,$1,$2,$3);}
//     ;
pType analyseLAndExp(pNode node){
    pType p,q;
    if(strcmp(node->name,"LAndExp1")==0){
        if(DEBUG) printf("LAndExp1\n");
        p = analyseEqExp(node->child);
    }
    else if(strcmp(node->name,"LAndExp2")==0){
        if(DEBUG) printf("LAndExp2\n");
        p = analyseLAndExp(node->child);
        q = analyseEqExp(node->child->next->next);
    }
    return p;
}

// LOrExp: LAndExp                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"LOrExp1",1,$1);}
//     | LOrExp OR LAndExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LOrExp2",3,$1,$2,$3);}
//     ;
pType analyseLOrExp(pNode node){
    pType p,q;
    if(strcmp(node->name,"LOrExp1")==0){
        if(DEBUG) printf("LOrExp1\n");
        p = analyseLAndExp(node->child);
    }
    else if(strcmp(node->name,"LOrExp2")==0){
        if(DEBUG) printf("LOrExp2\n");
        p = analyseLOrExp(node->child);
        q = analyseLAndExp(node->child->next->next);
    }
    return p;
}

// Exp:LOrExp                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"Exp",1,$1);}
//     ;
pType analyseExp(pNode node){
    if(DEBUG) printf("Exp\n");
    paralevel++;
    pType p;
    p = analyseLOrExp(node->child);
    return p;
}

// Array:LB INT RB Array                               {$$ = newNode(@$.first_line,NOT_TOKEN,"Array1",4,$1,$2,$3,$4);}
//     | LB INT RB                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"Array2",3,$1,$2,$3);}
//     ;
pType analyseArray(pNode node,pType p){
    if(node==NULL){
        return NULL;
    }
    p->u.array.elem = newType(ARRAY,VOID_TYPE,newType(ARRAY,VOID_TYPE,newType(LABEL,VOID_TYPE),0),atoi(node->child->next->value));
    analyseArray(node->child->next->next->next,p->u.array.elem);
    return p;
}

// VarDef:ID ASSIGNOP Exp                              {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef1",3,$1,$2,$3);}
//     | ID Array                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef2",2,$1,$2);}
//     | ID                                            {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef3",1,$1);}
//     ;
void analyseVarDef(pNode node){
    pType p;
    if(strcmp(node->name,"VarDef1")==0){
        if(DEBUG) printf("VarDef1\n");
        string ID(node->child->value);
        if(checkall(ID)){
            res = 1;
            cout<<"464"<<endl;
        }
        p = analyseExp(node->child->next->next);
        add_element(ID,newType(BASIC,INT_TYPE));
    }
    else if(strcmp(node->name,"VarDef2")==0){
        if(DEBUG) printf("VarDef2\n");
        string ID(node->child->value);
        if(checkall(ID)){
            res = 1;
            cout<<"474"<<endl;
        }
        p = newType(ARRAY,VOID_TYPE,NULL,atoi(node->child->next->child->next->value));
        pNode t = node->child->next->child->next->next->next;
         
        if(t!=NULL){
            p->u.array.elem = newType(ARRAY,VOID_TYPE,NULL,atoi(t->child->next->value));
            t = t->child->next->next->next;
        }
        pType x = p->u.array.elem;
        if(t!=NULL){
            x->u.array.elem = newType(ARRAY,VOID_TYPE,NULL,atoi(t->child->next->value));
        }
        add_element(ID,p);
    }
    else if(strcmp(node->name,"VarDef3")==0){
        if(DEBUG) printf("VarDef3\n");
        string ID(node->child->value);
        if(checkall(ID)){
            if(findkey(ID)->kind==BASIC){
                res = 1;
                cout<<"489"<<endl;
            }
        }
        add_element(ID,newType(BASIC,INT_TYPE));
    }
}

// VarDeflist:VarDef                                   {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDeflist1",1,$1);}
//     |VarDef COMMA VarDeflist                        {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDeflist2",3,$1,$2,$3);}
//     ;
void analyseVarDeflist(pNode node){
    if(strcmp(node->name,"VarDeflist1")==0){
        if(DEBUG) printf("VarDeflist1\n");
        analyseVarDef(node->child);
    }
    else if(strcmp(node->name,"VarDeflist2")==0){
        if(DEBUG) printf("VarDeflist2\n");
        analyseVarDef(node->child);
        analyseVarDeflist(node->child->next->next);
    }
}

// VarDecl:TYPE VarDeflist SEMI                         {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDecl",3,$1,$2,$3);}
//     ;
void analyseVarDecl(pNode node){
    if(DEBUG) printf("VarDecl\n");
    analyseVarDeflist(node->child->next);
}

// FPDimension: LB INT RB                              { $$ = newNode(@$.first_line,NOT_TOKEN,"FPDimension1",3,$1,$2,$3);}
//     | LB INT RB FPDimension                         { $$ = newNode(@$.first_line,NOT_TOKEN,"FPDimension2",4,$1,$2,$3,$4);}
//     ;
pType analyseFPDimension(pNode node){
    if(strcmp(node->name,"FPDimension1")==0){
        if(DEBUG) printf("FPDimension1\n");
    }
    else if(strcmp(node->name,"FPDimension2")==0){
        if(DEBUG) printf("FPDimension2\n");
        analyseFPDimension(node->child->next->next->next);
    }
}

// FuncFParam:TYPE ID LB RB                            {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam1",4,$1,$2,$3,$4);}
//     | TYPE ID                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam2",2,$1,$2);}
//     | TYPE ID LB RB FPDimension                     {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam3",5,$1,$2,$3,$4,$5);}
//     ;
pFuncparaList analyseFuncFParam(pNode node){
    pType paratype = newType(BASIC, INT_TYPE);
    pNode id = node->child->next;
    pFuncparaList FuncparaList = newFuncparaList(id->value, NULL);
    if(strcmp(node->name,"FuncFParam1")==0){
        if(DEBUG) printf("FuncFParam1\n");
        FuncparaList->type = newType(ARRAY, INT_TYPE, paratype, 0);
    }
    else if(strcmp(node->name,"FuncFParam2")==0){
        if(DEBUG) printf("FuncFParam2\n");
        FuncparaList->type = paratype;
    }
    else if(strcmp(node->name,"FuncFParam3")==0){
        if(DEBUG) printf("FuncFParam3\n");
        FuncparaList->type = newType(ARRAY, INT_TYPE, NULL, 0);
        pNode p = node->child->next->next->next->next;
        if(p!=NULL){
            FuncparaList->type->u.array.elem = newType(ARRAY, INT_TYPE, NULL, atoi(p->child->next->value));
            p = p->child->next->next->next;
        }
        pType x = FuncparaList->type->u.array.elem;
        if(p!=NULL){
            x->u.array.elem =  newType(ARRAY, INT_TYPE, NULL, atoi(p->child->next->value));
        }
        
    }
    return FuncparaList;
}

// FuncFParams:FuncFParam                              {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParams1",1,$1);}
//     | FuncFParam COMMA FuncFParams                  {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParams2",3,$1,$2,$3);}
//     ;
void analyseFuncFParams(pNode node,pType func){
    pFuncparaList p;
    if(strcmp(node->name,"FuncFParams1")==0){
        if(DEBUG) printf("FuncFParams1\n");
        p = analyseFuncFParam(node->child);
        add_func_para(func,p);
    }
    else if(strcmp(node->name,"FuncFParams2")==0){
        if(DEBUG) printf("FuncFParams2\n");
        p = analyseFuncFParam(node->child);
        add_func_para(func,p);
        analyseFuncFParams(node->child->next->next,func);
    }
}

// Stmt:LVal ASSIGNOP Exp SEMI                         {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt1",4,$1,$2,$3,$4);}
//     |Exp SEMI                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt2",2,$1,$2);}
//     |Block                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt3",1,$1);}
//     |IF LP Exp RP Stmt                              {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt4",5,$1,$2,$3,$4,$5);}
//     |IF LP Exp RP Stmt ELSE Stmt                    {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt5",7,$1,$2,$3,$4,$5,$6,$7);}
//     |WHILE LP Exp RP Stmt                           {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt6",5,$1,$2,$3,$4,$5);}
//     |BREAK SEMI                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt7",2,$1,$2);}
//     |CONTINUE SEMI                                  {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt8",2,$1,$2);}
//     |RETURN SEMI                                    {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt9",2,$1,$2);}
//     |RETURN Exp SEMI                                {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt10",3,$1,$2,$3);}
//     ;
void analyseStmt(pNode node,pType returnType){
    pType p,q;
    if(strcmp(node->name,"Stmt1")==0){
        if(DEBUG) printf("Stmt1\n");
        p = analyseLVal(node->child);
        q = analyseExp(node->child->next->next);
        if(q!=NULL&&q->kind==ARRAY){
            res = 1;
            cout<<"581"<<endl;
        }
        if(q!=NULL&&q->kind==FUNCTION&&q->u.function.returnType->u.basic==VOID_TYPE){
            res = 1;
            cout<<"585"<<endl;
        }
    }
    else if(strcmp(node->name,"Stmt2")==0){
        if(DEBUG) printf("Stmt2\n");
        analyseExp(node->child);
    }
    else if(strcmp(node->name,"Stmt3")==0){
        if(DEBUG) printf("Stmt3\n");
        analyseBlock(node->child,returnType,NULL);
    }
    else if(strcmp(node->name,"Stmt4")==0){
        if(DEBUG) printf("Stmt4\n");
        analyseExp(node->child->next->next);
        analyseStmt(node->child->next->next->next->next,returnType);
    }
    else if(strcmp(node->name,"Stmt5")==0){
        if(DEBUG) printf("Stmt5\n");
        analyseExp(node->child->next->next);
        analyseStmt(node->child->next->next->next->next,returnType);
        analyseStmt(node->child->next->next->next->next->next->next,returnType);
    }
    else if(strcmp(node->name,"Stmt6")==0){
        if(DEBUG) printf("Stmt6\n");
        analyseExp(node->child->next->next);
        analyseStmt(node->child->next->next->next->next,returnType);
    }
    else if(strcmp(node->name,"Stmt9")==0){
        if(DEBUG) printf("Stmt9\n");
        if(returnType->u.basic!=VOID_TYPE){
            res = 1;
            cout<<"617"<<endl;
        }
    }
    else if(strcmp(node->name,"Stmt10")==0){
        if(DEBUG) printf("Stmt10\n");
        analyseExp(node->child->next);
    }
}

// BlockItem:Decl                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItem1",1,$1);}
//     |Stmt                                           {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItem2",1,$1);}
//     ;
void analyseBlockItem(pNode node,pType returnType){
    if(strcmp(node->name,"BlockItem1")==0){
        if(DEBUG) printf("BlockItem1\n");
        analyseDecl(node->child);
    }
    else if(strcmp(node->name,"BlockItem2")==0){
        if(DEBUG) printf("BlockItem2\n");
        analyseStmt(node->child,returnType);
    }
}

// BlockItems: BlockItem                               {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItems1",1,$1);}         
//     | BlockItem BlockItems                          {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItems2",2,$1,$2);}       
//     ;
void analyseBlockItems(pNode node,pType returnType){
    if(strcmp(node->name,"BlockItems1")==0){
        if(DEBUG) printf("BlockItems1\n");
        analyseBlockItem(node->child,returnType);
    }
    else if(strcmp(node->name,"BlockItems2")==0){
        if(DEBUG) printf("BlockItems2\n");
        analyseBlockItem(node->child,returnType);
        analyseBlockItems(node->child->next,returnType);
    }

}

// Block:LC BlockItems RC                              {$$ = newNode(@$.first_line,NOT_TOKEN,"Block",3,$1,$2,$3);}
//     ;
void analyseBlock(pNode node,pType returnType,pType func){
    if(DEBUG) printf("Block\n");
    push_label();
    if(func!=NULL){
        int len = func->u.function.argc;
        pFuncparaList temp = func->u.function.argv;
        for(int i=0;i<len;i++){
            string ID(temp->name);
            add_element(ID,temp->type);
            temp = temp->tail;
        }
    }
    analyseBlockItems(node->child->next,returnType);
    pop_label();
}

// Decl: VarDecl                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"Decl",1,$1);}
//     ;
void analyseDecl(pNode node){
    if(DEBUG) printf("Decl\n");
    analyseVarDecl(node->child);
}

// FuncDef:TYPE ID LP FuncFParams RP Block          {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef1",6,$1,$2,$3,$4,$5,$6);}
//     |TYPE ID LP RP Block                         {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef2",5,$1,$2,$3,$4,$5);}
//     |VOID ID LP FuncFParams RP Block             {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef3",6,$1,$2,$3,$4,$5,$6);}
//     |VOID ID LP RP Block                         {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef4",5,$1,$2,$3,$4,$5);}
//     ;
void analyseFuncDef(pNode node){
    BasicKind b;
    globelvar_func.push_back(node);
    if(strcmp(node->child->value,"int")==0){
        b = INT_TYPE;
    }
    else{
        b = VOID_TYPE;
    }
    string ID(node->child->next->value);
    if(checkall(ID)){
        res = 1;
        cout<<"687"<<endl;
    }
    pType p = newType(BASIC, b);
    pType func = newType(FUNCTION, INT_TYPE, 0, NULL, p);
    if(strcmp(node->name,"FuncDef1")==0){
        if(DEBUG) printf("FuncDef1\n");
        analyseFuncFParams(node->child->next->next->next,func);
        add_element(node->child->next->value,func);
        analyseBlock(node->child->next->next->next->next->next,p,func);
    }
    else if(strcmp(node->name,"FuncDef2")==0){
        if(DEBUG) printf("FuncDef2\n");
        add_element(node->child->next->value,func);
        analyseBlock(node->child->next->next->next->next,p,func);
    }
    else if(strcmp(node->name,"FuncDef3")==0){
        if(DEBUG) printf("FuncDef3\n");
        analyseFuncFParams(node->child->next->next->next,func);
        add_element(node->child->next->value,func);
        analyseBlock(node->child->next->next->next->next->next,p,func);
    }
    else if(strcmp(node->name,"FuncDef4")==0){
        if(DEBUG) printf("FuncDef4\n");
        add_element(node->child->next->value,func);
        analyseBlock(node->child->next->next->next->next,p,func);
    }
}

// CompUnit: Decl CompUnit                             {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit1",2,$1,$2);}
//     | FuncDef CompUnit                              {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit2",2,$1,$2);}
//     | Decl                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit3",1,$1);}
//     | FuncDef                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit4",1,$1);}
//     ;
void analyseCompUnit(pNode node){
    
    if(strcmp(node->name,"CompUnit1")==0){
        if(DEBUG) printf("CompUnit1\n");
        analyseDecl(node->child);
        globelvar_func.push_back(node->child->child);
        analyseCompUnit(node->child->next);
    }
    else if(strcmp(node->name,"CompUnit2")==0){
        if(DEBUG) printf("CompUnit2\n");
        analyseFuncDef(node->child);
        analyseCompUnit(node->child->next);
    }
    else if(strcmp(node->name,"CompUnit3")==0){
        if(DEBUG) printf("CompUnit3\n");
        analyseDecl(node->child);
        globelvar_func.push_back(node->child->child);
    }
    else if(strcmp(node->name,"CompUnit4")==0){
        if(DEBUG) printf("CompUnit4\n");
        analyseFuncDef(node->child);
    }
}

void hash_init(){
    pType p1 = newType(BASIC, VOID_TYPE);
    pType func1 = newType(FUNCTION, INT_TYPE, 1, newFuncparaList("number",newType(BASIC, INT_TYPE)), p1);
    add_element("putint",func1);
    pType p2 = newType(BASIC, INT_TYPE);
    pType func2 = newType(FUNCTION, INT_TYPE, 0, NULL, p2);
    add_element("getint",func2);
    pType p3 = newType(BASIC, INT_TYPE);
    pType func3 = newType(FUNCTION, INT_TYPE, 0, NULL, p3);
    add_element("getch",func3);
}

int semantic_analyse(pNode root){
    hash_init();
    analyseCompUnit(root->child);
    // cout<<"len:"<<globelvar_func.size()<<endl;
    // for(int i=0;i<globelvar_func.size();i++){
    //     cout<<globelvar_func[i]->name<<endl;
    // }
    return res;
}
