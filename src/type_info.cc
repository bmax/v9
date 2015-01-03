#include "type_info.h"

namespace Type {
  std::string AsString(int type) {
    switch (type) {
      case VOID: return "void";
      case NUMBER: return "number";
      case BOOL: return "bool";
      case STRING: return "string";
      case OBJECT: return "object";
      case ARRAY: return "array";
      case REFERENCE: return "reference";
    }
    return "unknown";
  }
};
