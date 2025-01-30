//
// Created by moltmanns on 1/30/25.
//

#ifndef GENERATORTWO_H
#define GENERATORTWO_H
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/TargetParser/Triple.h>

#include "parser/ast/nodes/OperatorNode.h"
#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Module.h"
#include "scoper/symbols/PassedParam.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"


class GeneratorTwo {
public:
    void align_global(llvm::GlobalVariable *global, llvm::DataLayout *layout, const llvm::Align *align);

    llvm::Module *generate_module(const Module *module_symbol, llvm::LLVMContext &context,
                                  const llvm::DataLayout &data_layout,
                                  const llvm::Triple &triple);

    llvm::GlobalVariable *declare_const(std::shared_ptr<Constant> &constant);

    llvm::GlobalVariable *declare_gl_var(std::shared_ptr<Variable> &variable);

    llvm::Type *declare_derived_type(std::shared_ptr<DerivedType> &derived_type);

    llvm::StructType *declare_record_type(std::shared_ptr<RecordType> &record_type, llvm::LLVMContext &context);

    llvm::ArrayType *declare_array_type(std::shared_ptr<ArrayType> &array_type);

    llvm::AllocaInst *allocate_local_variable(std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder);

    llvm::LoadInst *load_local_variable(std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder);

    llvm::Value *load_local_variable(std::shared_ptr<PassedParam> &passed_param, llvm::IRBuilder<> &builder);

    llvm::StoreInst *store_val_to_variable(llvm::Value *val, std::shared_ptr<Variable> &variable,
                                           llvm::IRBuilder<> &builder);

    llvm::StoreInst *store_val_to_variable(llvm::Value *val, std::shared_ptr<PassedParam> &passed_param,
                                           llvm::IRBuilder<> &builder);

    llvm::Value *apply_op(llvm::Value *lhs, OperatorNode *op, llvm::Value *rhs, llvm::IRBuilder<> &builder) const;

    llvm::Value *evaluate_expression(const std::shared_ptr<Node> &expression_node, llvm::IRBuilder<> &builder,
                                     Scope &scope);

    llvm::Function *create_func(std::shared_ptr<Procedure> &procedure, llvm::Module *module, llvm::IRBuilder<> &builder);
};



#endif //GENERATORTWO_H
