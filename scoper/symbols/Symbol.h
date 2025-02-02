//
// Created by moltmanns on 12/21/24.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include <memory>
#include <string>
#include <utility>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "global.h"

enum SymbolKind {
    SYMBOL, TYPE, DERIVED_TYPE, ARRAY_TYPE, RECORD_TYPE, CONSTANT, MODULE, PROCEDURE, PASSED_PARAM, VARIABLE, BASE_BOOL, BASE_INT
};

class Symbol {
protected:
    std::string name_; // what was this called in the source?
    FilePos declared_at_; // where was this declared? (things may only be declared once)

public:
    llvm::Type *llvm_type = nullptr; // if this symbol is a type, this is its llvm type
    llvm::Value *llvm_ptr = nullptr; // if this symbol is a variable/reference, this the llvm pointer to it TODO this needs refactoring - evident why

    Symbol(std::string name, FilePos pos) : name_(std::move(name)), declared_at_(std::move(pos)) {}

    explicit Symbol(std::string name) : name_(std::move(name)), declared_at_() {}

    virtual ~Symbol();

    [[nodiscard]] std::string name() const;

    [[nodiscard]] std::unique_ptr<FilePos> pos() const;

    virtual void print(std::ostream& s, int tabs);
};


#endif //SYMBOL_H
