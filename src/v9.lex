%{
#include "symbol_table.h"
#include "type_info.h"
#include "ast.h"
#include "v9-parser.tab.hh"

#include <iostream>
#include <stdio.h>
#include <string>

int line_num = 1;
%}

%option nounput

id          [_a-zA-Z][a-zA-Z0-9_]*
number_lit  [0-9]+\.[0-9]*|\.[0-9]+|[0-9]+
string_lit  '(\\.|[^\\']+)*'|\"(\\.|[^\\"]+)*\"
comment     \/\/.*
whitespace  [ \t\r]
passthrough [+\-*/%=(),!{}[\].;:~&\|^]

%%

"if"       { return COMMAND_IF; }
"else"     { return COMMAND_ELSE; }
"while"    { return COMMAND_WHILE; }
"for"      { return COMMAND_FOR; }
"in"       { return COMMAND_IN; }
"break"    { return COMMAND_BREAK; }
"delete"   { return COMMAND_DELETE; }
"true"     { return TRUE; }
"false"    { return FALSE; }
"null"     { return NLL; }
"console"  { return CONSOLE; }
"log"      { return LOG; }
"Number"   { return NUMBER; }
"Boolean"  { return BOOLEAN; }
"String"   { return STRING; }
"toString" { return TO_STRING; }
"typeof"   { return TYPEOF; }
"void"     { return VOID; }

"var"         { yylval.lexeme = strdup(yytext);  return VAR; }
{id}          { yylval.lexeme = strdup(yytext);  return ID; }
{number_lit}  { yylval.lexeme = strdup(yytext);  return NUMBER_LIT; }
{string_lit}  { yylval.lexeme = strdup(yytext);  return STRING_LIT; }
{passthrough} { yylval.lexeme = strdup(yytext);  return (int) yytext[0]; }

"+=" { return CASSIGN_ADD; }
"-=" { return CASSIGN_SUB; }
"*=" { return CASSIGN_MULT; }
"/=" { return CASSIGN_DIV; }
"%=" { return CASSIGN_MOD; }
"++" { return INCREMENT; }
"--" { return DECREMENT; }

"&=" { return CASSIGN_BITWISE_AND; }
"|=" { return CASSIGN_BITWISE_OR; }
"^=" { return CASSIGN_BITWISE_XOR; }

"<<" { return LSHIFT; }
">>" { return RSHIFT; }
"<<=" { return CASSIGN_LSHIFT; }
">>=" { return CASSIGN_RSHIFT; }
">>>" { return ZF_RSHIFT; }
">>>=" { return CASSIGN_ZF_RSHIFT; }

"=="  { return COMP_EQU; }
"!="  { return COMP_NEQU; }
"<"   { return COMP_LESS; }
"<="  { return COMP_LTE; }
">"   { return COMP_GTR; }
">="  { return COMP_GTE; }
"===" { return COMP_SEQU; }
"!==" { return COMP_SNEQU; }

"&&" { return BOOL_AND; }
"||" { return BOOL_OR; }

{comment} { ; }
{whitespace} { ; }
\n { line_num++; }

. { std::cout << "ERROR(line " << line_num << "): Unknown Token '" << yytext << "'." << std::endl; exit(1); }

%%

void LexMain(int argc, char * argv[])
{
  FILE * file = NULL;
  bool input_found = false;

  for (int arg_id = 1; arg_id < argc; arg_id++) {
    std::string cur_arg(argv[arg_id]);

    if (cur_arg == "-h") {
      std::cout << "V9 JavaScript Engine"  << std::endl;
      std::cout << "Format: " << argv[0] << "[flags] [filename]" << std::endl;
      std::cout << "Available Flags:" << std::endl;
      std::cout << "  -h  :  Help (this information)" << std::endl;
      exit(0);
    }

    if (cur_arg[0] == '-') {
      std::cerr << "ERROR: Unknown command-line flag: " << cur_arg << std::endl;
      exit(1);
    }

    if (!input_found) {
      file = fopen(argv[arg_id], "r");
      if (!file) {
        std::cerr << "Error opening " << cur_arg << std::endl;
        exit(1);
      }
      yyin = file;
      input_found = true;
      continue;
    }

  }

  if (!input_found) {
    std::cerr << "Format: " << argv[0] << " [flags] [input filename]" << std::endl;
    std::cerr << "Type '" << argv[0] << " -h' for help." << std::endl;
    exit(1);
  }

  return;
}
