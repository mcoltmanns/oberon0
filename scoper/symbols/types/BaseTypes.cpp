//
// Created by moltmanns on 12/21/24.
//

#include "BaseTypes.h"

IntegerType::~IntegerType() {

}

void IntegerType::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "BASE TYPE \"" << this->name_ << "\"" << std::endl;
}

StringType::~StringType() {

}

void StringType::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "BASE TYPE \"" << this->name_ << "\"" << std::endl;
}

BooleanType::~BooleanType() {

}

void BooleanType::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "BASE TYPE \"" << this->name_ << "\"" << std::endl;
}
