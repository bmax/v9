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
  if(GetType() == Type::NUMBER) {
    out_var->SetNumberValue(atof(lexeme.c_str()));
  }
  else if(GetType() == Type::STRING) {
    out_var->SetStringValue(lexeme);
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

  if(left->GetType() == Type::NUMBER) {
    left->SetNumberValue(right->GetNumberValue());
  }
  else if(left->GetType() == Type::BOOL) {
    left->SetBoolValue(right->GetBoolValue());
  }
  else if(left->GetType() == Type::STRING) {
    left->SetStringValue(right->GetStringValue());
  }

  return left;
}


/////////////////////
// ASTNode_Math1

ASTNode_Math1::ASTNode_Math1(ASTNode * in_child, int op)
  : ASTNode(Type::NUMBER), math_op(op)
{
  children.push_back(in_child);
}

tableEntry * ASTNode_Math1::Interpret(symbolTable & table)
{
  tableEntry * in = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::NUMBER);

  switch (math_op) {
    case '-':
      out_var->SetNumberValue(-in->GetNumberValue());
      break;
  }

  if (in->GetTemp() == true) table.RemoveEntry( in );

  return out_var;
}


/////////////////////
// ASTNode_Math2

ASTNode_Math2::ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::NUMBER), math_op(op)
{
  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Math2::Interpret(symbolTable & table)
{
  tableEntry * in1 = GetChild(0)->Interpret(table);
  tableEntry * in2 = GetChild(1)->Interpret(table);
  tableEntry * out_var;

  if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::NUMBER) {
    out_var = table.AddTempEntry(Type::NUMBER);
    float in1_val = in1->GetNumberValue();
    float in2_val = in2->GetNumberValue();

    if (math_op == '+') { out_var->SetNumberValue(in1_val + in2_val); }
    else if (math_op == '-') { out_var->SetNumberValue(in1_val - in2_val); }
    else if (math_op == '*') { out_var->SetNumberValue(in1_val * in2_val); }
    else if (math_op == '/') { out_var->SetNumberValue(in1_val / in2_val); }
    else if (math_op == '%') { out_var->SetNumberValue(fmod(in1_val, in2_val)); }
  }
  else if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::STRING) {
    out_var = table.AddTempEntry(Type::STRING);

    std::stringstream ss;
    ss << in1->GetNumberValue();
    std::string in1_val = ss.str();
    std::string in2_val = in2->GetStringValue();

    if (math_op == '+') { out_var->SetStringValue(in1_val + in2_val); }
  }
  else if(in1->GetType() == Type::STRING && in2->GetType() == Type::NUMBER) {
    out_var = table.AddTempEntry(Type::STRING);

    std::stringstream ss;
    ss << in2->GetNumberValue();
    std::string in1_val = in1->GetStringValue();
    std::string in2_val = ss.str();

    if (math_op == '+') { out_var->SetStringValue(in1_val + in2_val); }
  }
  else if(in1->GetType() == Type::STRING && in2->GetType() == Type::STRING) {
    out_var = table.AddTempEntry(Type::STRING);
    std::string in1_val = in1->GetStringValue();
    std::string in2_val = in2->GetStringValue();

    if (math_op == '+') { out_var->SetStringValue(in1_val + in2_val); }
  }

  return out_var;
}

ASTNode_Comparison::ASTNode_Comparison(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::BOOL), comp_op(op)
{
  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Comparison::Interpret(symbolTable & table)
{
  tableEntry * in1 = GetChild(0)->Interpret(table);
  tableEntry * in2 = GetChild(1)->Interpret(table);
  tableEntry * out_var;

  if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::NUMBER) {
    out_var = table.AddTempEntry(Type::BOOL);
    float in1_val = in1->GetNumberValue();
    float in2_val = in2->GetNumberValue();

    bool value;

    if (comp_op == COMP_EQU) { value = (in1_val == in2_val); }
    else if (comp_op == COMP_NEQU) { value = (in1_val != in2_val); }
    else if (comp_op == COMP_GTR) { value = (in1_val > in2_val); }
    else if (comp_op == COMP_GTE) { value = (in1_val >= in2_val); }
    else if (comp_op == COMP_LESS) { value = (in1_val < in2_val); }
    else if (comp_op == COMP_LTE) { value = (in1_val <= in2_val); }

    out_var->SetBoolValue(value);
  }

  return out_var;
}


ASTNode_BoolCast::ASTNode_BoolCast(ASTNode * in)
  : ASTNode(Type::NUMBER)
{
  children.push_back(in);
}


tableEntry * ASTNode_BoolCast::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  if(in_var->GetType() == Type::BOOL) {
    return in_var;
  }

  tableEntry * out_var = table.AddTempEntry(Type::BOOL);

  if(in_var->GetType() == Type::NUMBER) {
    out_var->SetBoolValue(in_var->GetNumberValue() != 0);
  }

  return out_var;
}

ASTNode_Bool1::ASTNode_Bool1(ASTNode * in, int op)
  : ASTNode(Type::BOOL), bool_op(op)
{
  children.push_back(in);
}


tableEntry * ASTNode_Bool1::Interpret(symbolTable & table)
{
  ASTNode_BoolCast * cast = new ASTNode_BoolCast(GetChild(0));
  tableEntry * in_var = cast->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::BOOL);

  switch(bool_op) {
    case '!':
      out_var->SetBoolValue(!in_var->GetBoolValue());
      break;
  }

  return out_var;
}

/////////////////////
// ASTNode_Bool2

ASTNode_Bool2::ASTNode_Bool2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::NUMBER), bool_op(op)
{
  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Bool2::Interpret(symbolTable & table)
{
  ASTNode_BoolCast * cast1 = new ASTNode_BoolCast(GetChild(0));
  tableEntry * in1 = cast1->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::BOOL);

  out_var->SetBoolValue(in1->GetBoolValue());

  // Determine the correct operation for short-circuiting
  if (bool_op == BOOL_AND) {
    if(!out_var->GetBoolValue()) {
      return out_var;
    }
  }
  else if (bool_op == BOOL_OR) {
    if(out_var->GetBoolValue()) {
      return out_var;
    }
  }

  // Only reach here if we don't short circuit
  ASTNode_BoolCast * cast2 = new ASTNode_BoolCast(GetChild(1));
  tableEntry * in2 = cast2->Interpret(table);

  if (bool_op == BOOL_AND) {
    out_var->SetBoolValue(in1->GetBoolValue() && in2->GetBoolValue());
  }
  else if (bool_op == BOOL_OR) {
    out_var->SetBoolValue(in1->GetBoolValue() || in2->GetBoolValue());
  }

  // Cleanup symbol table.
  if (in1->GetTemp() == true) table.RemoveEntry( in1 );
  if (in2->GetTemp() == true) table.RemoveEntry( in2 );

  return out_var;
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
  tableEntry * in0 = GetChild(0)->Interpret(table);
  if(in0->GetBoolValue()) {
    if (GetChild(1)) {
      tableEntry * in1 = GetChild(1)->Interpret(table);
      if (in1 && in1->GetTemp() == true) table.RemoveEntry( in1 );
    }
  }
  else {
    if (GetChild(2)) {
      tableEntry * in2 = GetChild(2)->Interpret(table);
      if (in2 && in2->GetTemp() == true) table.RemoveEntry( in2 );
    }
  }
  return NULL;
}


/////////////////////
// ASTNode_While

ASTNode_While::ASTNode_While(ASTNode * in1, ASTNode * in2)
  : ASTNode(Type::VOID)
{
  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_While::Interpret(symbolTable & table)
{
  ASTNode_BoolCast * cast = new ASTNode_BoolCast(GetChild(0));

  while(cast->Interpret(table)->GetBoolValue()) {
    if (GetChild(1)) {
      tableEntry * in1 = GetChild(1)->Interpret(table);
      if (in1 && in1->GetTemp() == true) table.RemoveEntry( in1 );
    }
  }

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
      case Type::NUMBER:
        std::cout << cur_var->GetNumberValue();
        break;
      case Type::BOOL:
        if(cur_var->GetBoolValue()) {
          std::cout << "true";
        }
        else {
          std::cout << "false";
        }
        break;
      case Type::STRING:
        std::cout << cur_var->GetStringValue();
        break;
    }
  }

  std::cout << std::endl;

  return NULL;
}
