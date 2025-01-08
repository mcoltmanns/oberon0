//
// Created by moltmanns on 12/21/24.
//

#include "Constant.h"

void Constant::print(std::ostream& s) {
    s << "CONSTANT " << name_ << " with value " << value_ << " declared at " << pos_.fileName << ":" << pos_.lineNo <<
            ":" << pos_.charNo;
}