//
// Created by moltmanns on 12/21/24.
//

#include "Variable.h"

void Variable::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "VARIABLE \"" << name_ << "\" of TYPE \"" << type_name_ << "\" and SIZE " << size_ << " and offset " << offset_ << " declared at " << declared_at_.fileName << ":" << declared_at_.lineNo << ":" << declared_at_.charNo << std::endl;
}
