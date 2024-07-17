%{
#include <stdio.h>
#include "ast.h"
void yyerror(char *s);
int yylex(void);
extern int synErr;
// extern int l;
pNode root;
%}

%union{
    pNode node; 
}

%token <node>  INT
%token <node>  VOID
%token <node>  ID
%token <node>  TYPE
%token <node>  COMMA
%token <node>  DOT
%token <node>  SEMI
%token <node>  RELOP
%token <node>  EQOP
%token <node>  ASSIGNOP
%token <node>  PLUS MINUS STAR DIV MOD
%token <node>  AND OR NOT
%token <node>  LP RP LB RB LC RC
%token <node>  CR
%token <node>  blank
%token <node>  IF
%token <node>  ELSE
%token <node>  WHILE
%token <node>  STRUCT
%token <node>  RETURN
%token <node>  BREAK
%token <node>  CONTINUE

%type <node> Program CompUnit Decl VarDecl VarDeflist VarDef Array
%type <node> FuncDef FuncFParams FuncFParam FPDimension
%type <node> Block BlockItems BlockItem Stmt Exps
%type <node> Exp LVal PrimaryExp Number UnaryExp UnaryOp FuncRParams 
%type <node> MulExp AddExp RelExp EqExp LAndExp LOrExp

%left LB RB
%left LP RP
%left RELOP
%left STAR DIV DOT
%right ASSIGNOP
%right NOT
%left AND OR

%%
Program: CompUnit                                   {$$ = newNode(@$.first_line, NOT_TOKEN, "Program",1,$1); root = $$; }
    ;
CompUnit: Decl CompUnit                             {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit1",2,$1,$2);}
    | FuncDef CompUnit                              {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit2",2,$1,$2);}
    | Decl                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit3",1,$1);}
    | FuncDef                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"CompUnit4",1,$1);}
    ;
Decl: VarDecl                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"Decl",1,$1);}
    ;
VarDecl:TYPE VarDeflist SEMI                         {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDecl",3,$1,$2,$3);}
    ;
VarDeflist:VarDef                                   {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDeflist1",1,$1);}
    |VarDef COMMA VarDeflist                        {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDeflist2",3,$1,$2,$3);}
    ;
VarDef:ID ASSIGNOP Exp                              {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef1",3,$1,$2,$3);}
    | ID Array                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef2",2,$1,$2);}
    | ID                                            {$$ = newNode(@$.first_line,NOT_TOKEN,"VarDef3",1,$1);}
    ;
Array:LB INT RB Array                               {$$ = newNode(@$.first_line,NOT_TOKEN,"Array1",4,$1,$2,$3,$4);}
    | LB INT RB                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"Array2",3,$1,$2,$3);}
    ;
FuncDef:TYPE ID LP FuncFParams RP Block          {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef1",6,$1,$2,$3,$4,$5,$6);}
    |TYPE ID LP RP Block                         {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef2",5,$1,$2,$3,$4,$5);}
    |VOID ID LP FuncFParams RP Block             {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef3",6,$1,$2,$3,$4,$5,$6);}
    |VOID ID LP RP Block                         {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncDef4",5,$1,$2,$3,$4,$5);}
    ;

FuncFParams:FuncFParam                              {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParams1",1,$1);}
    | FuncFParam COMMA FuncFParams                  {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParams2",3,$1,$2,$3);}
    ;
FuncFParam:TYPE ID LB RB                            {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam1",4,$1,$2,$3,$4);}
    | TYPE ID                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam2",2,$1,$2);}
    | TYPE ID LB RB FPDimension                     {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncFParam3",5,$1,$2,$3,$4,$5);}
    ;
FPDimension: LB INT RB                              { $$ = newNode(@$.first_line,NOT_TOKEN,"FPDimension1",3,$1,$2,$3);}
    | LB INT RB FPDimension                         { $$ = newNode(@$.first_line,NOT_TOKEN,"FPDimension2",4,$1,$2,$3,$4);}
    ;

Block:LC BlockItems RC                              {$$ = newNode(@$.first_line,NOT_TOKEN,"Block",3,$1,$2,$3);}
    ;
BlockItems: BlockItem                               {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItems1",1,$1);}         
    | BlockItem BlockItems                          {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItems2",2,$1,$2);}       
    ;
BlockItem:Decl                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItem1",1,$1);}
    |Stmt                                           {$$ = newNode(@$.first_line,NOT_TOKEN,"BlockItem2",1,$1);}
    ;
Stmt:LVal ASSIGNOP Exp SEMI                         {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt1",4,$1,$2,$3,$4);}
    |Exp SEMI                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt2",2,$1,$2);}
    |Block                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt3",1,$1);}
    |IF LP Exp RP Stmt                              {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt4",5,$1,$2,$3,$4,$5);}
    |IF LP Exp RP Stmt ELSE Stmt                    {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt5",7,$1,$2,$3,$4,$5,$6,$7);}
    |WHILE LP Exp RP Stmt                           {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt6",5,$1,$2,$3,$4,$5);}
    |BREAK SEMI                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt7",2,$1,$2);}
    |CONTINUE SEMI                                  {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt8",2,$1,$2);}
    |RETURN SEMI                                    {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt9",2,$1,$2);}
    |RETURN Exp SEMI                                {$$ = newNode(@$.first_line,NOT_TOKEN,"Stmt10",3,$1,$2,$3);}
    ;

Exp:LOrExp                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"Exp",1,$1);}
    ;
LVal:ID                                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LVal1",1,$1);}
    |ID Exps                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"LVal2",2,$1,$2);}
    ;
Exps:Exps LB Exp RB                                 {$$ = newNode(@$.first_line,NOT_TOKEN,"Exps1",4,$1,$2,$3,$4);}
    |LB Exp RB                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"Exps2",3,$1,$2,$3);}
    ;
PrimaryExp: LP Exp RP                               {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp1",3,$1,$2,$3);}
    | LVal                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp2",1,$1);}
    | Number                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"PrimaryExp3",1,$1);}
    ;
Number:INT                                          {$$ = newNode(@$.first_line,NOT_TOKEN,"Number",1,$1);}
    ;
UnaryExp:PrimaryExp                                 {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp1",1,$1);}
    | ID LP FuncRParams RP                          {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp2",4,$1,$2,$3,$4);}
    | ID LP RP                                      {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp3",3,$1,$2,$3);}
    | UnaryOp UnaryExp                              {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryExp4",2,$1,$2);}
    ;
UnaryOp: PLUS                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryOp1",1,$1);}
    | MINUS                                         {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryOp2",1,$1);}
    | NOT                                           {$$ = newNode(@$.first_line,NOT_TOKEN,"UnaryOp3",1,$1);}
    ;
FuncRParams:Exp                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncRParams1",1,$1);}
    |Exp COMMA FuncRParams                          {$$ = newNode(@$.first_line,NOT_TOKEN,"FuncRParams2",3,$1,$2,$3);}
    ;

MulExp: UnaryExp                                    {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp1",1,$1);}
    | MulExp STAR UnaryExp                          {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp2",3,$1,$2,$3);}
    | MulExp DIV UnaryExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp3",3,$1,$2,$3);}
    | MulExp MOD UnaryExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"MulExp4",3,$1,$2,$3);}
    ;
AddExp:MulExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp1",1,$1);}
    | AddExp PLUS MulExp                            {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp2",3,$1,$2,$3);}
    | AddExp MINUS MulExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"AddExp3",3,$1,$2,$3);}
    ;
RelExp:AddExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"RelExp1",1,$1);}
    | RelExp RELOP AddExp                           {$$ = newNode(@$.first_line,NOT_TOKEN,"RelExp2",3,$1,$2,$3);}
    ;
EqExp:RelExp                                        {$$ = newNode(@$.first_line,NOT_TOKEN,"EqExp1",1,$1);}
    | EqExp EQOP RelExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"EqExp2",3,$1,$2,$3);}
    ;   
LAndExp:EqExp                                       {$$ = newNode(@$.first_line,NOT_TOKEN,"LAndExp1",1,$1);}
    | LAndExp AND EqExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LAndExp2",3,$1,$2,$3);}
    ;
LOrExp: LAndExp                                     {$$ = newNode(@$.first_line,NOT_TOKEN,"LOrExp1",1,$1);}
    | LOrExp OR LAndExp                             {$$ = newNode(@$.first_line,NOT_TOKEN,"LOrExp2",3,$1,$2,$3);}
    ;
%%


void yyerror(char *str){
    printf("ERROR!!");
}

// int yywrap(){
//     return 1;
// }
