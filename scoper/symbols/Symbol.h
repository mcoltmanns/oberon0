//
// Created by moltmanns on 12/21/24.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "global.h"
#include "parser/ast/nodes/Node.h"

enum SymbolKind {
    SYMBOL, TYPE, DERIVED_TYPE, ARRAY_TYPE, RECORD_TYPE, CONSTANT, MODULE, PROCEDURE, PASSED_PARAM, VARIABLE, BASE_BOOL, BASE_INT
};

class Symbol {
protected:
    std::string name_;
    FilePos declared_at_; // where was this declared? (things may only be declared once)

public:
    int size_; // how many memory units does this thing take up?
    int offset_; // at what point in the scope AR is this thing kept?
    std::vector<std::shared_ptr<Node>> uses_; // where in the program is this used?
    SymbolKind kind_ = SymbolKind::SYMBOL; // what kind of symbol is this?
    llvm::Type *llvm_type = nullptr; // what kind of thing is this according to llvm?
    llvm::Value *llvm_ptr = nullptr;

    Symbol(std::string name, FilePos pos, const int size) : name_(std::move(name)), declared_at_(std::move(pos)), size_(size), offset_(0) {}

    explicit Symbol(std::string name, const int size) : name_(std::move(name)), declared_at_(), size_(size), offset_(0) {}

    virtual ~Symbol();

    [[nodiscard]] std::string name() const;

    [[nodiscard]] std::unique_ptr<FilePos> pos() const;

    virtual void print(std::ostream& s, int tabs);
};


#endif //SYMBOL_H
