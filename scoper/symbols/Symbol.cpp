//
// Created by moltmanns on 12/21/24.
//

#include "Symbol.h"

Symbol::~Symbol() {}

std::string Symbol::name() const { return name_; }

std::unique_ptr<FilePos> Symbol::pos() const { return std::make_unique<FilePos>(declared_at_); }

void Symbol::print(std::ostream& s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "Unknown symbol" << std::endl;
}
