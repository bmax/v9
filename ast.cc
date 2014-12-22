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
  for (int i = 0; i < GetNumChildren(); i++) {
    tableEntry * current = GetChild(i)->Interpret(table);
    if (current != NULL && current->GetTemp() == true) {
      table.RemoveEntry( current );
    }
  }

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
  tableEntry * out_var = table.AddTempEntry(GetType());
  if(GetType() == Type::NUM) {
    out_var->SetFloatValue(atof(lexeme.c_str()));
  }
  return out_var;
}


//////////////////////
// ASTNode_Assign

ASTNode_Assign::ASTNode_Assign(ASTNode * lhs, ASTNode * rhs)
  : ASTNode(lhs->GetType())
{
  children.push_back(lhs);
  children.push_back(rhs);
}

tableEntry * ASTNode_Assign::Interpret(symbolTable & table)
{
  tableEntry * left = GetChild(0)->Interpret(table);
  tableEntry * right = GetChild(1)->Interpret(table);

  left->SetType(right->GetType());

  if(left->GetType() == Type::NUM) {
    left->SetFloatValue(right->GetFloatValue());
  }
  else if(left->GetType() == Type::BOOL) {
    left->SetBoolValue(right->GetBoolValue());
  }
  return NULL;
}


/////////////////////
// ASTNode_Math1

ASTNode_Math1::ASTNode_Math1(ASTNode * in_child, int op)
  : ASTNode(Type::NUM), math_op(op)
{
  children.push_back(in_child);
}

tableEntry * ASTNode_Math1::Interpret(symbolTable & table)
{
  tableEntry * in = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::NUM);

  switch (math_op) {
    case '-':
      out_var->SetFloatValue(-in->GetFloatValue());
      break;
    case '!':
      out_var->SetFloatValue(!in->GetFloatValue());
      break;
  }

  if (in->GetTemp() == true) table.RemoveEntry( in );

  return out_var;
}


/////////////////////
// ASTNode_Math2

ASTNode_Math2::ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::NUM), math_op(op)
{
  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Math2::Interpret(symbolTable & table)
{
  tableEntry * in1 = GetChild(0)->Interpret(table);
  tableEntry * in2 = GetChild(1)->Interpret(table);
  tableEntry * out_var;

  if(in1->GetType() == Type::NUM && in2->GetType() == Type::NUM) {
    out_var = table.AddTempEntry(Type::NUM);
    float in1_val = in1->GetFloatValue();
    float in2_val = in2->GetFloatValue();

    if (math_op == '+') { out_var->SetFloatValue(in1_val + in2_val); }
    else if (math_op == '+') { out_var->SetFloatValue(in1_val + in2_val); }
    else if (math_op == '-') { out_var->SetFloatValue(in1_val - in2_val); }
    else if (math_op == '*') { out_var->SetFloatValue(in1_val * in2_val); }
    else if (math_op == '/') { out_var->SetFloatValue(in1_val / in2_val); }
    else if (math_op == '%') { out_var->SetFloatValue(fmod(in1_val, in2_val)); }
  }

  return out_var;
}


ASTNode_BoolCast::ASTNode_BoolCast(ASTNode * in)
  : ASTNode(Type::NUM)
{
  children.push_back(in);
}


tableEntry * ASTNode_BoolCast::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::BOOL);

  if(in_var->GetType() == Type::NUM) {
    out_var->SetBoolValue(in_var->GetFloatValue() != 0);
  }

  return out_var;
}

/////////////////////
// ASTNode_Bool2

ASTNode_Bool2::ASTNode_Bool2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::NUM), bool_op(op)
{
  ASTNode * in_test = (in1->GetType() != Type::NUM) ? in1 : in2;
  if (in_test->GetType() != Type::NUM) {
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

  if (in1->GetType() != Type::NUM) {
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
  for (int i = 0; i < GetNumChildren(); i++) {
    tableEntry * cur_var = GetChild(i)->Interpret(table);
    switch (cur_var->GetType()) {
      case Type::NUM:
        std::cout << cur_var->GetFloatValue();
        break;
      case Type::BOOL:
        if(cur_var->GetBoolValue()) {
          std::cout << "true";
        }
        else {
          std::cout << "false";
        }
    }
  }

  std::cout << std::endl;

  return NULL;
}
