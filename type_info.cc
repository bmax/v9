#include "type_info.h"

namespace Type {
  std::string AsString(int type) {
    switch (type) {
    case VOID: return "void";
    case NUMBER: return "number";
    case BOOL: return "bool";
    case STRING: return "string";
    };
    return "unknown";
  }
};
