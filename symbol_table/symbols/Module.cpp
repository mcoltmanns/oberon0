//
// Created by moltmanns on 12/24/24.
//

#include "Module.h"

void Module::print(std::ostream &s) {
    s << "Module " << name_ << " at " << pos_.fileName << ": " << pos_.lineNo << ":" << pos_.charNo;
}
