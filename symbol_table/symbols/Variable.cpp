//
// Created by moltmanns on 12/21/24.
//

#include "Variable.h"

void Variable::print(std::ostream& s) {
    s << "VARIABLE \"" << name_ << "\" of TYPE \"" << type_name_ << "\" and SIZE " << size_ << " declared at " << pos_.fileName << ":" << pos_.lineNo << ":" << pos_.charNo;
}
