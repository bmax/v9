#ifndef AST_H
#define AST_H

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  The classes in this file hold info about the nodes that form the Abstract Syntax Tree (AST)
//
//  ASTNode : The base class for all of the others, with useful virtual functions.
//
//  ASTNode_TempNode : AST Node that will be replaced (used for argument lists).
//  ASTNode_Block : Blocks of statements, including the overall program.
//  ASTNode_Variable : Leaf node containing a variable.
//  ASTNode_Literal : Leaf node contiaing a literal value.
//  ASTNode_Assign : Assignements
//  ASTNode_Math1 : One-input math operations (unary '-' and '!')
//  ASTNode_Math2 : Two-input math operations ('+', '-', '*', '/', '%', and comparisons)
//  ASTNode_Bool2 : Two-input bool operations ('&&' and '||')
//  ASTNode_If : If-conditional node.
//  ASTNode_While : While-loop node.
//  ASTNode_Break : Break node
//  ASTNode_Print : Print command
//

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>

#include "type_info.h"
#include "symbol_table.h"

class ASTNode {
protected:
  int type;                         // What type should this node pass up?
  int line_num;                     // What line of the source program generated this node?
  std::vector<ASTNode *> children;  // What sub-trees does this node have?

  void SetType(int new_type) { type = new_type; } // Use inside constructor only!
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


// A placeholder node in the AST.
class ASTNode_TempNode : public ASTNode {
public:
  ASTNode_TempNode(int in_type) : ASTNode(in_type) { ; }
  ~ASTNode_TempNode() { ; }
  tableEntry * Interpret(symbolTable & table) { return NULL; }
};

// Block...
class ASTNode_Block : public ASTNode {
public:
  ASTNode_Block() : ASTNode(Type::VOID) { ; }
  tableEntry * Interpret(symbolTable & table);
};

// Leaves...
class ASTNode_Variable : public ASTNode {
private:
  tableEntry * var_entry;
public:
  ASTNode_Variable(tableEntry * in_entry)
    : ASTNode(in_entry->GetType()), var_entry(in_entry) {;}

  tableEntry * GetVarEntry() { return var_entry; }
  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Literal : public ASTNode {
private:
  std::string lexeme;     // When we print, how should this node look?
public:
  ASTNode_Literal(int in_type, std::string in_lex);
  tableEntry * Interpret(symbolTable & table);
};

// Math...

class ASTNode_Assign : public ASTNode {
public:
  ASTNode_Assign(ASTNode * lhs, ASTNode * rhs);
  ~ASTNode_Assign() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Math1 : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Math1(ASTNode * in_child, int op);
  virtual ~ASTNode_Math1() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Math2 : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNode_Math2() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Comparison : public ASTNode {
protected:
  int comp_op;
public:
  ASTNode_Comparison(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNode_Comparison() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_BoolCast : public ASTNode {
public:
  ASTNode_BoolCast(ASTNode * in);
  virtual ~ASTNode_BoolCast() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Bool1 : public ASTNode {
protected:
  int bool_op;
public:
  ASTNode_Bool1(ASTNode * in, int op);
  virtual ~ASTNode_Bool1() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Bool2 : public ASTNode {
protected:
  int bool_op;
public:
  ASTNode_Bool2(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNode_Bool2() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_If : public ASTNode {
public:
  ASTNode_If(ASTNode * in1, ASTNode * in2, ASTNode * in3);
  virtual ~ASTNode_If() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_While : public ASTNode {
public:
  ASTNode_While(ASTNode * in1, ASTNode * in2);
  virtual ~ASTNode_While() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Break : public ASTNode {
public:
  ASTNode_Break();
  virtual ~ASTNode_Break() { ; }

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_Print : public ASTNode {
public:
  ASTNode_Print(ASTNode * out_child);
  virtual ~ASTNode_Print() {;}

  tableEntry * Interpret(symbolTable & table);
};

class ASTNode_StringCast : public ASTNode {
public:
  ASTNode_StringCast(ASTNode * in);
  virtual ~ASTNode_StringCast() { ; }

  tableEntry * Interpret(symbolTable & table);
};

#endif
