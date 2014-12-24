%{
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
 
#include "symbol_table.h"
#include "ast.h"
#include "type_info.h"
 
extern int line_num;
extern int yylex();
 
symbolTable symbol_table;
int error_count = 0;
 
// Create an error function to call when the current line has an error
void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): "
       << err_string << std::endl;
  error_count++;
}
 
// Create an alternate error function when a *different* line than being read in has an error.
void yyerror2(std::string err_string, int orig_line) {
  std::cout << "ERROR(line " << orig_line << "): "
       << err_string << std::endl;
  error_count++;
}
 
%}
 
%union {
  char * lexeme;
  ASTNode * ast_node;
}
 
%token CASSIGN_ADD CASSIGN_SUB CASSIGN_MULT CASSIGN_DIV CASSIGN_MOD INCREMENT DECREMENT COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE BOOL_AND BOOL_OR CONSOLE LOG BOOLEAN TO_STRING COMMAND_IF COMMAND_ELSE COMMAND_WHILE COMMAND_BREAK
%token <lexeme> NUMBER_LIT STRING_LIT ID VAR
 
%right '=' CASSIGN_ADD CASSIGN_SUB CASSIGN_MULT CASSIGN_DIV CASSIGN_MOD
%left BOOL_OR
%left BOOL_AND
%nonassoc COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS '!'
%left '.'
 
%nonassoc NOELSE
%nonassoc COMMAND_ELSE
 
 
%type <ast_node> var_declare expression declare_assign statement statement_list var_usage lhs_ok command argument_list code_block if_start while_start flow_command
%%
 
program:      statement_list {
                 // Traverse AST
                 $1->Interpret(symbol_table);
              }
             ;
 
statement_list:         {
                   $$ = new ASTNode_Block;
                 }
        |        statement_list statement {
                   if ($2 != NULL) $1->AddChild($2);
                   $$ = $1;
                 }
        ;
 
statement:   var_declare ';'    {  $$ = $1;  }
        |    declare_assign ';' {  $$ = $1;  }
        |    expression ';'     {  $$ = $1;  }
        |    command ';'        {  $$ = $1;  }
        |    flow_command       {  $$ = $1;  }
        |    code_block         {  $$ = $1;  }
        |    ';'                {  $$ = NULL;  }
        ;
 
var_declare:        VAR ID {
                  if (symbol_table.InCurScope($2) != 0) {
                    std::string err_string = "redeclaration of variable '";
                    err_string += $2;
                    err_string += "'";
                    yyerror(err_string);
                    exit(1);
                  }
 
                  tableEntry * cur_entry = symbol_table.AddEntry(0, $2);
                  $$ = new ASTNode_Variable(cur_entry);
                  $$->SetLineNum(line_num);
                }
        ;
 
declare_assign:  var_declare '=' expression {
                   $$ = new ASTNode_Assign($1, $3);
                   $$->SetLineNum(line_num);
                 }
        ;
 
var_usage:   ID {
               tableEntry * cur_entry = symbol_table.Lookup($1);
               if (cur_entry == NULL) {
                 std::string err_string = "unknown variable '";
                 err_string += $1;
                 err_string += "'";
                 yyerror(err_string);
                 exit(1);
               }
               $$ = new ASTNode_Variable(cur_entry);
               $$->SetLineNum(line_num);
             }
        ;
 
lhs_ok:  var_usage { $$ = $1; }
      ;
 
expression:  expression '+' expression {
               $$ = new ASTNode_Math2($1, $3, '+');
               $$->SetLineNum(line_num);
             }
        |    expression '-' expression {
               $$ = new ASTNode_Math2($1, $3, '-');
               $$->SetLineNum(line_num);
             }
        |    expression '*' expression {
               $$ = new ASTNode_Math2($1, $3, '*');
               $$->SetLineNum(line_num);
             }
        |    expression '/' expression {
               $$ = new ASTNode_Math2($1, $3, '/');
               $$->SetLineNum(line_num);
             }
        |    expression '%' expression {
               $$ = new ASTNode_Math2($1, $3, '%');
               $$->SetLineNum(line_num);
             }
        |    expression COMP_EQU expression {
               $$ = new ASTNode_Comparison($1, $3, COMP_EQU);
               $$->SetLineNum(line_num);
             }
        |    expression COMP_NEQU expression {
               $$ = new ASTNode_Comparison($1, $3, COMP_NEQU);
               $$->SetLineNum(line_num);
             }
        |    expression COMP_LESS expression {
               $$ = new ASTNode_Comparison($1, $3, COMP_LESS);
               $$->SetLineNum(line_num);
             }
        |    expression COMP_LTE expression {
               $$ = new ASTNode_Comparison($1, $3, COMP_LTE);
               $$->SetLineNum(line_num);
             }
        |    expression COMP_GTR expression {
               $$ = new ASTNode_Comparison($1, $3, COMP_GTR);
               $$->SetLineNum(line_num);
             }
        |    expression COMP_GTE expression {
               $$ = new ASTNode_Comparison($1, $3, COMP_GTE);
               $$->SetLineNum(line_num);
             }
        |    expression BOOL_AND expression {
               $$ = new ASTNode_Bool2($1, $3, BOOL_AND);
               $$->SetLineNum(line_num);
             }
        |    expression BOOL_OR expression {
               $$ = new ASTNode_Bool2($1, $3, BOOL_OR);
               $$->SetLineNum(line_num);
             }
        |    lhs_ok '=' expression {
               $$ = new ASTNode_Assign($1, $3);
               $$->SetLineNum(line_num);
             }
        |    lhs_ok CASSIGN_ADD expression {
               $$ = new ASTNode_Assign($1, new ASTNode_Math2($1, $3, '+') );
               $$->SetLineNum(line_num);
             }
        |    lhs_ok CASSIGN_SUB expression {
               $$ = new ASTNode_Assign($1, new ASTNode_Math2($1, $3, '-') );
               $$->SetLineNum(line_num);
             }
        |    lhs_ok CASSIGN_MULT expression {
               $$ = new ASTNode_Assign($1, new ASTNode_Math2($1, $3, '*') );
               $$->SetLineNum(line_num);
             }
        |    lhs_ok CASSIGN_DIV expression {
               $$ = new ASTNode_Assign($1, new ASTNode_Math2($1, $3, '/') );
               $$->SetLineNum(line_num);
             }
        |    lhs_ok CASSIGN_MOD expression {
               $$ = new ASTNode_Assign($1, new ASTNode_Math2($1, $3, '%') );
               $$->SetLineNum(line_num);
             }
        |    INCREMENT var_usage {
               ASTNode * one_const = new ASTNode_Literal(Type::NUMBER, "1");
               ASTNode * addition = new ASTNode_Math2($2, one_const, '+');
               $$ = new ASTNode_Assign($2, addition);
               $$->SetLineNum(line_num);
             }
        |    DECREMENT var_usage {
               ASTNode * one_const = new ASTNode_Literal(Type::NUMBER, "1");
               ASTNode * subtraction = new ASTNode_Math2($2, one_const, '-');
               $$ = new ASTNode_Assign($2, subtraction);
               $$->SetLineNum(line_num);
             }
        |    '-' expression %prec UMINUS {
               $$ = new ASTNode_Math1($2, '-');
               $$->SetLineNum(line_num);
             }
        |    '!' expression %prec UMINUS {
               $$ = new ASTNode_Bool1($2, '!');
               $$->SetLineNum(line_num);
             }
        |    '(' expression ')' { $$ = $2; }
        |    NUMBER_LIT {
               $$ = new ASTNode_Literal(Type::NUMBER, $1);
               $$->SetLineNum(line_num);
             }
        |    STRING_LIT {
               std::string lit = $1;

               // Strip off outside quotes
               lit = lit.substr(1, lit.length() - 2);
               $$ = new ASTNode_Literal(Type::STRING, lit);
               $$->SetLineNum(line_num);
             }
        |    var_usage { $$ = $1; }
        |    BOOLEAN '(' expression ')' {
               $$ = new ASTNode_BoolCast($3);
               $$->SetLineNum(line_num);
            }
        |    var_usage '.' TO_STRING '(' ')' {
               $$ = new ASTNode_StringCast($1);
               $$->SetLineNum(line_num);
            }
        ;
 
argument_list:        argument_list ',' expression {
                  ASTNode * node = $1; // Grab the node used for arg list.
                  node->AddChild($3);    // Save this argument in the node.
                  $$ = node;
                }
        |        expression {
                  // Create a temporary AST node to hold the arg list.
                  $$ = new ASTNode_TempNode(Type::VOID);
                  $$->AddChild($1);   // Save this argument in the temp node.
                  $$->SetLineNum(line_num);
                }
        ;
 
command:   CONSOLE '.' LOG '(' argument_list ')' {
             $$ = new ASTNode_Print(NULL);
             $$->TransferChildren($5);
             $$->SetLineNum(line_num);
             delete $5;
           }
        |  COMMAND_BREAK {
             $$ = new ASTNode_Break();
             $$->SetLineNum(line_num);
           }
        ;
 
if_start:  COMMAND_IF '(' expression ')' {
             $$ = new ASTNode_If(new ASTNode_BoolCast($3), NULL, NULL);
             $$->SetLineNum(line_num);
           }
        ;
 
while_start:  COMMAND_WHILE '(' expression ')' {
                $$ = new ASTNode_While($3, NULL);
                $$->SetLineNum(line_num);
              }
           ;
 
flow_command:  if_start statement COMMAND_ELSE statement {
                 $$->SetChild(1, $2);
                 $$->SetChild(2, $4);
                 $$ = $1;
               }
            |  if_start statement %prec NOELSE {
                 $$ = $1;
                 $$->SetChild(1, $2);
               }
            |  while_start statement {
                 $$ = $1;
                 $$->SetChild(1, $2);
               }
            ;
 
block_start: '{' { symbol_table.IncScope(); } ;
block_end:   '}' { symbol_table.DecScope(); } ;
code_block:  block_start statement_list block_end { $$ = $2; } ;
 
%%
void LexMain(int argc, char * argv[]);
 
int main(int argc, char * argv[])
{
  error_count = 0;
  LexMain(argc, argv);
 
  yyparse();
 
  return 0;
}
