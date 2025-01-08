//
// Created by moltmanns on 12/21/24.
//

#include "BaseTypes.h"

IntegerType::~IntegerType() {

}

void IntegerType::print(std::ostream& s) {
    s << "BASE TYPE \"" << this->name_ << "\"";
}

StringType::~StringType() {

}

void StringType::print(std::ostream& s) {
    s << "BASE TYPE \"" << this->name_ << "\"";
}

BooleanType::~BooleanType() {

}

void BooleanType::print(std::ostream& s) {
    s << "BASE TYPE \"" << this->name_ << "\"";
}
