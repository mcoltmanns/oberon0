//
// Created by moltmanns on 1/9/25.
//

#ifndef REFERENCE_H
#define REFERENCE_H
#include "Symbol.h"

#define REF_SIZE 1

class PassedParam final : public Symbol {
private:
    std::string referenced_type_name_;
    bool is_reference_;

public:
    PassedParam(std::string name, std::string referenced_type_name, const bool is_reference, FilePos pos) : Symbol(std::move(name), std::move(pos), REF_SIZE), referenced_type_name_(std::move(referenced_type_name)), is_reference_(is_reference) {
        kind_ = SymbolKind::PASSED_PARAM;
    }

    std::string referenced_type_name() { return referenced_type_name_; }
    bool is_reference() { return is_reference_; }

    void print(std::ostream &s, int tabs) override;
};



#endif //REFERENCE_H
