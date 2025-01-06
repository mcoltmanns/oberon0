//
// Created by moltmanns on 12/21/24.
//

#include "Symbol.h"

std::unique_ptr<string> Symbol::getName() const { return std::make_unique<std::string>(name_); }

std::unique_ptr<FilePos> Symbol::getPos() const { return std::make_unique<FilePos>(pos_); }

void Symbol::print(std::ostream &s) {
    s << "Generic symbol";
}
