#include "type_info.h"

namespace Type {
  std::string AsString(int type) {
    switch (type) {
    case VOID: return "void";
    case NUM: return "num";
    case BOOL: return "bool";
    case STRING: return "string";
    };
    return "unknown";
  }
};
