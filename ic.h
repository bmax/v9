#ifndef IC_H
#define IC_H

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  The classes in this file hold information about the intermediate code (IC) and how it is stored
//  in memory before the file is printed out.
//
//  The IC_Entry class holds information about a single instruction.
//
//  The IC_Array class holds an array of IC_Entries that make up the full intermediate code program.
//
//  The IC_Arg_* classes are private within IC_Entry and hold information about the arguments
//  associated with each instruction
//    IC_Arg_Base is the base class that all of the others are derived from.
//    IC_Arg_VarScalar holds info about scalar variables used as arguments (eg, s27).
//    IC_Arg_VarConst holds info about literal numbers or chars used as arguments (eg 10 or 'Q').
//    IC_Arg_VarArray holds info about array variables used as arguments (eg, a5).
//

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>


class IC_Entry {
private:
  class IC_Arg_Base {
  public:
    IC_Arg_Base() { ; }
    virtual ~IC_Arg_Base() { ; }
    
    virtual std::string AsString() = 0;
    virtual int GetID() { return -1; }
    
    virtual bool IsScalar() { return false; }
    virtual bool IsConst() { return false; }
  };

  class IC_Arg_VarScalar : public IC_Arg_Base {
  private:
    int var_id;
  public:
    IC_Arg_VarScalar(int _id) : var_id(_id) { ; }
    ~IC_Arg_VarScalar() { ; }
    
    std::string AsString() {
      std::stringstream out_str;
      out_str << "s" << var_id;
      return out_str.str();
    }
    int GetID() { return var_id; }
    
    bool IsScalar() { return true; }
  };
  
  // All constant values: int, char, or label.
  class IC_Arg_Const : public IC_Arg_Base {
  private:
    std::string value;
  public:
    IC_Arg_Const(std::string _val) : value(_val) { ; }
    ~IC_Arg_Const() { ; }
    
    std::string AsString() { return value; }
    
    bool IsConst() { return true; }
  };

  // Hint: This is only useful starting in project 5!
  class IC_Arg_VarArray : public IC_Arg_Base {
  private:
    int var_id;
  public:
    IC_Arg_VarArray(int _id) : var_id(_id) { ; }
    ~IC_Arg_VarArray() { ; }
    
    std::string AsString() {
      std::stringstream out_str;
      out_str << "a" << var_id;
      return out_str.str();
    }
    int GetID() { return var_id; }
  };

  std::string inst;
  std::string label;
  std::string comment;
  std::vector<IC_Arg_Base*> args;

public:
  IC_Entry(std::string in_inst="", std::string in_label="") : inst(in_inst), label(in_label) { ; }
  ~IC_Entry() { ; }

  const std::string & GetInstName() const { return inst; }
  const std::string & GetLabel() const { return label; }
  const std::string & GetComment() const { return comment; }
  unsigned int GetNumArgs() const { return args.size(); }

  void AddArrayArg(int id)    { args.push_back(new IC_Arg_VarArray(id)); }
  void AddConstArg(std::string id) { args.push_back(new IC_Arg_Const(id)); }
  void AddScalarArg(int id)    { args.push_back(new IC_Arg_VarScalar(id)); }

  void SetLabel(std::string in_lab) { label = in_lab; }
  void SetComment(std::string cmt) { comment = cmt; }

  void PrintIC(std::ostream & ofs);
};


class IC_Array {
private:
  std::vector <IC_Entry*> ic_array;

  // There are three types of argument requirements for instructions:
  // * VALUE  - This arg is an input value: literal numbers or chars, scalars, labels, etc.
  // * SCALAR - This arg is an output scalar variable that gets written to.
  // * ARRAY  - This arg is an array that gets somehow manipulated

  struct ArgType {
    enum type { NONE=0, VALUE, SCALAR, ARRAY };
  };
  
  std::map<std::string, std::vector<ArgType::type> > arg_type_map;
  
  // Helper method to identify types of arguments expected with each instruction
  void SetupArgs(std::string inst_name, ArgType::type type1, ArgType::type type2, ArgType::type type3) {
    std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
    arg_types.push_back(type1);
    arg_types.push_back(type2);
    arg_types.push_back(type3);
  }

  // Helper methods to add arguments to instructions, while verifying their types.
  void AddArg(IC_Entry * entry, int in_arg, ArgType::type expected_type);
  void AddArg(IC_Entry * entry, const std::string & in_arg, ArgType::type expected_type);

public:
  IC_Array() {
    // Fill out the arg types for each instruction
    SetupArgs("val_copy",    ArgType::VALUE,  ArgType::SCALAR, ArgType::NONE);
    SetupArgs("add",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("sub",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("mult",        ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("div",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("mod",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_less",   ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_gtr",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_equ",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_nequ",   ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_lte",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_gte",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("jump",        ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("jump_if_0",   ArgType::VALUE,  ArgType::VALUE,  ArgType::NONE);
    SetupArgs("jump_if_n0",  ArgType::VALUE,  ArgType::VALUE,  ArgType::NONE);
    SetupArgs("random",      ArgType::VALUE,  ArgType::SCALAR, ArgType::NONE);
    SetupArgs("out_int",     ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("out_char",    ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("nop",         ArgType::NONE,   ArgType::NONE,   ArgType::NONE);
    SetupArgs("push",        ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("pop",         ArgType::SCALAR, ArgType::NONE,   ArgType::NONE);
    SetupArgs("ar_get_idx",  ArgType::ARRAY,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("ar_set_idx",  ArgType::ARRAY,  ArgType::VALUE,  ArgType::VALUE);
    SetupArgs("ar_get_size", ArgType::ARRAY,  ArgType::SCALAR, ArgType::NONE);
    SetupArgs("ar_set_size", ArgType::ARRAY,  ArgType::VALUE,  ArgType::NONE);
    SetupArgs("ar_copy",     ArgType::ARRAY,  ArgType::ARRAY,  ArgType::NONE);
    SetupArgs("ar_push",     ArgType::ARRAY,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("ar_pop",      ArgType::ARRAY,  ArgType::NONE,   ArgType::NONE);
  }
  ~IC_Array() { ; }

  IC_Entry& AddLabel(std::string label_id, std::string cmt="");

  // All forms of Add() method.
  // Arguments can either be variables (where an int represents the variable ID) or
  // constant values (where a string holds the constant's lexeme).  And 'a' or 's' will
  // automatically be prepended to an int for a variable based on the instruction used.
  IC_Entry& Add(std::string inst, int arg1=-1, int arg2=-1, int arg3=-1, std::string cmt="");
  IC_Entry& Add(std::string inst, std::string arg1, int arg2=-1, int arg3=-1, std::string cmt="");
  IC_Entry& Add(std::string inst, int arg1, std::string arg2, int arg3=-1, std::string cmt="");
  IC_Entry& Add(std::string inst, std::string arg1, std::string arg2, int arg3=-1, std::string cmt="");
  IC_Entry& Add(std::string inst, int arg1, int arg2, std::string arg3, std::string cmt="");
  IC_Entry& Add(std::string inst, std::string arg1, int arg2, std::string arg3, std::string cmt="");
  IC_Entry& Add(std::string inst, int arg1, std::string arg2, std::string arg3, std::string cmt="");
  IC_Entry& Add(std::string inst, std::string arg1, std::string arg2, std::string arg3, std::string cmt="");

  void PrintIC(std::ostream & ofs);
};

#endif
