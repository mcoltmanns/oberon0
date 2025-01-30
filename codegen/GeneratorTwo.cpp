//
// Created by moltmanns on 1/30/25.
//

#include "GeneratorTwo.h"

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/TargetParser/Triple.h>

#include "Generator.h"
#include "LLVMMachine.h"
#include "parser/ast/nodes/LiteralNode.h"
#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Module.h"
#include "scoper/symbols/PassedParam.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"

void GeneratorTwo::align_global(llvm::GlobalVariable *global, llvm::DataLayout *layout, const llvm::Align *align) {
    auto size = layout->getTypeStoreSize(global->getType());
    if (size >= align->value()) {
        global->setAlignment(llvm::MaybeAlign(*align));
    }
    else {
        #ifdef _LLVM_LEGACY
        global->setAlignment(llvm::MaybeAlign(layout->getPrefTypeAlignment(global->getType())));
        #else
        global->setAlignment(llvm::MaybeAlign(layout->getPrefTypeAlign(global->getType())));
        #endif
    }
}

llvm::Module* GeneratorTwo::generate_module(const Module *module_symbol, llvm::LLVMContext &context, const llvm::DataLayout& data_layout, const llvm::Triple& triple) {
    auto module = new llvm::Module(module_symbol->name(), context);
    module->setDataLayout(data_layout);
    module->setTargetTriple(triple.getTriple());

    llvm::IRBuilder<> builder(context);

    BASIC_TYPE_INT->llvm_type = builder.getInt32Ty();
    BASIC_TYPE_BOOL->llvm_type = builder.getInt1Ty();

    for (const auto &symbol : module_symbol->scope_->table_) {
        if (auto constant = dynamic_pointer_cast<Constant>(symbol); constant) {
            module->insertGlobalVariable(declare_const(constant));
        }
        else if (auto derived_type = dynamic_pointer_cast<DerivedType>(symbol); derived_type) {
            derived_type->llvm_type = derived_type->base_type()->llvm_type;
        }
        else if (auto record_type = dynamic_pointer_cast<RecordType>(symbol); record_type) {
            std::vector<llvm::Type*> fields;
            for (const auto &field : record_type->fields()) {
                fields.push_back(field.second->llvm_type);
            }
            record_type->llvm_type = llvm::StructType::get(context, fields, false);
        }
        else if (auto array_type = dynamic_pointer_cast<ArrayType>(symbol); array_type) {
            array_type->llvm_type = llvm::ArrayType::get(array_type->base_type()->llvm_type, static_cast<uint64_t>(array_type->length()));
        }
        else if (auto variable = dynamic_pointer_cast<Variable>(symbol); variable) {
            module->insertGlobalVariable(declare_global_variable(variable));
        }
        else if (auto procedure = dynamic_pointer_cast<Procedure>(symbol); procedure) {
            create_func(procedure, module, builder);
        }
    }

    return module;
}

llvm::GlobalVariable* GeneratorTwo::declare_const(std::shared_ptr<Constant> &constant) {
    auto* llvm_const = new llvm::GlobalVariable(BASIC_TYPE_INT->llvm_type, true, llvm::GlobalVariable::InternalLinkage, llvm::Constant::getIntegerValue(BASIC_TYPE_INT->llvm_type, constant->toAPInt(BASIC_TYPE_INT->llvm_type->getIntegerBitWidth())), constant->name());
    constant->llvm_ptr = llvm_const;
    return llvm_const;
}

llvm::GlobalVariable* GeneratorTwo::declare_global_variable(std::shared_ptr<Variable> &variable) {
    auto* llvm_var = new llvm::GlobalVariable(variable->type()->llvm_type, false, llvm::GlobalVariable::InternalLinkage, llvm::Constant::getNullValue(variable->type()->llvm_type), variable->name());
    variable->llvm_ptr = llvm_var;
    return llvm_var;
}

llvm::Type* GeneratorTwo::declare_derived_type(std::shared_ptr<DerivedType> &derived_type) {
    derived_type->llvm_type = derived_type->base_type()->llvm_type;
    return derived_type->llvm_type;
}

llvm::StructType* GeneratorTwo::declare_record_type(std::shared_ptr<RecordType> &record_type, llvm::LLVMContext &context) {
    std::vector<llvm::Type*> fields;
    for (const auto &field : record_type->fields()) {
        fields.push_back(field.second->llvm_type);
    }
    llvm::StructType *t = llvm::StructType::get(context, fields, false);
    record_type->llvm_type = t;
    return t;
}

llvm::ArrayType* GeneratorTwo::declare_array_type(std::shared_ptr<ArrayType> &array_type) {
    llvm::ArrayType* t = llvm::ArrayType::get(array_type->base_type()->llvm_type, static_cast<uint64_t>(array_type->length()));
    array_type->llvm_type = t;
    return t;
}

llvm::AllocaInst* GeneratorTwo::allocate_local_variable(std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder) {
    llvm::AllocaInst* alloc_place = builder.CreateAlloca(variable->type()->llvm_type, nullptr, variable->name());
    variable->llvm_ptr = alloc_place;
    return alloc_place;
}

// local variables are easy - just grab the type/ptr out of the symbol table and generate a load instruction
llvm::LoadInst* GeneratorTwo::load_local_variable(std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder) {
    llvm::LoadInst* load_place = builder.CreateLoad(variable->type()->llvm_type, variable->llvm_ptr, variable->name());
    return load_place;
}

// parameters are a little tougher
// but not too awful
llvm::Value* GeneratorTwo::load_local_variable(std::shared_ptr<PassedParam> &passed_param, llvm::IRBuilder<> &builder) {
    auto func = builder.GetInsertBlock()->getParent(); // what function are we working in?
    auto param = func->arg_begin() + passed_param->index(); // find the parameter
    if (passed_param->is_reference()) { // if the parameter is a reference, load it
        llvm::LoadInst* param_load = builder.CreateLoad(passed_param->type()->llvm_type, param, passed_param->name());
        return param_load;
    }
    return param; // otherwise just return the actual parameter
}

llvm::StoreInst* GeneratorTwo::store_val_to_variable(llvm::Value* val, std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder) {
    llvm::StoreInst* store_place = builder.CreateStore(val, variable->llvm_ptr);
    return store_place;
}

llvm::StoreInst* GeneratorTwo::store_val_to_variable(llvm::Value* val, std::shared_ptr<PassedParam> &passed_param, llvm::IRBuilder<> &builder) {
    auto func = builder.GetInsertBlock()->getParent();
    auto param = func->arg_begin() + passed_param->index();
    return builder.CreateStore(val, param);
}

// apply an op node to two loaded values
llvm::Value * GeneratorTwo::apply_op(llvm::Value *lhs, OperatorNode *op, llvm::Value *rhs, llvm::IRBuilder<> &builder) const {
    switch (op->operation()) {
        case EQ: {
            return builder.CreateCmp(llvm::CmpInst::ICMP_EQ, lhs, rhs);
        }
        case NEQ: {
            return builder.CreateCmp(llvm::CmpInst::ICMP_NE, lhs, rhs);
        }
        case LT: {
            return builder.CreateCmp(llvm::CmpInst::ICMP_SLT, lhs, rhs);
        }
        case LEQ: {
            return builder.CreateCmp(llvm::CmpInst::ICMP_SLE, lhs, rhs);
        }
        case GT: {
            return builder.CreateCmp(llvm::CmpInst::ICMP_SGT, lhs, rhs);
        }
        case GEQ: {
            return builder.CreateCmp(llvm::CmpInst::ICMP_SGE, lhs, rhs);
        }
        case AND: {
            return builder.CreateAnd(lhs, rhs);
        }
        case OR: {
            return builder.CreateOr(lhs, rhs);
        }
        case NOT: {
            return builder.CreateNot(lhs);
        }
        case TIMES: {
            return builder.CreateMul(lhs, rhs);
        }
        case DIV: {
            return builder.CreateSDiv(lhs, rhs);
        }
        case MOD: {
            return builder.CreateSRem(lhs, rhs);
        }
        case PLUS: {
            return builder.CreateAdd(lhs, rhs);
        }
        case PLUS_UNARY: {
            return rhs;
        }
        case MINUS: {
            return builder.CreateSub(lhs, rhs);
        }
        case MINUS_UNARY: {
            return builder.CreateNeg(lhs);
        }
        default: {
            return nullptr;
        }
    }
}

llvm::Value* GeneratorTwo::evaluate_expression(const std::shared_ptr<Node> &expression_node, llvm::IRBuilder<> &builder, Scope &scope) {
    switch (expression_node->type()) {
        case NodeType::literal: {
            auto lit = std::dynamic_pointer_cast<LiteralNode>(expression_node);
            if (lit->is_bool()) return builder.getInt1(lit->value());
            return builder.getInt32(lit->value());
        }
        case NodeType::ident: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(expression_node);
            if (auto var = scope.lookup_by_name<Variable>(ident->name())) {
                return load_local_variable(var, builder);
            }
            if (auto param = scope.lookup_by_name<PassedParam>(ident->name())) {
                return load_local_variable(param, builder);
            }
            if (auto constant = scope.lookup_by_name<Constant>(ident->name())) {
                return builder.getInt32(constant->value());
            }
            return nullptr;
        }
        default: {
            std::stack<OperatorNode*> operators;
            std::stack<llvm::Value*> values;
            for (const auto &child : expression_node->children()) {
                if (child) {
                    if (child->type() == NodeType::op)
                        operators.push(dynamic_cast<OperatorNode*>(child.get()));
                    else
                        values.push(evaluate_expression(child, builder, scope));
                }
            }
            while (!operators.empty()) {
                auto op = operators.top();
                operators.pop();
                if (op->operation() == PLUS_UNARY || op->operation() == MINUS_UNARY) {
                    auto lhs = values.top();
                    values.pop();
                    values.push(apply_op(lhs, op, nullptr, builder));
                }
                else {
                    auto lhs = values.top();
                    values.pop();
                    auto rhs = values.top();
                    values.pop();
                    values.push(apply_op(lhs, op, rhs, builder));
                }
            }
            return values.top();
        }
    }
}

llvm::Value* GeneratorTwo::generate_statement(const std::shared_ptr<Node> &statement, llvm::IRBuilder<> &builder, Scope &scope, llvm::Function* function) {
    switch (statement->type()) {
        case NodeType::assignment: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(statement->children().front());
            auto expr = statement->children().back();

            auto expr_res = evaluate_expression(expr, builder, scope);

            if (auto variable = scope.lookup_by_name<Variable>(ident->name()); variable) {
                return store_val_to_variable(expr_res, variable, builder);
            }
            if (auto param = scope.lookup_by_name<PassedParam>(ident->name()); param) {
                return store_val_to_variable(expr_res, param, builder);
            }
            return nullptr;
        }
        case NodeType::proc_call: {
            return nullptr;
        }
        case NodeType::if_statement: {
            auto if_cond = evaluate_expression(statement->children().front(), builder, scope); // evaluate the condition
            auto if_block = llvm::BasicBlock::Create(builder.getContext(), "if", function); // create the block for the condition
            llvm::BasicBlock* else_block = nullptr;
            if (statement->children().size() > 2) {
                else_block = llvm::BasicBlock::Create(builder.getContext(), "else", function); // create the block for the noncondition
            }
            auto after_block = llvm::BasicBlock::Create(builder.getContext(), "after", function); // create the block to jump to afterwards

            // if there is an else, jump there if the condition fails
            if (else_block) {
                builder.CreateCondBr(if_cond, if_block, else_block);
            }
            else { // otherwise, jump to afterwards
                builder.CreateCondBr(if_cond, if_block, after_block);
            }

            // remeber:
            // if
            //  expression      (0)
            //  statement seq   (1)
            //  if              (2)
            //      expression
            //      statement seq

            // generate statements in the if block
            builder.SetInsertPoint(if_block);
            for (const auto &child : statement->children().at(1)->children()) {
                generate_statement(child, builder, scope, function);
            }
            builder.CreateBr(after_block);

            // leave the builder at the end
            builder.SetInsertPoint(after_block);
            return if_block;
        }
        default: {
            return nullptr;
        }
    }
}

llvm::Function* GeneratorTwo::create_func(std::shared_ptr<Procedure> &procedure, llvm::Module* module, llvm::IRBuilder<> &builder) {
    // construct function signature
    std::vector<llvm::Type*> arg_types;
    for (const auto &param : procedure->params_) {
        if (const auto param_symbol = procedure->scope_->lookup_by_name<PassedParam>(param.first); param_symbol && param_symbol->is_reference()) {
            arg_types.push_back(builder.getPtrTy());
        }
        else {
            const auto param_type = procedure->scope_->outer_->lookup_by_name<Type>(param.second);
            arg_types.push_back(param_type->llvm_type);
        }
    }
    auto* func_type = llvm::FunctionType::get(builder.getVoidTy(), arg_types, false);
    auto* func = llvm::Function::Create(func_type, llvm::Function::InternalLinkage, procedure->name(), module); // declare function in the module
    procedure->llvm_function = func;
    auto entry = llvm::BasicBlock::Create(builder.getContext(), procedure->name(), func);
    builder.SetInsertPoint(entry); // insert after function start

    // process function declarations
    for (const auto& symbol : procedure->scope_->table_) {
        if (auto constant = dynamic_pointer_cast<Constant>(symbol); constant) {
            module->insertGlobalVariable(declare_const(constant));
        }
        else if (auto derived_type = dynamic_pointer_cast<DerivedType>(symbol); derived_type) {
            derived_type->llvm_type = derived_type->base_type()->llvm_type;
        }
        else if (auto record_type = dynamic_pointer_cast<RecordType>(symbol); record_type) {
            std::vector<llvm::Type*> fields;
            for (const auto &field : record_type->fields()) {
                fields.push_back(field.second->llvm_type);
            }
            record_type->llvm_type = llvm::StructType::get(builder.getContext(), fields, false);
        }
        else if (auto array_type = dynamic_pointer_cast<ArrayType>(symbol); array_type) {
            array_type->llvm_type = llvm::ArrayType::get(array_type->base_type()->llvm_type, static_cast<uint64_t>(array_type->length()));
        }
        else if (auto variable = dynamic_pointer_cast<Variable>(symbol); variable) {
            allocate_local_variable(variable, builder);
        }
    }

    // process function statements
    for (const auto &statement : procedure->sseq_node_->children()) {
        generate_statement(statement, builder, *procedure->scope_, func);
    }

    return func;
}
