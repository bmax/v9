#include "ast.h"
#include "v9-parser.tab.hh"

extern void yyerror(std::string err_string);
extern void yyerror2(std::string err_string, int orig_line);


/////////////////////
//  ASTNode

void ASTNode::TransferChildren(ASTNode * from_node)
{
  // Move all of the children out of the from_node
  for (int i = 0; i < from_node->GetNumChildren(); i++) {
    AddChild(from_node->GetChild(i));
  }

  // Clear the children in from_node so they are not recursively deleted when it is.
  from_node->children.resize(0);
}


/////////////////////
//  ASTNode_Block

tableEntry * ASTNode_Block::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////////
//  ASTNode_Variable

tableEntry * ASTNode_Variable::Interpret(symbolTable & table)
{
  return var_entry;   // Return the symbol-table entry associated with this variable.
}


////////////////////////
//  ASTNode_Literal

ASTNode_Literal::ASTNode_Literal(int in_type, std::string in_lex)
  : ASTNode(in_type), lexeme(in_lex)
{
}  

tableEntry * ASTNode_Literal::Interpret(symbolTable & table)
{
  return NULL;
}


//////////////////////
// ASTNode_Assign

ASTNode_Assign::ASTNode_Assign(ASTNode * lhs, ASTNode * rhs)
  : ASTNode(lhs->GetType())
{ 
  if (lhs->GetType() != rhs->GetType()) {
    std::string err_message = "types do not match for assignment (lhs='";
    err_message += Type::AsString(lhs->GetType());
    err_message += "', rhs='";
    err_message += Type::AsString(rhs->GetType());
    err_message += "')";
    yyerror(err_message);
    exit(1);
  }
  children.push_back(lhs);
  children.push_back(rhs);
}

tableEntry * ASTNode_Assign::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_Math1

ASTNode_Math1::ASTNode_Math1(ASTNode * in_child, int op)
  : ASTNode(Type::INT), math_op(op)
{
  if (in_child->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_child->GetType());
    err_message += "' in mathematical expressions";
    yyerror(err_message);
    exit(1);
  }
  children.push_back(in_child);
}

tableEntry * ASTNode_Math1::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_Math2

ASTNode_Math2::ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::INT), math_op(op)
{
  bool rel_op = (op==COMP_EQU) || (op==COMP_NEQU) || (op==COMP_LESS) || (op==COMP_LTE) ||
    (op==COMP_GTR) || (op==COMP_GTE);

  ASTNode * in_test = (in1->GetType() != Type::INT) ? in1 : in2;
  if (in_test->GetType() != Type::INT) {
    if (!rel_op || in_test->GetType() != Type::CHAR) {
      std::string err_message = "cannot use type '";
      err_message += Type::AsString(in_test->GetType());
      err_message += "' in mathematical expressions";
      yyerror(err_message);
      exit(1);
    } else if (rel_op && (in1->GetType() != Type::CHAR || in2->GetType() != Type::CHAR)) {
      std::string err_message = "types do not match for relationship operator (lhs='";
      err_message += Type::AsString(in1->GetType());
      err_message += "', rhs='";
      err_message += Type::AsString(in2->GetType());
      err_message += "')";
      yyerror(err_message);
      exit(1);
    }
  }

  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Math2::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_Bool2

ASTNode_Bool2::ASTNode_Bool2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::INT), bool_op(op)
{
  ASTNode * in_test = (in1->GetType() != Type::INT) ? in1 : in2;
  if (in_test->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_test->GetType());
    err_message += "' in mathematical expressions";
    yyerror(err_message);
    exit(1);
  }

  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Bool2::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_If

ASTNode_If::ASTNode_If(ASTNode * in1, ASTNode * in2, ASTNode * in3)
  : ASTNode(Type::VOID)
{
  if (in1->GetType() != Type::INT) {
    yyerror("condition for if statements must evaluate to type int");
    exit(1);
  }

  children.push_back(in1);
  children.push_back(in2);
  children.push_back(in3);
}


tableEntry * ASTNode_If::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_While

ASTNode_While::ASTNode_While(ASTNode * in1, ASTNode * in2)
  : ASTNode(Type::VOID)
{
  children.push_back(in1);
  children.push_back(in2);

  if (in1->GetType() != Type::INT) {
    yyerror("condition for while statements must evaluate to type int");
    exit(1);
  }
}


tableEntry * ASTNode_While::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_Break

ASTNode_Break::ASTNode_Break()
  : ASTNode(Type::VOID)
{
}


tableEntry * ASTNode_Break::Interpret(symbolTable & table)
{
  return NULL;
}


/////////////////////
// ASTNode_Print

ASTNode_Print::ASTNode_Print(ASTNode * out_child)
  : ASTNode(Type::VOID)
{
  // Save the child...
  if (out_child != NULL) AddChild(out_child);
}


tableEntry * ASTNode_Print::Interpret(symbolTable & table)
{
  return NULL;
}
