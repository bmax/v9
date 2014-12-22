%{
#include "symbol_table.h"
#include "type_info.h"
#include "ast.h"
#include "v9-parser.tab.hh"

#include <iostream>
#include <stdio.h>
#include <string>

// Two global variables (not clean, but works...)
int line_num = 1;
%}

%option nounput

id	        [_a-zA-Z][a-zA-Z0-9_]*
int_lit         [0-9]+
comment		#.*
whitespace	[ \t\r]
passthrough	[+\-*/%=(),!{}[\].;]

%%

"if"      { return COMMAND_IF; }
"else"    { return COMMAND_ELSE; }
"while"   { return COMMAND_WHILE; }
"break"   { return COMMAND_BREAK; }
"console" { return CONSOLE; }
"log"     { return LOG; }
"Boolean" { return BOOLEAN; }

"var"         { yylval.lexeme = strdup(yytext);  return VAR; }
{id}          { yylval.lexeme = strdup(yytext);  return ID; }
{int_lit}     { yylval.lexeme = strdup(yytext);  return INT_LIT; }

{passthrough}  { yylval.lexeme = strdup(yytext);  return (int) yytext[0]; }

"+=" { return CASSIGN_ADD; }
"-=" { return CASSIGN_SUB; }
"*=" { return CASSIGN_MULT; }
"/=" { return CASSIGN_DIV; }
"%=" { return CASSIGN_MOD; }

"==" { return COMP_EQU; }
"!=" { return COMP_NEQU; }
"<" { return COMP_LESS; }
"<=" { return COMP_LTE; }
">" { return COMP_GTR; }
">=" { return COMP_GTE; }

"&&" { return BOOL_AND; }
"||" { return BOOL_OR; }

{comment} { ; }
{whitespace} { ; }
\n  { line_num++; }

.      { std::cout << "ERROR(line " << line_num << "): Unknown Token '" << yytext << "'." << std::endl; exit(1); }

%%

void LexMain(int argc, char * argv[])
{
  FILE * file = NULL;
  bool input_found = false;

  // Loop through all of the command-line arguments.
  for (int arg_id = 1; arg_id < argc; arg_id++) {
    std::string cur_arg(argv[arg_id]);

    if (cur_arg == "-h") {
      std::cout << "Tubular Compiler v. 0.4 (Project 4)"  << std::endl
           << "Format: " << argv[0] << "[flags] [filename]" << std::endl
           << std::endl
           << "Available Flags:" << std::endl
           << "  -h  :  Help (this information)" << std::endl
        ;
      exit(0);
    }

    // PROCESS OTHER ARGUMENTS HERE IF YOU ADD THEM

    // If the next argument begins with a dash, assume it's an unknown flag...
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

  if (input_found == false) {
    std::cerr << "Format: " << argv[0] << " [flags] [input filename]" << std::endl;
    std::cerr << "Type '" << argv[0] << " -h' for help." << std::endl;
    exit(1);
  }
 
  return;
}

