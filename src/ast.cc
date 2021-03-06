#include "ast.h"
#include "v9-parser.tab.hh"

extern void yyerror(std::string err_string);
extern void yyerror2(std::string err_string, int orig_line);

// ASTNode

void ASTNode::TransferChildren(ASTNode * from_node)
{
  // Move all of the children out of the from_node
  for (int i = 0; i < from_node->GetNumChildren(); i++) {
    AddChild(from_node->GetChild(i));
  }

  // Clear the children in from_node so they are not recursively deleted when it is.
  from_node->children.resize(0);
}

//  ASTNode_Block

tableEntry * ASTNode_Block::Interpret(symbolTable & table)
{
  for (int i = 0; i < GetNumChildren(); i++) {
    tableEntry * current = GetChild(i)->Interpret(table);
  }

  return NULL;
}

// ASTNode_Variable

tableEntry * ASTNode_Variable::Interpret(symbolTable & table)
{
  if(var_entry->GetType() == Type::REFERENCE) {
    ASTNode * var = new ASTNode_Variable(var_entry->GetReference());
    return var->Interpret(table);
  }
  else {
    return var_entry;
  }
}

// ASTNode_Literal

ASTNode_Literal::ASTNode_Literal(int in_type)
  : ASTNode(in_type)
{
}

ASTNode_Literal::ASTNode_Literal(int in_type, std::string in_lex)
  : ASTNode(in_type), lexeme(in_lex)
{
}

tableEntry * ASTNode_Literal::Interpret(symbolTable & table)
{
  tableEntry * out_var = table.AddTempEntry(GetType());
  if(GetType() == Type::NUMBER) {
    if(lexeme.length() > 1) {
      if(lexeme[0] == '0' && lexeme[1] == 'x') {
        out_var->SetNumberValue(strtol(lexeme.c_str(), NULL, 16));
      }
      else if(lexeme[0] == '0') {
        out_var->SetNumberValue(strtol(lexeme.c_str(), NULL, 8));
      }
      else {
        out_var->SetNumberValue(atof(lexeme.c_str()));
      }
    }
    else {
      out_var->SetNumberValue(atof(lexeme.c_str()));
    }
  }
  else if(GetType() == Type::BOOL) {
    if(lexeme == "true") {
      out_var->SetBoolValue(true);
    }
    else {
      out_var->SetBoolValue(false);
    }
  }
  else if(GetType() == Type::STRING) {
    out_var->SetStringValue(lexeme);
  }
  else if(GetType() == Type::OBJECT) {
    out_var->InitializeObject();
    if(GetNumChildren() > 0) {
      ASTNode * obj = new ASTNode_Variable(out_var);
      for(int i = 0; i < GetNumChildren(); i += 2) {
        ASTNode * property = new ASTNode_Property(obj, GetChild(i), true);
        ASTNode * assign = new ASTNode_Assign(property, GetChild(i + 1));
        assign->Interpret(table);
      }
    }
  }
  else if(GetType() == Type::ARRAY) {
    out_var->InitializeArray();
  }

  return out_var;
}

// ASTNode_Property

ASTNode_Property::ASTNode_Property(ASTNode * obj, ASTNode * index,
    bool assignment) : ASTNode(Type::VOID)
{
  children.push_back(obj);
  children.push_back(index);
  this->assignment = assignment;
}

tableEntry * ASTNode_Property::Interpret(symbolTable & table)
{
  tableEntry * obj = GetChild(0)->Interpret(table);

  ASTNode * cast = new ASTNode_StringCast(GetChild(1));
  std::string sindex = cast->Interpret(table)->GetStringValue();

  if(obj->GetType() == Type::OBJECT) {
    if(assignment) {
      tableEntry * prop = table.AddTempEntry(Type::VOID);
      obj->SetProperty(sindex, prop);
      return prop;
    }
    else {
      tableEntry * prop = obj->GetProperty(sindex);
      if(prop) {
        SetType(prop->GetType());
        return prop;
      }
      else {
        std::string error = "object ";
        error += obj->GetName();
        error += " does not have property";
        error += sindex;
        yyerror(error);
      }
    }
  }
  else if(obj->GetType() == Type::ARRAY) {
    unsigned int idx = atoi(sindex.c_str());
    if(assignment) {
      tableEntry * val = table.AddTempEntry(Type::VOID);
      obj->SetIndex(idx, val);
      return val;
    }
    else {
      tableEntry * val = obj->GetIndex(idx);
      if(val) {
        SetType(val->GetType());
        return val;
      }
      else {
        std::string error = "array ";
        error += obj->GetName();
        error += " does not have index";
        error += sindex;
        yyerror(error);
      }
    }
  }

}

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

  // Right expression is undefined, don't perform any assignment
  if(!right) {
    return NULL;
  }

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
  else if(left->GetType() == Type::OBJECT) {
    left->SetReference(right);
    left->SetType(Type::REFERENCE);
  }
  else if(left->GetType() == Type::ARRAY) {
    left->SetReference(right);
    left->SetType(Type::REFERENCE);
  }

  return left;
}

// ASTNode_Math1

ASTNode_Math1::ASTNode_Math1(ASTNode * in_child, int op, bool pre)
  : ASTNode(Type::NUMBER), math_op(op), prefix(pre)
{
  children.push_back(in_child);
}

tableEntry * ASTNode_Math1::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::NUMBER);

  switch (math_op) {
    case '-':
      out_var->SetNumberValue(-in_var->GetNumberValue());
      break;
    case INCREMENT:
      if(prefix) {
        in_var->SetNumberValue(in_var->GetNumberValue() + 1);
        out_var->SetNumberValue(in_var->GetNumberValue());
      }
      else {
        out_var->SetNumberValue(in_var->GetNumberValue());
        in_var->SetNumberValue(in_var->GetNumberValue() + 1);
      }
      break;
    case DECREMENT:
      if(prefix) {
        in_var->SetNumberValue(in_var->GetNumberValue() - 1);
        out_var->SetNumberValue(in_var->GetNumberValue());
      }
      else {
        out_var->SetNumberValue(in_var->GetNumberValue());
        in_var->SetNumberValue(in_var->GetNumberValue() - 1);
      }
      break;
  }

  return out_var;
}

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

// ASTNode_Comparison

ASTNode_Comparison::ASTNode_Comparison(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::BOOL), comp_op(op)
{
  children.push_back(in1);
  children.push_back(in2);
}

bool strict_equality(tableEntry * a, tableEntry * b) {
  if(a->GetType() != b->GetType()) {
    return false;
  }

  else if(a->GetType() == Type::NUMBER) {
    if(isnan(a->GetNumberValue()) || isnan(b->GetNumberValue())) {
      return false;
    }

    return a->GetNumberValue() == b->GetNumberValue();
  }

  else if(a->GetType() == Type::STRING) {
    return a->GetStringValue() == b->GetStringValue();
  }

  else if(a->GetType() == Type::BOOL) {
    return a->GetBoolValue() == b->GetBoolValue();
  }

  else {
    return a == b;
  }
}

bool abstract_equality(tableEntry * a, tableEntry * b, symbolTable & table) {
  if(a && b && a->GetType() == b->GetType()) {
    return strict_equality(a, b);
  }

  else if(a->GetType() == NLL && !b) {
    return true;
  }

  else if(a->GetType() == NLL && !a) {
    return true;
  }

  else if(a->GetType() == Type::NUMBER && b->GetType() == Type::STRING) {
    ASTNode * cast = new ASTNode_NumberCast(new ASTNode_Variable(b));
    return a->GetNumberValue() == cast->Interpret(table)->GetNumberValue();
  }

  else if(a->GetType() == Type::STRING && b->GetType() == Type::NUMBER) {
    ASTNode * cast = new ASTNode_NumberCast(new ASTNode_Variable(b));
    return cast->Interpret(table)->GetNumberValue() == b->GetNumberValue();
  }

  else if(a->GetType() == Type::NUMBER && b->GetType() == Type::BOOL) {
    ASTNode * cast = new ASTNode_NumberCast(new ASTNode_Variable(b));
    return a->GetNumberValue() == cast->Interpret(table)->GetNumberValue();
  }

  else if(a->GetType() == Type::BOOL && b->GetType() == Type::NUMBER) {
    ASTNode * cast = new ASTNode_NumberCast(new ASTNode_Variable(b));
    return cast->Interpret(table)->GetNumberValue() == b->GetNumberValue();
  }

  return false;
}

tableEntry * ASTNode_Comparison::Interpret(symbolTable & table)
{
  tableEntry * in1 = GetChild(0)->Interpret(table);
  tableEntry * in2 = GetChild(1)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::BOOL);

  bool value;
  switch(comp_op) {
    case COMP_EQU:
      value = abstract_equality(in1, in2, table);
      break;
    case COMP_NEQU:
      value = !abstract_equality(in1, in2, table);
      break;
    case COMP_SEQU:
      value = strict_equality(in1, in2);
      break;
    case COMP_SNEQU:
      value = !strict_equality(in1, in2);
      break;
    case COMP_GTR:
      if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::NUMBER) {
        value = (in1->GetNumberValue() > in2->GetNumberValue());
      }
      break;
    case COMP_GTE:
      if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::NUMBER) {
        value = (in1->GetNumberValue() >= in2->GetNumberValue());
      }
      break;
    case COMP_LESS:
      if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::NUMBER) {
        value = (in1->GetNumberValue() < in2->GetNumberValue());
      }
      break;
    case COMP_LTE:
      if(in1->GetType() == Type::NUMBER && in2->GetType() == Type::NUMBER) {
        value = (in1->GetNumberValue() <= in2->GetNumberValue());
      }
      break;
  }

  out_var->SetBoolValue(value);

  return out_var;
}

// ASTNode_Bool1

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

  return out_var;
}

// ASTNode_Bitwise1

ASTNode_Bitwise1::ASTNode_Bitwise1(ASTNode * in, int op)
  : ASTNode(Type::NUMBER), bitwise_op(op)
{
  children.push_back(in);
}

tableEntry * ASTNode_Bitwise1::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);

  int value = in_var->GetNumberValue();

  tableEntry * out_var = table.AddTempEntry(Type::NUMBER);

  switch(bitwise_op) {
    case '~':
      value = ~value;
      break;
  }

  out_var->SetNumberValue(value);
  return out_var;
}

// ASTNode_Bitwise2

ASTNode_Bitwise2::ASTNode_Bitwise2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::NUMBER), bitwise_op(op)
{
  children.push_back(in1);
  children.push_back(in2);
}

tableEntry * ASTNode_Bitwise2::Interpret(symbolTable & table)
{
  tableEntry * in0 = GetChild(0)->Interpret(table);
  tableEntry * in1 = GetChild(1)->Interpret(table);

  int left = in0->GetNumberValue();
  int right = in1->GetNumberValue();

  tableEntry * out_var = table.AddTempEntry(Type::NUMBER);

  int value;
  switch(bitwise_op) {
    case '&':
      value = left & right;
      break;
    case '|':
      value = left | right;
      break;
    case '^':
      value = left ^ right;
      break;
    case LSHIFT:
      value = left << right;
      break;
    case RSHIFT:
      value = left >> right;
      break;
    case ZF_RSHIFT:
      value = left >> unsigned(right);
      break;
  }

  out_var->SetNumberValue(value);
  return out_var;
}

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
    }
  }
  else {
    if (GetChild(2)) {
      tableEntry * in2 = GetChild(2)->Interpret(table);
    }
  }
  return NULL;
}

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
    }
  }

  return NULL;
}

// ASTNode_For

ASTNode_For::ASTNode_For(ASTNode * in1, ASTNode * in2, ASTNode * in3,
    ASTNode * in4) : ASTNode(Type::VOID)
{
  children.push_back(in1);
  children.push_back(in2);
  children.push_back(in3);
  children.push_back(in4);
}

tableEntry * ASTNode_For::Interpret(symbolTable & table)
{
  ASTNode_BoolCast * cast = new ASTNode_BoolCast(GetChild(1));

  if(GetChild(0)) {
    tableEntry * in0 = GetChild(0)->Interpret(table);
  }
  while(cast->Interpret(table)->GetBoolValue()) {
    if (GetChild(3)) {
      tableEntry * in3 = GetChild(3)->Interpret(table);
    }
    if (GetChild(2)) {
      tableEntry * in2 = GetChild(2)->Interpret(table);
    }
  }

  return NULL;
}

// ASTNode_ForIn

ASTNode_ForIn::ASTNode_ForIn(ASTNode * in1, ASTNode * in2, ASTNode * in3)
  : ASTNode(Type::VOID)
{
  children.push_back(in1);
  children.push_back(in2);
  children.push_back(in3);
}

tableEntry * ASTNode_ForIn::Interpret(symbolTable & table)
{
  // Setup a variable to be assigned at each iteration
  tableEntry * iterator = GetChild(0)->Interpret(table);
  ASTNode * iterator_usage = new ASTNode_Variable(iterator);

  // The item to be iterated over
  tableEntry * iterable = GetChild(1)->Interpret(table);

  if(iterable->GetType() == Type::OBJECT) {
    // Iterate over each property of the object
    std::map<std::string, tableEntry*> * pm = iterable->GetPropertyMap();
    for (std::map<std::string, tableEntry*>::iterator i = pm->begin();
         i != pm->end(); i++) {
      // Assign the iterator
      ASTNode * prop_str = new ASTNode_Literal(Type::STRING, i->first);
      ASTNode * assignment = new ASTNode_Assign(iterator_usage, prop_str);
      assignment->Interpret(table);

      // Run body of loop
      if(GetChild(2)) {
        GetChild(2)->Interpret(table);
      }
    }
  }

  return NULL;
}

// ASTNode_Break

ASTNode_Break::ASTNode_Break()
  : ASTNode(Type::VOID)
{
}

tableEntry * ASTNode_Break::Interpret(symbolTable & table)
{
  return NULL;
}

// ASTNode_Print

ASTNode_Print::ASTNode_Print(ASTNode * out_child)
  : ASTNode(Type::VOID)
{
  if (out_child) {
    AddChild(out_child);
  }
}

tableEntry * ASTNode_Print::Interpret(symbolTable & table)
{
  for (int i = 0; i < GetNumChildren(); i++) {
    ASTNode * cast = new ASTNode_StringCast(GetChild(i));
    tableEntry * cur_var = cast->Interpret(table);

    std::cout << cur_var->GetStringValue();
  }

  std::cout << std::endl;

  return NULL;
}

// ASTNode_Delete

ASTNode_Delete::ASTNode_Delete(ASTNode * var)
  : ASTNode(Type::VOID)
{
  AddChild(var);
}

tableEntry * ASTNode_Delete::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  table.RemoveEntry(in_var);

  return NULL;
}

// ASTNode_NumberCast

ASTNode_NumberCast::ASTNode_NumberCast(ASTNode * in)
  : ASTNode(Type::NUMBER)
{
  children.push_back(in);
}

tableEntry * ASTNode_NumberCast::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::NUMBER);

  if(!in_var) {
    out_var->SetNumberValue(0.0 / 0.0);
    return out_var;
  }

  if(in_var->GetType() == Type::NUMBER) {
    return in_var;
  }

  if(in_var->GetType() == Type::STRING) {
    out_var->SetNumberValue(atof(in_var->GetStringValue().c_str()));
  }
  else if(in_var->GetType() == Type::BOOL) {
    if(in_var->GetBoolValue()) {
      out_var->SetNumberValue(1);
    }
    else {
      out_var->SetNumberValue(0);
    }
  }

  return out_var;
}

// ASTNode_BoolCast

ASTNode_BoolCast::ASTNode_BoolCast(ASTNode * in)
  : ASTNode(Type::NUMBER)
{
  children.push_back(in);
}

tableEntry * ASTNode_BoolCast::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::BOOL);

  if(!in_var) {
    out_var->SetBoolValue(false);
    return out_var;
  }

  if(in_var->GetType() == Type::BOOL) {
    return in_var;
  }

  if(in_var->GetType() == Type::NUMBER) {
    out_var->SetBoolValue(in_var->GetNumberValue() != 0);
  }

  return out_var;
}

// ASTNode_StringCast

ASTNode_StringCast::ASTNode_StringCast(ASTNode * in)
  : ASTNode(Type::STRING)
{
  children.push_back(in);
}

tableEntry * ASTNode_StringCast::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * out_var = table.AddTempEntry(Type::STRING);

  if(!in_var) {
    out_var->SetStringValue("undefined");
    return out_var;
  }

  if(in_var->GetType() == Type::STRING) {
    return in_var;
  }

  std::stringstream ss;

  if(in_var->GetType() == Type::NUMBER) {
    ss << in_var->GetNumberValue();
  }
  else if(in_var->GetType() == Type::BOOL) {
    if(in_var->GetBoolValue()) {
      ss << "true";
    }
    else {
      ss << "false";
    }
  }
  else if(in_var->GetType() == Type::NLL) {
    ss << "null";
  }

  out_var->SetStringValue(ss.str());

  return out_var;
}

// ASTNode_TypeOf

ASTNode_TypeOf::ASTNode_TypeOf(ASTNode * in)
  : ASTNode(Type::STRING)
{
  children.push_back(in);
}

tableEntry * ASTNode_TypeOf::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);

  std::string type;
  if(in_var) {
    type = Type::AsString(in_var->GetType());
  }
  else {
    type = "undefined";
  }

  ASTNode * out_var = new ASTNode_Literal(Type::STRING, type);

  return out_var->Interpret(table);
}

// ASTNode_Void

ASTNode_Void::ASTNode_Void(ASTNode * in)
  : ASTNode(Type::VOID)
{
  children.push_back(in);
}

tableEntry * ASTNode_Void::Interpret(symbolTable & table)
{
  GetChild(0)->Interpret(table);

  return NULL;
}

// ASTNode_Join

ASTNode_Join::ASTNode_Join(ASTNode * in, ASTNode * sep)
  : ASTNode(Type::STRING)
{
  children.push_back(in);
  children.push_back(sep);
}

tableEntry * ASTNode_Join::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * seperator = GetChild(1)->Interpret(table);

  std::string join_str = "";

  std::map<unsigned int, tableEntry*> * pm = in_var->GetArray();
  for (std::map<unsigned int, tableEntry*>::iterator i = pm->begin();
       i != pm->end(); i++) {
    ASTNode * var = new ASTNode_Variable(i->second);
    ASTNode * cast = new ASTNode_StringCast(var);
    tableEntry * string_val = cast->Interpret(table);
    join_str += string_val->GetStringValue();
    std::map<unsigned int, tableEntry*>::iterator end = pm->end();
    if(i != --end) {
      join_str += seperator->GetStringValue();
    }
  }

  ASTNode * out_var = new ASTNode_Literal(Type::STRING, join_str);
  return out_var->Interpret(table);
}

// ASTNode_Push

ASTNode_Push::ASTNode_Push(ASTNode * in, ASTNode * elem)
  : ASTNode(Type::VOID)
{
  children.push_back(in);
  children.push_back(elem);
}

tableEntry * ASTNode_Push::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);
  tableEntry * element = GetChild(1)->Interpret(table);

  std::map<unsigned int, tableEntry*>::iterator end = in_var->GetArray()->end();
  unsigned int last_index = end->first;

  in_var->SetIndex(last_index++, element);

  return NULL;
}

// ASTNode_Pop

ASTNode_Pop::ASTNode_Pop(ASTNode * in)
  : ASTNode(Type::VOID)
{
  children.push_back(in);
}

tableEntry * ASTNode_Pop::Interpret(symbolTable & table)
{
  tableEntry * in_var = GetChild(0)->Interpret(table);

  if(in_var->GetArray()->empty()) {
    return NULL;
  }

  std::map<unsigned int, tableEntry*>::iterator end = in_var->GetArray()->end();
  end--;
  tableEntry * last_elem = end->second;

  in_var->GetArray()->erase(end);

  return last_elem;
}
