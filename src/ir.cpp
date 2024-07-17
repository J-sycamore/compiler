#include <vector>
#include <iostream>
#include "ast.h"
#include<string>
#include<unordered_map>
#include<string.h>
using namespace std;
#define DEBUG 1
vector<string> ircode;
int regnum = 1;
int if_scope = 0;
int while_scope = -1;
int paranum;
int shortscope = 0;
string tranExp(pNode node);
void tranStmt(pNode node);
void tranBlock(pNode node);
string get_dim(string array_name);
vector<pair<string,int>>para_list;
unordered_map<string,int>func_list;
unordered_map<string,vector<int>>array_list;
vector<string> RParam;
vector<int>array_dem;
vector<string>Rdem;
vector<string>globel_init;
unordered_map<string,vector<int>>func_dem_array;
vector<int>func_dem_array_item;

string changeformat(string s){
    if(s[0]=='%'){
        s += ": i32";
        if(s[1]>='a'&&s[1]<='z'||s[1]>='A'&&s[1]<='Z'){
            s += "*";
        }
    }
    return s;
}

void tranExps(pNode node){
    if(node->child->next->next->next==NULL){
        string p = tranExp(node->child->next);
        p = changeformat(p);
        Rdem.push_back(p);
    }
    else{
        tranExps(node->child);
        string p = tranExp(node->child->next->next);
        p = changeformat(p);
        Rdem.push_back(p);
    }
}


// LVal:ID                                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LVal1",1,$1);}
//     |ID Exps                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"LVal2",2,$1,$2);}
//     ;
// Exps:Exps LB Exp RB                                 {$$ = newNode(@$.first_line,NOT_TOKEN,"Exps1",4,$1,$2,$3,$4);}
//     |LB Exp RB                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"Exps2",3,$1,$2,$3);}
//     ;
string tranLVal(pNode node){
    if(strcmp(node->name,"LVal1")==0){
        if(DEBUG) printf("LVal1\n");
        return string(node->child->value);
    }
    else if(strcmp(node->name,"LVal2")==0){
        // TODO
        if(DEBUG) printf("LVal2\n");
        Rdem.clear();
        tranExps(node->child->next);
        return string(node->child->value);
    }
}

bool checkpara(string p){
    bool res = false;
    for(int i=0;i<para_list.size();i++){
        if(para_list[i].first==p){
            res = true;
        }
    }
    return res;
}

bool checkinpara(string p){
    bool res = false;
    for(int i=0;i<para_list.size();i++){
        if(para_list[i].first==p&&para_list[i].second>=1){
            res = true;
        }
    }
    return res;
}

bool noExps(pNode node){
    bool res = false;
    if(node->child->next==NULL){
        res = true;
    }
    return res;
}


// PrimaryExp: LP Exp RP                               {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp1",3,$1,$2,$3);}
//     | LVal                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp2",1,$1);}
//     | Number                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp3",1,$1);}
//     ;
string tranPrimaryExp(pNode node){
    string p;
    if(strcmp(node->name,"PrimaryExp1")==0){
        if(DEBUG) printf("PrimaryExp1\n");
        p = tranExp(node->child->next);
    }
    else if(strcmp(node->name,"PrimaryExp2")==0){
        if(DEBUG) printf("PrimaryExp2\n");
        p = tranLVal(node->child);
        if(noExps(node->child)&&(array_list.find(p)!=array_list.end()||checkinpara(p))){
            return "%"+p;
        }
        // let %33: i32* = offset i32, #a: i32*, [%31: i32 < none], [%32: i32 < 9]
        // let %34: i32 = load %33: i32*
        if(array_list.find(p)!=array_list.end()&&checkinpara(p)){
            vector<int>temp = func_dem_array[p];
            string ss = "let %"+to_string(regnum++)+": i32* = offset i32, %"+p+": i32*";
            for(int i=0;i<temp.size();i++){
                string s;
                if(temp[i]==-1){
                    s = "none";
                }
                else{
                    s = to_string(temp[i]);
                }
                ss += ", ["+Rdem[i]+" < "+s+"]";
            }
            ircode.push_back(ss);
            ss = "let %"+to_string(regnum)+": i32 = load %"+to_string(regnum-1)+": i32*";
            ircode.push_back(ss);
            p = "%"+to_string(regnum++);
            return p;
        }
        if(array_list.find(p)!=array_list.end()){
            // let %6: i32* = offset i32, %a: i32*, [2 < 10]
            // let %7: i32 = load %6: i32*
            // if(p=="b"){
            //     cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
            //     for(int i=0;i<array_list["b"].size();i++){
            //         cout<<array_list["b"][i]<<endl;
            //     }
            //     cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
            // }
            string ss = "let %"+to_string(regnum++)+": i32* = offset i32, %"+p+": i32*"+get_dim(p);
            if(Rdem.size()<array_list[p].size()){
                for(int i=Rdem.size();i<array_list[p].size();i++){
                    ss += ",[0<"+to_string(array_list[p][i]) + "]";
                }
                ircode.push_back(ss);
                p = "%"+to_string(regnum-1);
            }
            else{
                ircode.push_back(ss);
                ss = "let %"+to_string(regnum)+": i32 = load %"+to_string(regnum-1)+": i32*";
                ircode.push_back(ss);
                p = "%"+to_string(regnum++);
            }

        }
        else if(checkinpara(p)){
            // let %13: i32* = offset i32, #a: i32*, [%12: i32 < none]
            // let %14: i32 = load %13: i32*
            string ss = "let %"+to_string(regnum++)+": i32* = offset i32, #"+p+": i32*";
            for(int i=0;i<Rdem.size();i++){
                ss += ", [" + Rdem[i] + "< none]";
            }
            ircode.push_back(ss);
            ss = "let %"+to_string(regnum)+": i32 = load %"+to_string(regnum-1)+": i32*";
            ircode.push_back(ss);
            p = "%"+to_string(regnum++);
        }
        else{
            if(checkpara(p)){
                p += ".addr";
            }
            string ss = "let %"+to_string(regnum)+": i32 = load %"+p+": i32*";
            ircode.push_back(ss);
            p = "%"+to_string(regnum++);
        }

    }
    else if(strcmp(node->name,"PrimaryExp3")==0){
        if(DEBUG) printf("PrimaryExp3\n");
        p = string(node->child->child->value);
    }
    return p;
}

// FuncRParams:Exp                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncRParams1",1,$1);}
//     |Exp COMMA FuncRParams                          {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncRParams2",3,$1,$2,$3);}
//     ;
void tranFuncRParams(pNode node){
    string p;
    if(strcmp(node->name,"FuncRParams1")==0){
        if(DEBUG) printf("FuncRParams1\n");
        p = tranExp(node->child);
        RParam.push_back(p);
        
    }
    else if(strcmp(node->name,"FuncRParams2")==0){
        // TODO
        if(DEBUG) printf("FuncRParams2\n");
        p = tranExp(node->child);
        RParam.push_back(p);
        tranFuncRParams(node->child->next->next);
    }
}


void get_paranum(pNode node){
    if(strcmp(node->name,"FuncRParams1")==0){
        if(DEBUG) printf("FuncRParams1\n");
        paranum++;
        
    }
    else if(strcmp(node->name,"FuncRParams2")==0){
        // TODO
        paranum++;
        get_paranum(node->child->next->next);
    }
}


// UnaryExp:PrimaryExp                                 {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp1",1,$1);}
//     | ID LP FuncRParams RP                          {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp2",4,$1,$2,$3,$4);}
//     | ID LP RP                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp3",3,$1,$2,$3);}
//     | UnaryOp UnaryExp                              {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp4",2,$1,$2);}
//     ;
string tranUnaryExp(pNode node){
    // TODO:建立函数表，根据返回值决定句子结构
    string p;
    if(strcmp(node->name,"UnaryExp1")==0){
        if(DEBUG) printf("UnaryExp1\n");
        p = tranPrimaryExp(node->child);
    }
    else if(strcmp(node->name,"UnaryExp2")==0){
        if(DEBUG) printf("UnaryExp2\n");
        RParam.clear();
        // RParam.shrink_to_fit();
        string func_name = node->child->value;
        paranum = 0;
        get_paranum(node->child->next->next);
        int para_number = paranum;
        tranFuncRParams(node->child->next->next);
        // let %21: () = call @putint, %20: i32
        p = "";
        for(int i=RParam.size()-para_number;i<RParam.size();i++){
            p += changeformat(RParam[i]);
            if(i<RParam.size()-1){
                p += ", ";
            }
        }
        string returntype = "()";
        if(func_list[func_name]==1){
            returntype = "i32";
        }
        string ss = "let %"+to_string(regnum++)+": "+returntype+" = call @"+func_name+", "+p;
        ircode.push_back(ss);
        p = "%"+to_string(regnum-1);
    }
    else if(strcmp(node->name,"UnaryExp3")==0){
        if(DEBUG) printf("UnaryExp3\n");
        // let %5: i32 = call @getint
        string func_name = node->child->value;
        string ss = "let %"+to_string(regnum++)+": i32 = call @"+func_name;
        ircode.push_back(ss); 
        p = "%"+to_string(regnum-1);
    }
    else if(strcmp(node->name,"UnaryExp4")==0){
        if(DEBUG) printf("UnaryExp4\n");
        p = tranUnaryExp(node->child->next);
        if(string(node->child->child->value)=="!"){
            // let %44: i32 = eq 0, %43: i32
            string ss = "let %"+to_string(regnum++)+": i32 = eq 0, "+p+": i32";
            ircode.push_back(ss);
            p = "%"+to_string(regnum-1);
            return p;
        }
        // TODO:只考虑了减法
        if(p[0]!='%'){
            if(p[0]=='-'){
                p = p.substr(1);
            }
            else{
                p = "-"+p;
            }
        }
        else{
            // let %17: i32 = sub 0, %16: i32
            string ss = "let %"+to_string(regnum++)+": i32 = sub 0, "+p+": i32";
            ircode.push_back(ss);
            p = "%"+to_string(regnum-1);
        }
    }
    return p;
}

// MulExp: UnaryExp                                    {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp1",1,$1);}
//     | MulExp STAR UnaryExp                          {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp2",3,$1,$2,$3);}
//     | MulExp DIV UnaryExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp3",3,$1,$2,$3);}
//     | MulExp MOD UnaryExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp4",3,$1,$2,$3);}
//     ;
string tranMulExp(pNode node){
    string p,q;
    if(strcmp(node->name,"MulExp1")==0){
        if(DEBUG) printf("MulExp1\n");
        p = tranUnaryExp(node->child);
    }
    else if(strcmp(node->name,"MulExp2")==0){
        if(DEBUG) printf("MulExp2\n");
        // let %15: i32 = mul %13: i32, %14: i32
        p = tranMulExp(node->child);
        q = tranUnaryExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        string ss = "let %"+to_string(regnum)+": i32 = mul "+p+", "+q;
        ircode.push_back(ss);
        p = "%" + to_string(regnum++);
    }
    else if(strcmp(node->name,"MulExp3")==0){
        if(DEBUG) printf("MulExp3\n");
        p = tranMulExp(node->child);
        q = tranUnaryExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        string ss = "let %"+to_string(regnum)+": i32 = div "+p+", "+q;
        ircode.push_back(ss);
        p = "%" + to_string(regnum++);
    }
    else if(strcmp(node->name,"MulExp4")==0){
        if(DEBUG) printf("MulExp4\n");
        p = tranMulExp(node->child);
        q = tranUnaryExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        string ss = "let %"+to_string(regnum)+": i32 = rem "+p+", "+q;
        ircode.push_back(ss);
        p = "%" + to_string(regnum++);
    }
    return p;
}

// AddExp:MulExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp1",1,$1);}
//     | AddExp PLUS MulExp                            {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp2",3,$1,$2,$3);}
//     | AddExp MINUS MulExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp3",3,$1,$2,$3);}
//     ;
string tranAddExp(pNode node){
    string p,q;
    if(strcmp(node->name,"AddExp1")==0){
        if(DEBUG) printf("AddExp1\n");
        p = tranMulExp(node->child);
    }
    else if(strcmp(node->name,"AddExp2")==0){
        if(DEBUG) printf("AddExp2\n");
        p = tranAddExp(node->child);
        q = tranMulExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        string ss = "let %"+to_string(regnum)+": i32 = add "+p+", "+q;
        ircode.push_back(ss);
        // TODO:全是常数时，是不是不需要%
        p = "%" + to_string(regnum++);
    }
    else if(strcmp(node->name,"AddExp3")==0){
        if(DEBUG) printf("AddExp3\n");
        p = tranAddExp(node->child);
        q = tranMulExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        string ss = "let %"+to_string(regnum)+": i32 = sub "+p+", "+q;
        ircode.push_back(ss);
        p = "%" + to_string(regnum++);
    }
    return p;
}

// RelExp:AddExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"RelExp1",1,$1);}
//     | RelExp RELOP AddExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"RelExp2",3,$1,$2,$3);}
//     ;
// 'lt' | 'gt' | 'le' | 'ge'
string tranRelExp(pNode node){
    string p,q;
    if(strcmp(node->name,"RelExp1")==0){
        if(DEBUG) printf("RelExp1\n");
        p = tranAddExp(node->child);
    }
    else if(strcmp(node->name,"RelExp2")==0){
        if(DEBUG) printf("RelExp2\n");
        p = tranRelExp(node->child);
        q = tranAddExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        // let %19: i32 = lt %18: i32, 100
        string op;
        string op_value = node->child->next->value;
        if(op_value=="<"){
            op = "lt";
        }
        else if(op_value==">"){
            op = "gt";
        }
        else if(op_value=="<="){
            op = "le";
        }
        else if(op_value==">="){
            op = "ge";
        }
        string ss = "let %"+to_string(regnum)+": i32 = "+op +" "+p+", "+ q;
        ircode.push_back(ss);
        p = "%" + to_string(regnum++);
    }
    return p;
}

// EqExp:RelExp                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"EqExp1",1,$1);}
//     | EqExp EQOP RelExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"EqExp2",3,$1,$2,$3);}
//     ; 
string tranEqExp(pNode node){
    string p,q;
    if(strcmp(node->name,"EqExp1")==0){
        if(DEBUG) printf("EqExp1\n");
        p = tranRelExp(node->child);
    }
    else if(strcmp(node->name,"EqExp2")==0){
        if(DEBUG) printf("EqExp2\n");
        p = tranEqExp(node->child);
        q = tranRelExp(node->child->next->next);
        p = changeformat(p);
        q = changeformat(q);
        // let %8: i32 = eq %7: i32, 5
        string eqop = node->child->next->value;
        string eq;
        if(eqop=="=="){
            eq = "eq";
        }
        else{
            eq = "ne";
        }
        string ss = "let %"+to_string(regnum)+": i32 = "+eq +" "  +p+", "+ q;
        ircode.push_back(ss);
        p = "%" + to_string(regnum++);
    }
    return p;
}

// LAndExp:EqExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"LAndExp1",1,$1);}
//     | LAndExp AND EqExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LAndExp2",3,$1,$2,$3);}
//     ;
//   let %9: i32 = gt %8: i32, 10
//   let %short_val.addr: i32* = alloca i32, 1
//   let %11: () = store %9: i32, %short_val.addr: i32*
//   br %9: i32, label %short.rhs, label %short.end
// %short.rhs:
//   let %14: i32 = call @add, %array: i32*
//   let %15: () = store %14: i32, %short_val.addr: i32*
//   jmp label %short.end
// %short.end:
//   let %16: i32 = load %short_val.addr: i32*
//   br %16: i32, label %if_then, label %if_else
string tranLAndExp(pNode node){
    string p,q;
    if(strcmp(node->name,"LAndExp1")==0){
        if(DEBUG) printf("LAndExp1\n");
        p = tranEqExp(node->child);
    }
    else if(strcmp(node->name,"LAndExp2")==0){
        if(DEBUG) printf("LAndExp2\n");
        p = tranLAndExp(node->child);
        string ss = string("let %")+"short_val_"+to_string(shortscope++)+".addr: i32* = alloca i32, 1";
        int now_scope = shortscope-1;
        ircode.push_back(ss);
        p = changeformat(p);
        ss = "let %" + to_string(regnum++)+": () = store "+p+", %"+"short_val_"+to_string(now_scope)+".addr: i32*";
        ircode.push_back(ss);
        string label1 = "%short.rhs_"+to_string(now_scope);
        string label2 = "%short.end_"+to_string(now_scope);
        ss = "br "+p+", label "+label1+", label "+label2;
        ircode.push_back(ss);
        ircode.push_back(label1+":");
        q = tranEqExp(node->child->next->next);
        q = changeformat(q);
        ss = "let %" + to_string(regnum++)+": () = store "+q+", %"+"short_val_"+to_string(now_scope)+".addr: i32*";
        ircode.push_back(ss);
        ircode.push_back("jmp label "+label2);
        ircode.push_back(label2+":");
        ss = "let %"+to_string(regnum++)+": i32 = load %short_val_"+to_string(now_scope)+".addr: i32*";
        ircode.push_back(ss);
        p = "%"+to_string(regnum-1);
    }
    return p;
}

// LOrExp: LAndExp                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"LOrExp1",1,$1);}
//     | LOrExp OR LAndExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LOrExp2",3,$1,$2,$3);}
//     ;
string tranLOrExp(pNode node){
    string p,q;
    if(strcmp(node->name,"LOrExp1")==0){
        if(DEBUG) printf("LOrExp1\n");
        p = tranLAndExp(node->child);
    }
    else if(strcmp(node->name,"LOrExp2")==0){
        if(DEBUG) printf("LOrExp2\n");
        p = tranLOrExp(node->child);
        string ss = string("let %")+"short_val_"+to_string(shortscope++)+".addr: i32* = alloca i32, 1";
        int now_scope = shortscope-1;
        ircode.push_back(ss);
        p = changeformat(p);
        ss = "let %" + to_string(regnum++)+": () = store "+p+", %"+"short_val_"+to_string(now_scope)+".addr: i32*";
        ircode.push_back(ss);
        string label1 = "%short.rhs_"+to_string(now_scope);
        string label2 = "%short.end_"+to_string(now_scope);
        ss = "br "+p+", label "+label2+", label "+label1;
        ircode.push_back(ss);
        ircode.push_back(label1+":");
        q = tranLAndExp(node->child->next->next);
        q = changeformat(q);
        ss = "let %" + to_string(regnum++)+": () = store "+q+", %"+"short_val_"+to_string(now_scope)+".addr: i32*";
        ircode.push_back(ss);
        ircode.push_back("jmp label "+label2);
        ircode.push_back(label2+":");
        ss = "let %"+to_string(regnum++)+": i32 = load %short_val_"+to_string(now_scope)+".addr: i32*";
        ircode.push_back(ss);
        p = "%"+to_string(regnum-1);
    }
    return p;
}

// Exp:LOrExp                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"Exp",1,$1);}
//     ;
string tranExp(pNode node){
    if(DEBUG) printf("Exp\n");
    string p = tranLOrExp(node->child);
    return p;
}



string get_dim(string array_name){
    string res = "";
    vector<int>temp1 = array_list[array_name];
    vector<string>temp2 = Rdem;
    if(temp1.size()<temp2.size()){
        for(int i=0;i<temp1.size();i++){
            res += ", ["+temp2[temp2.size()-i-1]+"<"+to_string(temp1[i])+"]";
        }
        return res;
    }
    for(int i=0;i<temp2.size();i++){
        try{
            res += ", ["+temp2[i]+"<"+to_string(temp1[i])+"]";
        }
        catch(...){
            res += ", ["+temp2[i]+"<none]";
        }
    }
    return res;
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
void tranStmt(pNode node){
    string p;
    if(strcmp(node->name,"Stmt1")==0){
        if(DEBUG) printf("Stmt1\n");
        string value = tranExp(node->child->next->next);
        string var_id = tranLVal(node->child);
        value = changeformat(value);
        // let %7: () = store 10, %a: i32*
        if(array_list.find(var_id)!=array_list.end()){
            // let %4: i32* = offset i32, %a: i32*, [2 < 10]
            // let %5: () = store 4, %4: i32*
            string ss = "let %"+to_string(regnum++)+": i32* = offset i32, %"+var_id+": i32*"+get_dim(var_id);
            ircode.push_back(ss);
            ss = "let %"+to_string(regnum)+": () = store "+value+", %"+to_string(regnum-1)+": i32*";
            ircode.push_back(ss);
            regnum++;
        }
        else if(checkinpara(var_id)){
            // let %13: i32* = offset i32, #a: i32*, [%12: i32 < none]
            // let %14: i32 = load %13: i32*
            string ss = "let %"+to_string(regnum++)+": i32* = offset i32, #"+var_id+": i32*";
            for(int i=0;i<Rdem.size();i++){
                ss += ", [" + Rdem[i] + "< none]";
            }
            ircode.push_back(ss);
            ss = "let %"+to_string(regnum)+": ()= store "+value+", %"+to_string(regnum-1)+": i32*";
            ircode.push_back(ss);
            p = "%"+to_string(regnum++);
        }
        else{
            if(checkpara(var_id)){
                var_id += ".addr";
            }
            string ss = "let %" + to_string(regnum++) + ": () = store "+value+",%"+var_id+": i32*";
            ircode.push_back(ss);
        }

    }
    else if(strcmp(node->name,"Stmt2")==0){
        if(DEBUG) printf("Stmt2\n");
        tranExp(node->child);
    }
    else if(strcmp(node->name,"Stmt3")==0){
        if(DEBUG) printf("Stmt3\n");
        tranBlock(node->child);
    }
    else if(strcmp(node->name,"Stmt4")==0){
        if(DEBUG) printf("Stmt4\n");
        //     |IF LP Exp RP Stmt 
        p = tranExp(node->child->next->next);
        p = changeformat(p);
        if_scope++;
        // br %8: i32, label %if_then, label %if_end
        string label1 = "%"+string("if_then_")+to_string(if_scope);
        string label2 = "%"+string("if_end_")+to_string(if_scope);
        string ss = "br "+p+", label "+label1+", label "+label2;
        ircode.push_back(ss);
        ircode.push_back(label1+":");
        tranStmt(node->child->next->next->next->next);
        if(ircode.back()[0]!='j'&&ircode.back()[1]!='m'){
            ircode.push_back("jmp label "+label2);
        }
        
        // if_scope--;
        ircode.push_back(label2+":");
        // tranStmt(node->child->next->next->next->next);
    }
    else if(strcmp(node->name,"Stmt5")==0){
        //     |IF LP Exp RP Stmt ELSE Stmt 
        if(DEBUG) printf("Stmt5\n");
        p = tranExp(node->child->next->next);
        p = changeformat(p);
        if_scope++;
        string label1 = "%"+string("if_then_")+to_string(if_scope);
        string label2 = "%"+string("if_else_")+to_string(if_scope);
        string label3 = "%"+string("if_end_")+to_string(if_scope);
        string ss = "br "+p+", label "+label1+", label "+label2;
        ircode.push_back(ss);
        ircode.push_back(label1+":");
        tranStmt(node->child->next->next->next->next);
        if(ircode.back()[0]!='j'&&ircode.back()[1]!='m') ircode.push_back("jmp label "+label3);
        ircode.push_back(label2+":");
        tranStmt(node->child->next->next->next->next->next->next);
        if(ircode.back()[0]!='j'&&ircode.back()[1]!='m') ircode.push_back("jmp label "+label3);
        ircode.push_back(label3+":");
    }
    else if(strcmp(node->name,"Stmt6")==0){
        if(DEBUG) printf("Stmt6\n");
        //     |WHILE LP Exp RP Stmt 
        // tranExp(node->child->next->next);
        // tranStmt(node->child->next->next->next->next);
        while_scope++;
        string cond = "%while_cond_" + to_string(while_scope);
        ircode.push_back("jmp label "+cond);
        ircode.push_back(cond+":");
        p = tranExp(node->child->next->next);
        p = changeformat(p);
        string body = "%while_body_" + to_string(while_scope);
        string while_end = "%while_end_" + to_string(while_scope);
        string ss = "br "+p+", label "+body+", label "+while_end;
        ircode.push_back(ss);
        ircode.push_back(body+":");
        tranStmt(node->child->next->next->next->next);
        ircode.push_back("jmp label "+cond);
        ircode.push_back(while_end+":");
    }
    else if(strcmp(node->name,"Stmt9")==0){
        // if(DEBUG) printf("Stmt9\n");
        ircode.push_back("jmp label %exit");
    }
    else if(strcmp(node->name,"Stmt10")==0){
        if(DEBUG) printf("Stmt10\n");
        p = tranExp(node->child->next);
        // TODO:返回的不是常数怎么处理？
        //  let %22: () = store 0, %ret_val.addr: i32*
        //  jmp label %exit
        p = changeformat(p);
        string ss = "let %" + to_string(regnum++)+": () = store "+p+", %ret_val.addr: i32*";
        ircode.push_back(ss);
        // TODO:返回时的label问题
        ircode.push_back("jmp label %exit");
    }
}



// Decl: VarDecl                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"Decl",1,$1);}
//     ;
// VarDecl:TYPE VarDeflist SEMI                         {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDecl",3,$1,$2,$3);}
//     ;
// VarDeflist:VarDef                                   {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDeflist1",1,$1);}
//     |VarDef COMMA VarDeflist                        {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDeflist2",3,$1,$2,$3);}
//     ;
// VarDef:ID ASSIGNOP Exp                              {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef1",3,$1,$2,$3);}
//     | ID Array                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef2",2,$1,$2);}
//     | ID                                            {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef3",1,$1);}
//     ;
// Array:LB INT RB Array                               {$$ = newNode(@$.first_line,NOT_TOKEN,"Array1",4,$1,$2,$3,$4);}
//     | LB INT RB                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"Array2",3,$1,$2,$3);}
//     ;
void get_array_info(pNode node){
    if(node==NULL){
        return ;
    }
    string dem = node->child->next->value;
    array_dem.push_back(stoi(dem));
    get_array_info(node->child->next->next->next);
}

void tranArray(pNode node){
    array_dem.clear();
    get_array_info(node);
}

int get_array_size(string s){
    vector<int> temp = array_list[s];
    int res = 1;
    for(int i=0;i<temp.size();i++){
        res *= temp[i];
    }
    return res;
}

void tranVarDef(pNode node,int globel){
    if(strcmp(node->name,"VarDef1")==0){
        if(DEBUG) printf("VarDef1\n");
        if(globel==0){
            string s = "let %"+ string(node->child->value) +": i32* = alloca i32, 1";
            ircode.push_back(s);
            string value = tranExp(node->child->next->next);
            // let %19: () = store %18: i32, %result: i32*
            value = changeformat(value);
            string ss = "let %" + to_string(regnum++) + ":() = store "+value+",%"+string(node->child->value)+": i32*";
            ircode.push_back(ss);
        }
        else{
            // @i: region i32, 1 
            string s = "@"+string(node->child->value)+": region i32, 1";
            ircode.push_back(s);
            // let %3: () = store 2024, @i: i32*
            string value = tranExp(node->child->next->next);
            value = changeformat(value);
            string ss = "let %" + to_string(regnum++) + ":() = store "+value+",@"+string(node->child->value)+": i32*";
            globel_init.push_back(ss);
        }

    }
    else if(strcmp(node->name,"VarDef2")==0){
        if(DEBUG) printf("VarDef2\n");
        tranArray(node->child->next);
        array_list[string(node->child->value)] = array_dem;
        int array_size = get_array_size(string(node->child->value));
        // let %a: i32* = alloca i32, 10
        if(globel==0){
            string ss = "let %"+string(node->child->value)+": i32* = alloca i32, "+to_string(array_size);
            ircode.push_back(ss);
        }
        else{
            // @c: region i32, 12 
            string ss = "@"+string(node->child->value)+": region i32, "+to_string(array_size);
            ircode.push_back(ss);            
        }

    }
    else if(strcmp(node->name,"VarDef3")==0){
        if(DEBUG) printf("VarDef3\n");
        if(globel==0){
            string s = "let %"+ string(node->child->value) +": i32* = alloca i32, 1";
            ircode.push_back(s);
        }
        else{
            string s = "@"+string(node->child->value)+": region i32, 1";
            ircode.push_back(s);
        }

    }
}

void tranVarDeflist(pNode node,int globel){
    if(strcmp(node->name,"VarDeflist1")==0){
        if(DEBUG) printf("VarDeflist1\n");
        tranVarDef(node->child,globel);
    }
    else if(strcmp(node->name,"VarDeflist2")==0){
        if(DEBUG) printf("VarDeflist2\n");
        tranVarDef(node->child,globel);
        tranVarDeflist(node->child->next->next,globel);
    }
}

void tranDecl(pNode node,int globel){
    // node = VarDeflist
    node = node->child->child->next;
    tranVarDeflist(node,globel);
}



void func_init(){
    ircode.push_back("fn @putint(#x: i32) -> ();\n");
    ircode.push_back("fn @putch(#x: i32) -> ();\n");
    ircode.push_back("fn @putarray(#n: i32, #arr: i32*) -> ();\n");
    ircode.push_back("fn @getint() -> i32;\n");
    ircode.push_back("fn @getch() -> i32;\n");
    ircode.push_back("fn @getarray(#n: i32, #arr: i32*) -> ();\n");
    func_list["putint"] = 0;
    func_list["getint"] = 1;

}


void tranBlockItem(pNode node){
    if(strcmp(node->name,"BlockItem1")==0){
        if(DEBUG) printf("BlockItem1\n");
        tranDecl(node->child,0);
    }
    else if(strcmp(node->name,"BlockItem2")==0){
        if(DEBUG) printf("BlockItem2\n");
        tranStmt(node->child);
    }
}

void tranBlockItems(pNode node){
    if(strcmp(node->name,"BlockItems1")==0){
        if(DEBUG) printf("BlockItems1\n");
        tranBlockItem(node->child);
    }
    else if(strcmp(node->name,"BlockItems2")==0){
        if(DEBUG) printf("BlockItems2\n");
        tranBlockItem(node->child);
        tranBlockItems(node->child->next);
    }
}

void tranBlock(pNode node){
// TODO:scope
    tranBlockItems(node->child->next);
}


// FPDimension: LB INT RB                              { $$ = newNode(@$.first_line,NOT_TOKEN,"FPDimension1",3,$1,$2,$3);}
//     | LB INT RB FPDimension                         { $$ = newNode(@$.first_line,NOT_TOKEN,"FPDimension2",4,$1,$2,$3,$4);}
//     ;
void tran_FPDimension(pNode node){
    if(node->child->next->next->next==NULL){
        func_dem_array_item.push_back(stoi(node->child->next->value));
        return ;
    }
    else{
        func_dem_array_item.push_back(stoi(node->child->next->value));
        tran_FPDimension(node->child->next->next->next);
    }
}


// FuncFParam:TYPE ID LB RB                            {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam1",4,$1,$2,$3,$4);}
//     | TYPE ID                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam2",2,$1,$2);}
//     | TYPE ID LB RB FPDimension                     {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam3",5,$1,$2,$3,$4,$5);}
//     ;
void tranFuncFParam(pNode node){
    if(strcmp(node->name,"FuncFParam1")==0){
        if(DEBUG) printf("FuncFParam1\n");
        pair<string,int> temp;
        temp.first = string(node->child->next->value);
        temp.second = 1;
        para_list.push_back(temp);
    }
    else if(strcmp(node->name,"FuncFParam2")==0){
        if(DEBUG) printf("FuncFParam2\n");
        pair<string,int> temp;
        temp.first = string(node->child->next->value);
        temp.second = 0;
        para_list.push_back(temp);
    }
    else if(strcmp(node->name,"FuncFParam3")==0){
        if(DEBUG) printf("FuncFParam3\n");
        pair<string,int> temp;
        temp.first = string(node->child->next->value);
        temp.second = 2;
        para_list.push_back(temp);
        func_dem_array_item.clear();
        func_dem_array_item.push_back(-1);
        tran_FPDimension(node->child->next->next->next->next);
        func_dem_array[string(node->child->next->value)] = func_dem_array_item;

    }
}


// FuncFParams:FuncFParam                              {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParams1",1,$1);}
//     | FuncFParam COMMA FuncFParams                  {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParams2",3,$1,$2,$3);}
//     ;
void tranFuncFParams(pNode node){
    if(strcmp(node->name,"FuncFParams1")==0){
        if(DEBUG) printf("FuncFParams1\n");
        tranFuncFParam(node->child);
    }
    else if(strcmp(node->name,"FuncFParams2")==0){
        if(DEBUG) printf("FuncFParams2\n");
        tranFuncFParam(node->child);
        tranFuncFParams(node->child->next->next);
    }
}

string tranparas(){
    int len = para_list.size();
    string res;
    for(int i=0;i<len;i++){
        string item = "#" + para_list[i].first + ": i32";
        if(para_list[i].second>=1){
            item += "*";
        }
        res += item;
        if(i<len-1){
            res += ", ";
        }
    }
    return res;
}


// FuncDef:TYPE ID LP FuncFParams RP Block          {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef1",6,$1,$2,$3,$4,$5,$6);}
//     |TYPE ID LP RP Block                         {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef2",5,$1,$2,$3,$4,$5);}
//     |VOID ID LP FuncFParams RP Block             {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef3",6,$1,$2,$3,$4,$5,$6);}
//     |VOID ID LP RP Block                         {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef4",5,$1,$2,$3,$4,$5);}
//     ;
// fn @main() -> i32
void tranFuncDef(pNode node){
    string func_id = node->child->next->value;
    string return_type = node->child->value;
    pNode blocknode;
    para_list.clear();
    string func_def_code;
    if(strcmp(node->name,"FuncDef1")==0||strcmp(node->name,"FuncDef2")==0){
        func_list[string(node->child->next->value)] = 1;
    }
    else{
        func_list[string(node->child->next->value)] = 0;
    }

    if(strcmp(node->name,"FuncDef1")==0||strcmp(node->name,"FuncDef3")==0){
        //TODO:参数列表
        blocknode = node->child->next->next->next->next->next;
        tranFuncFParams(node->child->next->next->next);
        string paras = tranparas();
        func_def_code = "fn @" + func_id + "("+paras+") -> ";
    }
    else{
        blocknode = node->child->next->next->next->next;
        func_def_code = "fn @" + func_id + "() -> ";
    }
    
    if(return_type=="int"){
        func_def_code += "i32";
    }
    else{
        func_def_code += "()";
    }
    func_def_code += " {";
    ircode.push_back(func_def_code);
    string ss = string("%") + string("entry:");
    ircode.push_back(ss);
    // func para init
    for (int i=0;i<para_list.size();i++){
        if(para_list[i].second==0){
            //  let %a.addr: i32* = alloca i32, 1
            //  let %3: () = store #a: i32, %a.addr: i32*
            ss = "let %"+para_list[i].first+".addr: i32* = alloca i32, 1";
            ircode.push_back(ss);
            ss = "let %"+to_string(regnum++)+": () = store #"+para_list[i].first+": i32, %"+ para_list[i].first +".addr: i32*";
            ircode.push_back(ss);
        }
        else{
            // TODO:指针
        }
    }

    
    if(return_type=="int") ircode.push_back("let %ret_val.addr: i32* = alloca i32, 1");
    if(func_id=="main"){
        for(int i=0;i<globel_init.size();i++){
            ircode.push_back(globel_init[i]);
        }
    }
    tranBlock(blocknode);
    // %exit:
    // let %25: i32 = load %ret_val.addr: i32*
    // ret %25: i32
    ss = string("%") + string("exit:");
    ircode.push_back(ss);    
    // TODO:返回值不同
    if(strcmp(node->name,"FuncDef1")==0||strcmp(node->name,"FuncDef2")==0){
        ss = "let %" + to_string(regnum++)+": i32 = load %ret_val.addr: i32*";
        ircode.push_back(ss); 
        ss = "ret %" + to_string(regnum-1)+": i32";
        ircode.push_back(ss);
    }
    else{
        ircode.push_back("ret ()");
    }

    ircode.push_back("}");
}

void tranCompUnit(pNode node){
    
    if(strcmp(node->name,"CompUnit1")==0){
        if(DEBUG) printf("CompUnit1\n");
        tranDecl(node->child,1);
        tranCompUnit(node->child->next);
    }
    else if(strcmp(node->name,"CompUnit2")==0){
        if(DEBUG) printf("CompUnit2\n");
        tranFuncDef(node->child);
        tranCompUnit(node->child->next);
    }
    else if(strcmp(node->name,"CompUnit3")==0){
        if(DEBUG) printf("CompUnit3\n");
        tranDecl(node->child,1);
    }
    else if(strcmp(node->name,"CompUnit4")==0){
        if(DEBUG) printf("CompUnit4\n");
        tranFuncDef(node->child);
    }
}

void translate(pNode root){
    func_init();
    tranCompUnit(root->child);
    for(int i=0;i<ircode.size();i++){
        cout<<ircode[i]<<endl;
    }
}