//
// Created by moltmanns on 1/9/25.
//

#ifndef REFERENCE_H
#define REFERENCE_H
#include "Symbol.h"

#define REF_SIZE 1

class PassedParam final : public Symbol {
private:
    std::string type_name_;
    bool is_reference_;

public:
    llvm::Value *llvm_ptr = nullptr;
    PassedParam(std::string name, std::string referenced_type_name, const bool is_reference, FilePos pos) : Symbol(std::move(name), std::move(pos), REF_SIZE), type_name_(std::move(referenced_type_name)), is_reference_(is_reference) {
        kind_ = SymbolKind::PASSED_PARAM;
    }

    std::string type_name() { return type_name_; }
    [[nodiscard]] bool is_reference() const { return is_reference_; }

    void print(std::ostream &s, int tabs) override;
};



#endif //REFERENCE_H
