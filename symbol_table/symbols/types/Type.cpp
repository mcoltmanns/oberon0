//
// Created by moltmanns on 12/21/24.
//

#include "Type.h"

Type::Type(std::string name, int size) : Symbol(name, size) {
}

Type::~Type() {}

void Type::print(std::ostream &s) {
    s << "TYPE named \"" << name_ << "\" of SIZE " << size_ << " declared at " << pos_.fileName << ":" << pos_.lineNo <<
            ":" << pos_.charNo;
}
