//
// Created by moltmanns on 12/21/24.
//

#include "Constant.h"

void Constant::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "CONSTANT " << name_ << " with value " << value_ << " declared at " << declared_at_.fileName << ":" << declared_at_.lineNo <<
            ":" << declared_at_.charNo << std::endl;
}