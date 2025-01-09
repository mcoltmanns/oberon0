//
// Created by moltmanns on 1/9/25.
//

#ifndef REFERENCE_H
#define REFERENCE_H
#include "Symbol.h"

#define REF_SIZE 1

class Reference final : public Symbol {
private:
    std::string referenced_type_name_;

public:
    Reference(std::string name, std::string referenced_type_name, FilePos pos) : Symbol(std::move(name), std::move(pos), REF_SIZE), referenced_type_name_(std::move(referenced_type_name)) {}

    std::string referenced_type_name() { return referenced_type_name_; }

    void print(std::ostream &s, int tabs) override;
};



#endif //REFERENCE_H
