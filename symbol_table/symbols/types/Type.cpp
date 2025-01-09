//
// Created by moltmanns on 12/21/24.
//

#include "Type.h"

Type::Type(const std::string &name, const int size) : Symbol(name, size) {
}

Type::~Type() {}

void Type::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "TYPE named \"" << name_ << "\" of SIZE " << size_ << " and offset " << offset_ << " declared at " << declared_at_.fileName << ":" << declared_at_.lineNo <<
            ":" << declared_at_.charNo << std::endl;
}
