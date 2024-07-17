%option noinput
%option nounput
%option noyywrap


%{
#include "ast.h"
#include "sysy.tab.hh"
#include<stdio.h>
void yyerror(char *);
#define YY_USER_ACTION yylloc.first_line=yylineno; 
%}

%option yylineno

blank [ \t\n]
digit [0-9]
COMMENT_SINGLE "//"(.)* 
COMMENT_MULTI "/*"([^*]|\*+[^*/])*"*"+"/" 
letter [_a-zA-Z]
ld[0-9_a-zA-Z]
nonletter [^_a-zA-Z]
RELOP 	>|<|>=|<=
EQOP ==|!=
TYPE int
BREAK break
CONTINUE continue
IF if
ELSE else
WHILE while
STRUCT struct
RETURN return
VOID void
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
MOD "%"
AND "&&"
OR "||"
NOT "!"
DOT "."
SEMI ";"
COMMA ","
ASSIGNOP "="
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
ID {letter}{ld}*
%%
{IF}            { yylval.node = newTokenNode(yylineno,TOKEN,"IF", yytext);return IF;}
{ELSE}          { yylval.node = newTokenNode(yylineno,TOKEN,"ELSE", yytext);return ELSE;}
{WHILE}         { yylval.node = newTokenNode(yylineno,TOKEN,"WHILE", yytext);return WHILE;}
{RETURN}        { yylval.node = newTokenNode(yylineno,TOKEN,"RETURN", yytext);return RETURN;}
{BREAK}         { yylval.node = newTokenNode(yylineno,TOKEN,"BREAK", yytext);return BREAK;}
{CONTINUE}      { yylval.node = newTokenNode(yylineno,TOKEN,"CONTINUE", yytext);return CONTINUE;}
{STRUCT}        { yylval.node = newTokenNode(yylineno,TOKEN,"RETURN", yytext);return RETURN;}
{VOID}          { yylval.node = newTokenNode(yylineno,TOKEN,"VOID", yytext);return VOID;}
{TYPE}          { yylval.node = newTokenNode(yylineno,TOKEN,"TYPE", yytext);return TYPE;}
{COMMENT_SINGLE}    {}
{COMMENT_MULTI}     {}
{digit}+        { yylval.node = newTokenNode(yylineno,TOKEN,"digit", yytext);return INT;}
{RELOP}         { yylval.node = newTokenNode(yylineno,TOKEN,"RELOP", yytext);return RELOP;}
{EQOP}          { yylval.node = newTokenNode(yylineno,TOKEN,"EQOP", yytext);return EQOP;}
{ID}            { yylval.node = newTokenNode(yylineno,TOKEN,"ID", yytext);return ID;}
{PLUS}          { yylval.node = newTokenNode(yylineno,TOKEN,"ADD", yytext);return PLUS;}
{MINUS}         { yylval.node = newTokenNode(yylineno,TOKEN,"SUB", yytext);return MINUS;}
{STAR}          { yylval.node = newTokenNode(yylineno,TOKEN,"MUL", yytext);return STAR;}
{DIV}           { yylval.node = newTokenNode(yylineno,TOKEN,"DIV", yytext);return DIV;}
{AND}           { yylval.node = newTokenNode(yylineno,TOKEN,"AND", yytext);return AND;}
{MOD}           { yylval.node = newTokenNode(yylineno,TOKEN,"MOD", yytext);return MOD;}
{OR}            { yylval.node = newTokenNode(yylineno,TOKEN,"OR", yytext);return OR;}
{NOT}           { yylval.node = newTokenNode(yylineno,TOKEN,"NOT", yytext);return NOT;}
{ASSIGNOP}      { yylval.node = newTokenNode(yylineno,TOKEN,"ASSIGN", yytext);return ASSIGNOP;}

{LP}            { yylval.node = newTokenNode(yylineno,TOKEN,"LP", yytext);return LP;}
{RP}            { yylval.node = newTokenNode(yylineno,TOKEN,"RP", yytext);return RP;}
{LB}            { yylval.node = newTokenNode(yylineno,TOKEN,"LB", yytext);return LB;}
{RB}            { yylval.node = newTokenNode(yylineno,TOKEN,"RB", yytext);return RB;}
{LC}            { yylval.node = newTokenNode(yylineno,TOKEN,"LC", yytext);return LC;}
{RC}            { yylval.node = newTokenNode(yylineno,TOKEN,"RC", yytext);return RC;}

{SEMI}          { yylval.node = newTokenNode(yylineno,TOKEN,"SEMI", yytext);return SEMI;}
{COMMA}         { yylval.node = newTokenNode(yylineno,TOKEN,"COMMA", yytext);return COMMA;}
{DOT}           { yylval.node = newTokenNode(yylineno,TOKEN,"DOT", yytext);return DOT;}

{blank}         { }
.               {printf("ERROR(%s)", yytext); }

%%

// int main(int argc, char **argv)
// {
//   yylex();
//   yywrap();
// }
// int yywrap()
// {
// 	return 1;
// }

// int main(int argc, char** argv) {
//     if (argc > 1) {
//         if (!(yyin = fopen(argv[1], "r"))) {
//             perror(argv[1]);
//             return 1;
//         }
//     }
//     yylex();
//     // yylval.node = newTokenNode(yylineno,TOKEN_,"%d\n",lines);
//     return 0;
// }