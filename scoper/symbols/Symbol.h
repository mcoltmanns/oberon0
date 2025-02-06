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
    llvm::Value *llvm_ptr_ = nullptr; // once we start generating, this holds the pointer to the llvm value this represents

public:

    Symbol(std::string name, FilePos pos) : name_(std::move(name)), declared_at_(std::move(pos)) {}

    explicit Symbol(std::string name) : name_(std::move(name)), declared_at_() {}

    virtual ~Symbol();

    [[nodiscard]] std::string name() const;

    [[nodiscard]] std::unique_ptr<FilePos> pos() const;

    [[nodiscard]] llvm::Value *llvm_ptr() const { return llvm_ptr_; };
    void set_llvm_ptr(llvm::Value *ptr) { llvm_ptr_ = ptr; };

    virtual void print(std::ostream& s, int tabs);
};


#endif //SYMBOL_H
