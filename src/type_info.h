#ifndef TYPE_INFO_H
#define TYPE_INFO_H

#include <string>

namespace Type {
  enum TypeNames { VOID=0, NUMBER, BOOL, STRING, OBJECT, ARRAY, REFERENCE,
                   NLL };

  // Convert the internal type to a string like "int"
  std::string AsString(int type);
};

#endif
