//
// Created by moltmanns on 12/21/24.
//

#include "Variable.h"

void Variable::print(std::ostream& s) {
    s << "Variable " << name_ << " with type " << type_name_ << " declared at " << pos_.fileName << ":" << pos_.lineNo << ":" << pos_.charNo;
}
