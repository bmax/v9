#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "type_info.h"
#include "table_entry.h"

#include <list>

// Interacted with by the rest of the code to look up information about variables
class symbolTable {
private:
  std::map<std::string, tableEntry *> tbl_map;          // A map of active variables
  std::vector<std::vector<tableEntry *> *> scope_info;  // Variables declared in each scope
  std::vector<tableEntry *> var_archive;                // Variables that are out of scope
  std::list<tableEntry *> temp_list;                    // List of temporary table entries
  int cur_scope;                                        // Current scope level

public:
  symbolTable() : cur_scope(0) {
    scope_info.push_back(new std::vector<tableEntry *>);
  }
  ~symbolTable() {
    // Clean up all variable entries
    while (cur_scope >= 0) DecScope();
    for (int i = 0; i < (int) var_archive.size(); i++) delete var_archive[i];

    // Clean up temporary entries
    for (std::list<tableEntry *>::iterator it = temp_list.begin();
         it != temp_list.end(); it++) {
      delete *it;
    }
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
    temp_list.push_back(new_entry);
    return new_entry;
  }

  void RemoveEntry(tableEntry * del_var) {
    delete del_var;
  }
};

#endif
