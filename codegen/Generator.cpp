//
// Created by moltmanns on 1/26/25.
//

#include "Generator.h"

#include <deque>
#include <stack>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Target/TargetMachine.h>
#include <parser/ast/nodes/IdentNode.h>

#include "parser/ast/nodes/LiteralNode.h"
#include "parser/ast/nodes/OperatorNode.h"
#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/PassedParam.h"

void align_global(llvm::GlobalVariable *global, llvm::DataLayout *layout, const llvm::Align *align) {
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

llvm::Module * Generator::gen_module(const Module *module_symbol) const {
    auto ll_module = new llvm::Module(module_symbol->name(), ctx_);
    ll_module->setDataLayout(tm_.createDataLayout());
    ll_module->setTargetTriple(tm_.getTargetTriple().getTriple());
    llvm::IRBuilder<> builder(ctx_);

    BASIC_TYPE_INT->llvm_type = builder.getInt32Ty();
    BASIC_TYPE_BOOL->llvm_type = builder.getInt1Ty();

    auto layout = ll_module->getDataLayout();
    auto align = layout.getStackAlignment();

    // generate declarations (constants, variables, type and procedure signatures)
    for (const auto& symbol : module_symbol->scope_->table_) {
        switch (symbol->kind_) {
            case CONSTANT: {
                auto constant = std::dynamic_pointer_cast<Constant>(symbol);
                // constants always have immutable runtime addresses which makes them global variables
                auto cons = new llvm::GlobalVariable(*ll_module, BASIC_TYPE_INT->llvm_type, true, llvm::GlobalValue::InternalLinkage, llvm::Constant::getIntegerValue(BASIC_TYPE_INT->llvm_type, constant->toAPInt(32)), constant->name());
                align_global(cons, &layout, &align);
                constant->llvm_type = cons->getValueType();
                constant->llvm_ptr = cons;
                break;
            }
            case VARIABLE: {
                auto variable = std::dynamic_pointer_cast<Variable>(symbol);
                auto type = variable->type()->llvm_type;
                auto name = variable->name();
                // variables declared at the module level have immutable runtime addresses, and get to be global variables
                auto var = new llvm::GlobalVariable(*ll_module, type, false, llvm::GlobalValue::InternalLinkage, llvm::Constant::getNullValue(type), name);
                align_global(var, &layout, &align);
                variable->llvm_type = var->getValueType();
                variable->llvm_ptr = var;
                break;
            }
            case DERIVED_TYPE: {
                auto derived = std::dynamic_pointer_cast<DerivedType>(symbol);
                auto base = derived->base_type();
                derived->llvm_type = base->llvm_type;
                break;
            }
            case RECORD_TYPE: {
                auto record = std::dynamic_pointer_cast<RecordType>(symbol);
                std::vector<llvm::Type*> members;
                for (const auto& field : record->fields()) {
                    members.push_back(field.second->llvm_type);
                }
                auto type = llvm::StructType::get(ctx_, members); // we don't pack structs because we're lazy, and there's really no need for normal use
                record->llvm_type = type;
                break;
            }
            case ARRAY_TYPE: {
                auto array = std::dynamic_pointer_cast<ArrayType>(symbol);
                auto base = array->base_type()->llvm_type;
                auto dim = array->length();
                auto type = llvm::ArrayType::get(base, static_cast<unsigned long int>(dim));
                array->llvm_type = type;
                break;
            }
            case PROCEDURE: {
                auto procedure = std::dynamic_pointer_cast<Procedure>(symbol);
                auto name = procedure->name();
                std::vector<llvm::Type*> args;
                int param_index = 0;
                for (const auto& param : procedure->params_) {
                    if (const auto psym = std::dynamic_pointer_cast<PassedParam>(procedure->scope_->lookup_by_index(param_index++)); psym->is_reference()) {
                        args.push_back(builder.getPtrTy()); // if the symbol is a reference, set a pointer in the function signature
                    }
                    else {
                        const auto ptype = procedure->scope_->lookup_by_name<Type>(param.second);
                        args.push_back(ptype->llvm_type); // otherwise set that parameter's type in the signature
                    }
                }
                std::vector<llvm::Type*> locals;
                //for (const auto& local : procedure->scope_.ta)
                procedure->llvm_sig = llvm::FunctionType::get(builder.getVoidTy(), args, false); // oberon procedures have no return type and no varargs
                procedure->llvm_callee = ll_module->getOrInsertFunction(name, procedure->llvm_sig);
                /*for (const auto& statement : procedure->sseq_node_->children()) {
                    gen_statement(statement, builder, *ll_module, *procedure->scope_);
                }*/
                break;
            }
            default: {
                logger_.error(*module_symbol->pos(), "Unexpected symbol during generation");
                return nullptr;
            }
        }
    }

    // define external printf
#ifdef _LLVM_LEGACY
    auto sig = llvm::FunctionType::get(builder.getInt32Ty(), { builder.getInt8PtrTy() }, true);
#else
    auto sig = llvm::FunctionType::get(builder.getInt32Ty(), { builder.getPtrTy() }, true);
#endif
    ll_module->getOrInsertFunction("printf", sig);

    // define module entry
    auto main_callee = ll_module->getOrInsertFunction("main", builder.getVoidTy());
    auto main_func = llvm::cast<llvm::Function>(main_callee.getCallee());
    auto entry = llvm::BasicBlock::Create(builder.getContext(), "main", main_func);
    builder.SetInsertPoint(entry);

    // process the module's statements
    for (const auto& statement : module_symbol->sseq_node->children()) {
        gen_statement(statement, builder, *ll_module, *module_symbol->scope_);
    }

    //auto str = builder.CreateGlobalStringPtr("Test\n");
    //builder.CreateCall(ll_module->getFunction("printf"), str);

    // create the return point for the main function
    builder.CreateRetVoid();
    llvm::verifyFunction(*main_func);
    llvm::verifyModule(*ll_module, &llvm::errs());
    return ll_module;
}

void Generator::gen_statement(const std::shared_ptr<Node> &n, llvm::IRBuilder<> &builder, llvm::Module& ll_mod, Scope& scope) const {
    switch (n->type()) {
        case NodeType::assignment: {
            // assignments are ident, expression
            auto ident = std::dynamic_pointer_cast<IdentNode>(n->children().front());
            auto expr = n->children().back();
            auto expr_res = eval_expr(expr, builder, ll_mod, scope);
            auto dest = get_ident_ptr(ident, builder, ll_mod, scope);
            builder.CreateStore(expr_res, dest.first);
            break;
        }
        case NodeType::proc_call: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(n->children().front());
            auto proc = scope.lookup_by_name<Procedure>(ident->name());
            std::vector<llvm::Value*> args;
            for (long unsigned int i = 1; i < n->children().size(); i++) {
                auto param = std::dynamic_pointer_cast<PassedParam>(proc->scope_->lookup_by_index(i - 1));
                if (param->is_reference()) {
                    auto passed_ident = std::dynamic_pointer_cast<IdentNode>(n->children().at(i));
                    if (!passed_ident) {
                        logger_.error(n->children().at(i)->pos(), "Cannot pass values by reference");
                        return;
                    }
                    args.push_back(get_ident_ptr(passed_ident, builder, ll_mod, scope).first); // if a parameter is a reference, just push back its pointer
                }
                else
                    args.push_back(eval_expr(n->children().at(i), builder, ll_mod, scope)); // otherwise evaluate it and push back the eval'd thing
            }
            auto func = ll_mod.getFunction(ident->name());
            builder.CreateCall(func, args);
            break;
        }
        case NodeType::if_statement: {
            break;
        }
        case NodeType::while_statement: {
            break;
        }
        case NodeType::unknown: {
            break;
        }
        default: {
            logger_.error(n->pos(), "Generation failed");
        }
    }
}

// evaluate and store a given expression node
llvm::Value* Generator::eval_expr(const std::shared_ptr<Node> &n, llvm::IRBuilder<> &builder, llvm::Module &ll_mod, Scope& scope) const {
    switch (n->type()) {
        case NodeType::literal: { // for literals, just return their value
            auto lit = std::dynamic_pointer_cast<LiteralNode>(n);
            if (lit->is_bool()) return builder.getInt1(lit->value());
            return builder.getInt32(lit->value());
        }
        case NodeType::ident: {
            // for identifiers, look up the pointer to the global value and load that value
            auto ident = std::dynamic_pointer_cast<IdentNode>(n);
            auto src = get_ident_ptr(ident, builder, ll_mod, scope);
            return builder.CreateLoad(src.second, src.first);
        }
        default: {
            // for anything else
            // iterate over the children and write instructions accordingly
            // following algorithm inspired by shunting yard, but adapted to the structure of my ast
            std::stack<OperatorNode*> operators;
            std::stack<llvm::Value*> values;
            for (const auto& child : n->children()) {
                if (child) { //FIXME this null check shouldn't be needed. sometimes the parser generates empty expressions and i can't figure out why. but they don't seem to affect correctness so i'm not gonna worry too hard
                    if (child->type() == NodeType::op) {
                        operators.push(dynamic_cast<OperatorNode*>(child.get()));
                    }
                    else {
                        values.push(eval_expr(child, builder, ll_mod, scope));
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
                    auto lhs = values.top();
                    values.pop();
                    auto rhs = values.top();
                    values.pop();
                    values.push(apply_op(lhs, op, rhs, builder));
                }
            }
            //FIXME there is still a lot of bugginess surround unary operators.
            // too bad! a problem for a later date!
            return values.top();
        }
    }
}

// apply an op node to two loaded values
llvm::Value * Generator::apply_op(llvm::Value *lhs, OperatorNode *op, llvm::Value *rhs, llvm::IRBuilder<> &builder) const {
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
            logger_.error(op->pos(), "Unsupported operator");
            return nullptr;
        }
    }
}

// get a given ident node's pointer and pointer type
std::pair<llvm::Value *, llvm::Type *> Generator::get_ident_ptr(const std::shared_ptr<IdentNode> &ident,
                                                                llvm::IRBuilder<> &builder, llvm::Module &ll_mod,
                                                                Scope &scope) const {
    llvm::Value* ptr = nullptr;
    llvm::Type* ptr_type;
    std::shared_ptr<Type> scope_type;
    //SymbolKind ident_kind;
    // lookup the identifier in the scope
    if (auto param = scope.lookup_by_name<PassedParam>(ident->name()); param) {
        //ident_kind = param->kind_;
        ptr = param->llvm_ptr;
        ptr_type = param->llvm_type;
        scope_type = scope.lookup_by_name<Type>(param->type_name());
    }
    else if (auto var = scope.lookup_by_name<Variable>(ident->name())) {
        //ident_kind = var->kind_;
        ptr = var->llvm_ptr;
        ptr_type = var->llvm_type;
        scope_type = var->type();
    }
    else if (auto constant = scope.lookup_by_name<Constant>(ident->name())) {
        //ident_kind = constant->kind_;
        ptr = constant->llvm_ptr;
        ptr_type = constant->llvm_type;
        scope_type = scope.lookup_by_name<Type>("INTEGER");
    }
    else {
        logger_.error(ident->pos(), "Generation failed (unable to find symbol)");
        return std::pair(ptr, ptr_type);
    }
    if (ident->selector_block()) { // if there is a selector block, evaluate it and access identifier's element according to if struct or array
        for (const std::shared_ptr<Node>& selector : ident->selector_block()->children()) {
            // as long as there are still selectors, find out what index in the destination they point to and continue from there
            if (selector->type() == NodeType::sel_index) {
                auto index = eval_expr(selector->children().front(), builder, ll_mod, scope);
                ptr = builder.CreateInBoundsGEP(ptr_type, ptr, { builder.getInt32(0), index });
            }
            else if (selector->type() == NodeType::sel_field) {
                auto rec_type = std::dynamic_pointer_cast<RecordType>(scope_type);
                auto field_ident = std::dynamic_pointer_cast<IdentNode>(selector->children().front());
                auto field_index = static_cast<unsigned int>(rec_type->get_field_index_by_name(field_ident->name()));
                ptr = builder.CreateInBoundsGEP(ptr_type, ptr, { builder.getInt32(0), builder.getInt32(field_index) });
            }
        }
    }
    return std::pair(ptr, ptr_type);
}
