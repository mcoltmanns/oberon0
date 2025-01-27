//
// Created by moltmanns on 1/26/25.
//

#ifndef GENERATOR_H
#define GENERATOR_H
#include <llvm/IR/IRBuilder.h>
#include <llvm/Target/TargetMachine.h>

#include "parser/ast/nodes/OperatorNode.h"
#include "parser/ast/visitor/NodeVisitor.h"
#include "scoper/symbols/Module.h"


class Generator {
private:
    llvm::TargetMachine& tm_;
    llvm::LLVMContext& ctx_;
    Logger& logger_;

public:
    Generator(llvm::TargetMachine& tm, llvm::LLVMContext& ctx, Logger& logger) : tm_(tm), ctx_(ctx), logger_(logger) {}

    llvm::Module* gen_module(const Module* module_symbol) const;
    void gen_statement(const std::shared_ptr<Node> &n, llvm::IRBuilder<> &builder, llvm::Module &ll_mod, Scope &scope) const;

    llvm::Value* eval_expr(const std::shared_ptr<Node> &n, llvm::IRBuilder<> &builder, llvm::Module &ll_mod, Scope& scope) const;

    llvm::Value* apply_op(llvm::Value *lhs, OperatorNode *op, llvm::Value *rhs, llvm::IRBuilder<> &builder) const;
};

void align_global(llvm::GlobalVariable* global, llvm::DataLayout* layout, const llvm::Align* align);

#endif //GENERATOR_H
