#ifndef TABLE_ENTRY_H
#define TABLE_ENTRY_H

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

class symbolTable;

// All of the stored information about a single variable
class tableEntry {
  friend class symbolTable;
protected:
  int type_id;       // What is the type of this variable?
  std::string name;  // Variable name used by sourcecode.
  int scope;         // What scope was this variable declared at?
  bool is_temp;      // Is this variable just temporary (internal to compiler)
  tableEntry * next; // A pointer to another variable that this one is shadowing

  union {
    float n;
    bool b;
    std::string * s;
    std::map<std::string, tableEntry*> * o;
    std::map<unsigned int, tableEntry*> * a;
    tableEntry * r;
  };

  tableEntry(int in_type)
    : type_id (in_type)
    , name("__TEMP__")
    , scope(-1)
    , is_temp(true)
    , next(NULL)
  {
  }

  tableEntry(int in_type, const std::string in_name)
    : type_id(in_type)
    , name(in_name)
    , scope(-1)
    , is_temp(false)
    , next(NULL)
  {
  }
  virtual ~tableEntry() { ; }

public:
  int GetType()                const { return type_id; }
  std::string GetName()        const { return name; }
  int GetScope()               const { return scope; }
  bool GetTemp()               const { return is_temp; }
  tableEntry * GetNext()       const { return next; }
  float GetNumberValue()       const { return n; }
  bool GetBoolValue()          const { return b; }
  std::string GetStringValue() const { return *s; }
  tableEntry * GetReference()  const { return r; }
  tableEntry * GetProperty(std::string p) {
    if(o->find(p) == o->end()) {
      return NULL;
    }
    else {
      return (*o)[p];
    }
  }
  std::map<std::string, tableEntry*>  * GetPropertyMap() const { return o; }
  std::map<unsigned int, tableEntry*>  * GetArray() const { return a; }
  tableEntry * GetIndex(unsigned int pos) {
    if(a->find(pos) == a->end()) {
      return NULL;
    }
    else {
      return (*a)[pos];
    }
  }
  std::map<unsigned int, tableEntry*>  * GetArrayMap() const { return a; }

  void SetType(int type) { type_id = type; }
  void SetName(std::string in_name) { name = in_name; }
  void SetScope(int in_scope) { scope = in_scope; }
  void SetNext(tableEntry * in_next) { next = in_next; }
  void SetNumberValue(float n) { this->n = n; }
  void SetBoolValue(bool b) { this->b = b; }
  void SetStringValue(std::string s) { this->s = new std::string(s); }
  void SetReference(tableEntry * ref) { r = ref; }
  void SetProperty(std::string k, tableEntry * v) { (*o)[k] = v; }
  void SetIndex(unsigned int pos, tableEntry * v) { (*a)[pos] = v; }
  void InitializeObject() { o = new std::map<std::string, tableEntry*>(); }
  void InitializeArray() { a = new std::map<unsigned int, tableEntry*>(); }
};

#endif
