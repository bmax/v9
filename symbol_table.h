#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

////////////////////////////////////////////////////////////////////////////////////////////////
//
// This file contains all of the information about the symbol table.
//
// symbolTable : interacted with by the rest of the code to look up information about variables
//
// tableEntry : all of the stored information about a single variable.
//

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "type_info.h"

class symbolTable;

class tableEntry {
  friend class symbolTable;
protected:
  int type_id;       // What is the type of this variable?
  std::string name;  // Variable name used by sourcecode.
  int scope;         // What scope was this variable declared at?
  bool is_temp;      // Is this variable just temporary (internal to compiler)
  tableEntry * next; // A pointer to another variable that this one is shadowing

  union {
    float f;
    bool b;
    std::string * s;
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
  float GetFloatValue()        const { return f; }
  bool GetBoolValue()          const { return b; }
  std::string GetStringValue() const { return *s; }

  void SetType(int type) { type_id = type; }
  void SetName(std::string in_name) { name = in_name; }
  void SetScope(int in_scope) { scope = in_scope; }
  void SetNext(tableEntry * in_next) { next = in_next; }
  void SetFloatValue(float f) { this->f = f; }
  void SetBoolValue(bool b) { this->b = b; }
  void SetStringValue(std::string s) { this->s = new std::string(s); }
};



class symbolTable {
private:
  std::map<std::string, tableEntry *> tbl_map;          // A map of active variables
  std::vector<std::vector<tableEntry *> *> scope_info;  // Variables declared in each scope
  std::vector<tableEntry *> var_archive;                // Variables that are out of scope
  int cur_scope;                                        // Current scope level

public:
  symbolTable() : cur_scope(0) { 
    scope_info.push_back(new std::vector<tableEntry *>);
  }
  ~symbolTable() {
    // Clean up all variable entries
    while (cur_scope >= 0) DecScope();
    for (int i = 0; i < (int) var_archive.size(); i++) delete var_archive[i];
  }

  int GetSize() const { return (int) tbl_map.size(); }
  int GetCurScope() const { return cur_scope; }
  const std::vector<tableEntry *> & GetScopeVars(int scope) {
    if (scope < 0 || scope >= (int) scope_info.size()) {
      std::cerr << "Internal Compiler Error: Requesting vars from scope #" << scope
                << ", but only " << scope_info.size() << " scopes exist." << std::endl;
    }
    return *(scope_info[scope]);
  }

  void IncScope() {
    scope_info.push_back(new std::vector<tableEntry *>);
    cur_scope++;
  }
  void DecScope() {
    // Remove variables in the old scope and store them in the archive.
    std::vector<tableEntry *> * old_scope = scope_info.back();
    scope_info.pop_back();
    var_archive.insert(var_archive.end(), old_scope->begin(), old_scope->end());

    // Make sure to clean up the tbl_map.
    for (int i = 0; i < (int) old_scope->size(); i++) {
      tableEntry * old_entry = (*old_scope)[i];

      // If this entry is shadowing another, make shadowed version active again.
      if (old_entry->GetNext() != NULL) {
        tbl_map[old_entry->GetName()] = old_entry->GetNext();
      }

      // Otherwise just remove it from being an active variable name.
      else {
        tbl_map.erase(old_entry->GetName());
      }
    }

    delete old_scope;
    cur_scope--;
  }

  // Lookup will find an entry and return it.  If that entry is not in the table, it will return NULL
  tableEntry * Lookup(std::string in_name) {
    if (tbl_map.find(in_name) == tbl_map.end()) return NULL;
    return tbl_map[in_name];
  }

  // Determine if a variable has been declared in the current scope.
  bool InCurScope(std::string in_name) {
    if (tbl_map.find(in_name) == tbl_map.end()) return false;
    return tbl_map[in_name]->GetScope() == cur_scope;
  }

  // Insert an entry into the symbol table.
  tableEntry * AddEntry(int in_type, std::string in_name) {
    // Create the new entry for this variable.
    tableEntry * new_entry = new tableEntry(in_type, in_name);

    // If an old entry exists by this name, shadow it.
    tableEntry * old_entry = Lookup(in_name);
    if (old_entry) new_entry->SetNext(old_entry);

    // Save the information for the new entry.
    tbl_map[in_name] = new_entry;
    scope_info[cur_scope]->push_back(new_entry);
    return new_entry;
  }

  // Insert a temp variable entry into the symbol table.
  tableEntry * AddTempEntry(int in_type) {
    tableEntry * new_entry = new tableEntry(in_type);
    return new_entry;
  }

  void FreeTempVarID(int id) { (void) id; /* Nothing for now... */ }

  void RemoveEntry(tableEntry * del_var) {
    // We no longer nead this entry...
    delete del_var;
  }
};

#endif
