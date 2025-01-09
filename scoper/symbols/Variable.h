//
// Created by moltmanns on 12/21/24.
//

#ifndef VARIABLE_H
#define VARIABLE_H

#include "Symbol.h"
#include "types/Type.h"

#define VAR_SIZE 1

class Variable final : public Symbol {
private:
    std::shared_ptr<Type> type_;

public:
    Variable(std::string name, std::shared_ptr<Type> type, FilePos pos, const int size) : Symbol(std::move(name), std::move(pos), size), type_(std::move(type)) {}

    std::shared_ptr<Type> type() { return type_; };
    void print(std::ostream &s, int tabs) override;
};



#endif //VARIABLE_H
