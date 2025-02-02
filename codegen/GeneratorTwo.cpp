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

// File is a little ugly! Rife with lots of repetition that could've probably been avoided had this been written with generics in mind
// Programmers that don't use generics when they should do not get a little handful of cranberries as a treat.
// LET IT BE KNOWN! the fellas over at llvm should not have made it possible
// to call BasicBlock::create() without passing in a parent function! took me 2 days to fix the stupid typo!
// but good programmers get a little handful of cranberries as a little treat for killing evil bugs

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

    // process declarations
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

    // entry basic block
    auto main = module->getOrInsertFunction("main", builder.getVoidTy());
    auto function = llvm::cast<llvm::Function>(main.getCallee());
    auto entry = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(entry);

    // main statements
    for (const auto &statement : module_symbol->sseq_node->children()) {
        generate_statement(statement, builder, *module_symbol->scope_, function);
    }

    return module;
}

llvm::GlobalVariable* GeneratorTwo::declare_const(const std::shared_ptr<Constant> &constant) {
    auto* llvm_const = new llvm::GlobalVariable(BASIC_TYPE_INT->llvm_type, true, llvm::GlobalVariable::InternalLinkage, llvm::Constant::getIntegerValue(BASIC_TYPE_INT->llvm_type, constant->toAPInt(BASIC_TYPE_INT->llvm_type->getIntegerBitWidth())), constant->name());
    constant->llvm_ptr = llvm_const;
    return llvm_const;
}

llvm::GlobalVariable* GeneratorTwo::declare_global_variable(const std::shared_ptr<Variable> &variable) {
    auto* llvm_var = new llvm::GlobalVariable(variable->type()->llvm_type, false, llvm::GlobalVariable::InternalLinkage, llvm::Constant::getNullValue(variable->type()->llvm_type), variable->name());
    variable->llvm_ptr = llvm_var;
    return llvm_var;
}

llvm::Type* GeneratorTwo::declare_derived_type(const std::shared_ptr<DerivedType> &derived_type) {
    derived_type->llvm_type = derived_type->base_type()->llvm_type;
    return derived_type->llvm_type;
}

llvm::StructType* GeneratorTwo::declare_record_type(const std::shared_ptr<RecordType> &record_type, llvm::LLVMContext &context) {
    std::vector<llvm::Type*> fields;
    for (const auto &field : record_type->fields()) {
        fields.push_back(field.second->llvm_type);
    }
    llvm::StructType *t = llvm::StructType::get(context, fields, false);
    record_type->llvm_type = t;
    return t;
}

llvm::ArrayType* GeneratorTwo::declare_array_type(const std::shared_ptr<ArrayType> &array_type) {
    llvm::ArrayType* t = llvm::ArrayType::get(array_type->base_type()->llvm_type, static_cast<uint64_t>(array_type->length()));
    array_type->llvm_type = t;
    return t;
}

llvm::AllocaInst* GeneratorTwo::allocate_local_variable(const std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder) {
    llvm::AllocaInst* alloc_place = builder.CreateAlloca(variable->type()->llvm_type, nullptr, variable->name());
    variable->llvm_ptr = alloc_place;
    return alloc_place;
}

llvm::LoadInst* GeneratorTwo::load_local_variable(const std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder) {
    llvm::LoadInst* load_place = builder.CreateLoad(variable->type()->llvm_type, variable->llvm_ptr, variable->name());
    return load_place;
}

// always dereference parameters when you load them.
llvm::Value* GeneratorTwo::load_local_variable(const std::shared_ptr<PassedParam> &passed_param, llvm::IRBuilder<> &builder) {
    auto func = builder.GetInsertBlock()->getParent(); // what function are we working in?
    auto param = func->arg_begin() + passed_param->index(); // find the parameter
    return builder.CreateLoad(passed_param->type()->llvm_type, param, passed_param->name());
}

// should be followed by a call to a load or store instruction - this returns a pointer
std::pair<llvm::Value *, llvm::Type *> GeneratorTwo::get_ptr_from_index(const std::shared_ptr<Variable> &variable, const std::shared_ptr<Node> &selector_block, Scope &scope, llvm::IRBuilder<> &builder) {
    llvm::Value *most_recent_load = variable->llvm_ptr;
    llvm::Value *index = nullptr;
    llvm::Type *end_ptr_type = nullptr;
    for (const std::shared_ptr<Node> &selector : selector_block->children()) {
        if (selector->type() == NodeType::sel_index) {
            auto array_type = dynamic_pointer_cast<ArrayType>(variable->type());
            end_ptr_type = array_type->base_type()->llvm_type;
            index = evaluate_expression(selector->children().front(), builder, scope);
            most_recent_load = builder.CreateInBoundsGEP(array_type->llvm_type, most_recent_load, { builder.getInt32(0), index });
        }
        else if (selector->type() == NodeType::sel_field) {
            auto rec_type = std::dynamic_pointer_cast<RecordType>(variable->type());
            auto field_ident = std::dynamic_pointer_cast<IdentNode>(selector->children().front());
            auto field_index = static_cast<uint32_t>(rec_type->get_field_index_by_name(field_ident->name()));
            end_ptr_type = rec_type->get_field_type_by_name(field_ident->name())->llvm_type;
            most_recent_load = builder.CreateInBoundsGEP(rec_type->llvm_type, most_recent_load, { builder.getInt32(0), builder.getInt32(field_index) });
        }
    }
    return { most_recent_load, end_ptr_type };
}

// should be followed by a call to a load or store instruction - this returns a pointer, and the type of the thing that pointer points to
std::pair<llvm::Value *, llvm::Type *> GeneratorTwo::get_ptr_from_index(
    const std::shared_ptr<PassedParam> &passed_param, const std::shared_ptr<Node> &selector_block, Scope &scope,
    llvm::IRBuilder<> &builder) {
    auto func = builder.GetInsertBlock()->getParent();
    llvm::Value *most_recent_load = func->arg_begin() + passed_param->index();
    llvm::Type *end_ptr_type = nullptr;
    for (const std::shared_ptr<Node> &selector : selector_block->children()) {
        if (selector->type() == NodeType::sel_index) {
            auto array_type = std::dynamic_pointer_cast<ArrayType>(passed_param->type());
            end_ptr_type = array_type->base_type()->llvm_type;
            auto index = evaluate_expression(selector->children().front(), builder, scope);
            most_recent_load = builder.CreateInBoundsGEP(array_type->llvm_type, most_recent_load, { builder.getInt32(0), index });
        }
        else if (selector->type() == NodeType::sel_field) {
            auto rec_type = std::dynamic_pointer_cast<RecordType>(passed_param->type());
            auto field_ident = std::dynamic_pointer_cast<IdentNode>(selector->children().front());
            auto field_index = static_cast<uint32_t>(rec_type->get_field_index_by_name(field_ident->name()));
            end_ptr_type = rec_type->get_field_type_by_name(field_ident->name())->llvm_type;
            most_recent_load = builder.CreateInBoundsGEP(rec_type->llvm_type, most_recent_load, { builder.getInt32(0), builder.getInt32(field_index) });
        }
    }
    return { most_recent_load, end_ptr_type };
}

llvm::StoreInst* GeneratorTwo::store_val_to_variable(llvm::Value* val, const std::shared_ptr<Variable> &variable, llvm::IRBuilder<> &builder) {
    llvm::StoreInst* store_place = builder.CreateStore(val, variable->llvm_ptr);
    return store_place;
}

llvm::StoreInst* GeneratorTwo::store_val_to_variable(llvm::Value* val, std::shared_ptr<Variable> &variable, const std::shared_ptr<Node>& selector_block, Scope &scope, llvm::IRBuilder<> &builder) {
    auto loaded = get_ptr_from_index(variable, selector_block, scope, builder);
    return builder.CreateStore(val, loaded.first);
}

// again, always store to the thing pointed to by the parameter (if it is a reference)
llvm::StoreInst* GeneratorTwo::store_val_to_variable(llvm::Value* val, const std::shared_ptr<PassedParam> &passed_param, llvm::IRBuilder<> &builder) {
    auto func = builder.GetInsertBlock()->getParent();
    auto param = func->arg_begin() + passed_param->index();
    return builder.CreateStore(val, param);
}

llvm::StoreInst* GeneratorTwo::store_val_to_variable(llvm::Value* val, const std::shared_ptr<PassedParam> &passed_param, const std::shared_ptr<Node>& selector_block, Scope &scope, llvm::IRBuilder<> &builder) {
    auto loaded = get_ptr_from_index(passed_param, selector_block, scope, builder);
    return builder.CreateStore(val, loaded.first);
}

// apply an op node to two loaded values
// not pointers!!
llvm::Value * GeneratorTwo::apply_op(llvm::Value *lhs, const OperatorNode *op, llvm::Value *rhs, llvm::IRBuilder<> &builder) const {
    if (rhs->getType() == builder.getPtrTy() || lhs->getType() == builder.getPtrTy()) {
        fprintf(stderr, "CANNOT OPERATE ON POINTERS\n");
    }
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

// evaluate an expression
// dereferences pointers!
llvm::Value* GeneratorTwo::evaluate_expression(const std::shared_ptr<Node> &expression_node, llvm::IRBuilder<> &builder, Scope &scope) {
    switch (expression_node->type()) {
        case NodeType::literal: {
            auto lit = std::dynamic_pointer_cast<LiteralNode>(expression_node);
            if (lit->is_bool()) return builder.getInt1(lit->value());
            return builder.getInt32(lit->value());
        }
        case NodeType::ident: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(expression_node);
            llvm::Value* loaded = nullptr;
            // first load the thing
            if (auto param = scope.lookup_by_name<PassedParam>(ident->name())) {
                if (ident->selector_block()) {
                    auto [ptr, ptr_t] = get_ptr_from_index(param, ident->selector_block(), scope, builder);
                    loaded = builder.CreateLoad(ptr_t, ptr);
                }
                else {
                    loaded = load_local_variable(param, builder);
                }
            }
            if (auto constant = scope.lookup_by_name<Constant>(ident->name())) {
                loaded = builder.getInt32(constant->value());
            }
            if (auto var = scope.lookup_by_name<Variable>(ident->name())) {
                if (ident->selector_block()) {
                    auto [ptr, ptr_t] = get_ptr_from_index(var, ident->selector_block(), scope, builder);
                    loaded = builder.CreateLoad(ptr_t, ptr);
                }
                else {
                    loaded = load_local_variable(var, builder);
                }
            }
            return loaded;
        }
        default: {
            std::stack<OperatorNode*> operators;
            std::stack<llvm::Value*> values;
            for (const auto &child : expression_node->children()) {
                if (child) {
                    if (child->type() == NodeType::op)
                        operators.push(dynamic_cast<OperatorNode*>(child.get()));
                    else {
                        values.push(evaluate_expression(child, builder, scope));
                    }
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
                    // we push left-right, so we have to pop right-left
                    auto rhs = values.top();
                    values.pop();
                    auto lhs = values.top();
                    values.pop();
                    values.push(apply_op(lhs, op, rhs, builder));
                }
            }
            return values.top();
        }
    }
}

llvm::Value* GeneratorTwo::generate_statement(const std::shared_ptr<Node> &statement, llvm::IRBuilder<> &builder, Scope &scope, llvm::Function* function) {
    llvm::Value* stmt = nullptr;
    switch (statement->type()) {
        case NodeType::assignment: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(statement->children().front());
            auto expr = statement->children().back();

            auto expr_res = evaluate_expression(expr, builder, scope);

            if (auto variable = scope.lookup_by_name<Variable>(ident->name()); variable) {
                if (ident->selector_block()) {
                    stmt = store_val_to_variable(expr_res, variable, ident->selector_block(), scope, builder);
                }
                else stmt = store_val_to_variable(expr_res, variable, builder);
            }
            if (auto param = scope.lookup_by_name<PassedParam>(ident->name()); param) {
                if (ident->selector_block()) {
                    stmt = store_val_to_variable(expr_res, param, ident->selector_block(), scope, builder);
                }
                else stmt = store_val_to_variable(expr_res, param, builder);
            }
            break;
        }
        case NodeType::proc_call: {
            // proc_call is proc ident, and then any number of arguments (these can be expressions, literals, etc)
            auto ident = std::dynamic_pointer_cast<IdentNode>(statement->children().front());

            auto proc = scope.lookup_by_name<Procedure>(ident->name());

            std::vector<llvm::Value*> args;
            for (uint64_t i = 1; i < statement->children().size(); i++) {
                auto passed_to = std::dynamic_pointer_cast<PassedParam>(proc->scope_->lookup_by_index(i - 1));
                if (passed_to->is_reference()) {
                    // if we are passing into a reference, the things we're passing must be declared values, and they can't be constants
                    // TODO: this should be enforced by the typechecker!
                    auto param_ident = std::dynamic_pointer_cast<IdentNode>(statement->children().at(i));
                    auto passed_in = scope.lookup_by_name<Symbol>(param_ident->name());
                    args.push_back(passed_in->llvm_ptr); // just pass the thing's pointer
                }
                else {
                    // if the thing isn't a reference, evaluate then pass
                    args.push_back(evaluate_expression(statement->children().at(i), builder, scope));
                }
            }

            builder.CreateCall(proc->llvm_function, args, ident->name());
            break;
        }
        case NodeType::if_statement: {
            /*
             * if statements are a little funny in the AST
             * if
             *  condition
             *  statements
             *  optionally: any number of subsequent ifs
             *  optionally: one if default
             *
             * we can always create the if, and we can always create the else
             * then generate the in betweens iteratively, and create jumps as so:
             * if condition jump to block you're in, else jump to next block
             */
            /*auto start = builder.GetInsertBlock();
            auto if_block = llvm::BasicBlock::Create(builder.getContext(), "if", function);
            auto else_block = statement->children().back()->type() == NodeType::if_default ? llvm::BasicBlock::Create(builder.getContext(), "else", function) : nullptr;
            auto end_block = llvm::BasicBlock::Create(builder.getContext(), "fi", function); // if there's a default we don't need an end block

            builder.SetInsertPoint(start); // insert conditional evaluation
            auto cond = evaluate_expression(statement->children().front(), builder, scope);
            builder.CreateCondBr(cond, if_block, else_block ? else_block : end_block); // jump according to if there's a default block or not

            for (const auto &st : statement->children().at(1)->children()) {
                builder.SetInsertPoint(if_block);
                generate_statement(st, builder, scope, function);
            }
            builder.SetInsertPoint(if_block);
            builder.CreateBr(end_block);

            for (auto branch = statement->children().begin() + 2; branch != statement->children().end(); ++branch) {
                builder.SetInsertPoint(else_block);
                generate_statement(*branch, builder, scope, function);
            }

            builder.SetInsertPoint(end_block);*/

            break;
        }
        case NodeType::if_alt: {
            /*auto start = builder.GetInsertBlock();
            auto if_block = llvm::BasicBlock::Create(builder.getContext(), "if", function);
            auto default_block = llvm::BasicBlock::Create(builder.getContext(), "else", function);

            builder.SetInsertPoint(start);
            auto cond = evaluate_expression(statement->children().front(), builder, scope);
            builder.CreateCondBr(cond, if_block, default_block);

            for (const auto &st : statement->children().at(1)->children()) {
                builder.SetInsertPoint(if_block);
                generate_statement(st, builder, scope, function);
            }

            builder.SetInsertPoint(default_block);*/
            break;
        }
        case NodeType::if_default: {
            /*for (const auto &st : statement->children().front()->children()) {
                generate_statement(st, builder, scope, function);
            }*/
            break;
        }
        case NodeType::while_statement: {
            // whiles are a condition followed by a statement sequence
            // code shape taken right out of the slides
            // FIXME shapes are still wonky
            auto start = builder.GetInsertBlock();
            auto while_block = llvm::BasicBlock::Create(builder.getContext(), "while", function);
            auto after_block = llvm::BasicBlock::Create(builder.getContext(), "while_tail", function);

            builder.SetInsertPoint(start);
            auto cond = evaluate_expression(statement->children().front(), builder, scope);
            builder.CreateCondBr(cond, while_block, after_block);

            for (const auto &child : statement->children().at(1)->children()) {
                builder.SetInsertPoint(while_block);
                generate_statement(child, builder, scope, function);
            }
            cond = evaluate_expression(statement->children().front(), builder, scope);
            builder.CreateCondBr(cond, while_block, after_block);
            builder.SetInsertPoint(after_block);
            stmt = after_block;
            break;
        }
        default: {
            break;
        }
    }
    return stmt;
}

llvm::Function* GeneratorTwo::create_func(const std::shared_ptr<Procedure> &procedure, llvm::Module* module, llvm::IRBuilder<> &builder) {
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
    auto *func = llvm::Function::Create(func_type, llvm::GlobalValue::InternalLinkage, procedure->name(), module); // declare function in the module, internally visible
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
        else if (auto proc = dynamic_pointer_cast<Procedure>(symbol); proc) {
            create_func(proc, module, builder);
        }
    }

    // process function statements
    for (const auto &statement : procedure->sseq_node_->children()) {
        generate_statement(statement, builder, *procedure->scope_, func);
    }

    llvm::verifyFunction(*func);

    return func;
}
