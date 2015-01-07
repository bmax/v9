#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>

#include "type_info.h"
#include "symbol_table.h"

// The base class for all of the others, with useful virtual functions
class ASTNode {
protected:
  int type;
  int line_num;
  std::vector<ASTNode *> children;

  void SetType(int new_type) { type = new_type; }
public:
  ASTNode(int in_type) : type(in_type), line_num(-1) { ; }
  virtual ~ASTNode() {
    for (int i = 0; i < (int) children.size(); i++) delete children[i];
  }

  int GetType() { return type; }
  int GetLineNum() { return line_num; }
  ASTNode * GetChild(int id) { return children[id]; }
  int GetNumChildren() { return children.size(); }

  void SetLineNum(int _in) { line_num = _in; }
  void SetChild(int id, ASTNode * in_node) { children[id] = in_node; }
  void AddChild(ASTNode * in_child) { children.push_back(in_child); }
  void TransferChildren(ASTNode * in_node);

  // Interpret a single node and return information about the
  // variable where the results are saved.  Call children recursively.
  virtual tableEntry * Interpret(symbolTable & table) = 0;
};


// Node that will be replaced (used for argument lists)
class ASTNode_TempNode : public ASTNode {
public:
  ASTNode_TempNode(int in_type) : ASTNode(in_type) { ; }
  ~ASTNode_TempNode() { ; }
  tableEntry * Interpret(symbolTable & table) { return NULL; }
};

// Blocks of statements, including the overall program
class ASTNode_Block : public ASTNode {
public:
  ASTNode_Block() : ASTNode(Type::VOID) { ; }
  tableEntry * Interpret(symbolTable & table);
};

// Simple variale usage
class ASTNode_Variable : public ASTNode {
private:
  tableEntry * var_entry;
public:
  ASTNode_Variable(tableEntry * in_entry)
    : ASTNode(in_entry->GetType()), var_entry(in_entry) {;}

  tableEntry * GetVarEntry() { return var_entry; }
  tableEntry * Interpret(symbolTable & table);
};

// Literals for several types
class ASTNode_Literal : public ASTNode {
private:
  std::string lexeme;
public:
  ASTNode_Literal(int in_type);
  ASTNode_Literal(int in_type, std::string in_lex);
  tableEntry * Interpret(symbolTable & table);
};

// Used to access the property or index of a given object or array
class ASTNode_Property : public ASTNode {
private:
  bool assignment;
public:
  ASTNode_Property(ASTNode * obj, ASTNode * index, bool assignment);
  tableEntry * Interpret(symbolTable & table);
};

// Transfer the value of one table entry to another
class ASTNode_Assign : public ASTNode {
public:
  ASTNode_Assign(ASTNode * lhs, ASTNode * rhs);
  ~ASTNode_Assign() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// One-input math operations (unary '-')
class ASTNode_Math1 : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Math1(ASTNode * in_child, int op);
  virtual ~ASTNode_Math1() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Two-input math operations ('+', '-', '*', '/', '%')
class ASTNode_Math2 : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNode_Math2() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Comparison operators ('<', '>', '<=', '>=', '==', '!=')
class ASTNode_Comparison : public ASTNode {
protected:
  int comp_op;
public:
  ASTNode_Comparison(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNode_Comparison() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Casts a variable into a boolean value
class ASTNode_BoolCast : public ASTNode {
public:
  ASTNode_BoolCast(ASTNode * in);
  virtual ~ASTNode_BoolCast() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// One-input bool operations ('!')
class ASTNode_Bool1 : public ASTNode {
protected:
  int bool_op;
public:
  ASTNode_Bool1(ASTNode * in, int op);
  virtual ~ASTNode_Bool1() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Two-input bool operations ('&&' and '||')
class ASTNode_Bool2 : public ASTNode {
protected:
  int bool_op;
public:
  ASTNode_Bool2(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNode_Bool2() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// If-conditional node
class ASTNode_If : public ASTNode {
public:
  ASTNode_If(ASTNode * in1, ASTNode * in2, ASTNode * in3);
  virtual ~ASTNode_If() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// While-loop node
class ASTNode_While : public ASTNode {
public:
  ASTNode_While(ASTNode * in1, ASTNode * in2);
  virtual ~ASTNode_While() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// For loop node
class ASTNode_For : public ASTNode {
public:
  ASTNode_For(ASTNode * in1, ASTNode * in2, ASTNode * in3, ASTNode * in4);
  virtual ~ASTNode_For() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// For-in loop node
class ASTNode_ForIn : public ASTNode {
public:
  ASTNode_ForIn(ASTNode * in1, ASTNode * in2, ASTNode * in3);
  virtual ~ASTNode_ForIn() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Break node
class ASTNode_Break : public ASTNode {
public:
  ASTNode_Break();
  virtual ~ASTNode_Break() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Prints each child, and then a new line
class ASTNode_Print : public ASTNode {
public:
  ASTNode_Print(ASTNode * out_child);
  virtual ~ASTNode_Print() {;}

  tableEntry * Interpret(symbolTable & table);
};

// Deletes a variable and frees memory
class ASTNode_Delete : public ASTNode {
public:
  ASTNode_Delete(ASTNode * var);
  virtual ~ASTNode_Delete() {;}

  tableEntry * Interpret(symbolTable & table);
};

// Casts a variable into a string value
class ASTNode_StringCast : public ASTNode {
public:
  ASTNode_StringCast(ASTNode * in);
  virtual ~ASTNode_StringCast() { ; }

  tableEntry * Interpret(symbolTable & table);
};

// Returns the type of a variable as a string
class ASTNode_TypeOf : public ASTNode {
public:
  ASTNode_TypeOf(ASTNode * in);
  virtual ~ASTNode_TypeOf() { ; }

  tableEntry * Interpret(symbolTable & table);
};

#endif
