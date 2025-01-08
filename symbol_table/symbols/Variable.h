//
// Created by moltmanns on 12/21/24.
//

#ifndef VARIABLE_H
#define VARIABLE_H

#include "Symbol.h"

#define VAR_SIZE 1

class Variable final : public Symbol {
private:
    std::string type_name_;

public:
    Variable(std::string name, std::string type_name, FilePos pos) : Symbol(std::move(name), std::move(pos), VAR_SIZE), type_name_(std::move(type_name)) {}

    std::string getTypeName() { return type_name_; };
    void print(std::ostream& s) override;
};



#endif //VARIABLE_H
