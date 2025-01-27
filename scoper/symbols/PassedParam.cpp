//
// Created by moltmanns on 1/9/25.
//

#include "PassedParam.h"

void PassedParam::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << (is_reference_ ? "REFERENCE" : "COPY") << " PARAM \"" << name_ << "\" of TYPE \"" << referenced_type_name_ << "\" and SIZE " << size_ << " and offset " << offset_ << " declared at " << declared_at_.fileName << ":" << declared_at_.lineNo << ":" << declared_at_.charNo << std::endl;
}
