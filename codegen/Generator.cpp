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
                break;
            }
            case VARIABLE: {
                auto variable = std::dynamic_pointer_cast<Variable>(symbol);
                auto type = variable->type()->llvm_type;
                auto name = variable->name();
                // variables declared at the module level have immutable runtime addresses, and get to be global variables
                auto var = new llvm::GlobalVariable(*ll_module, type, false, llvm::GlobalValue::InternalLinkage, llvm::Constant::getNullValue(type), name);
                align_global(var, &layout, &align);
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
                for (const auto& param : procedure->params_) {
                    const auto ptype = procedure->scope_->lookup_by_name<Type>(param.second);
                    args.push_back(ptype->llvm_type);
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

    // define module entry
    auto main_callee = ll_module->getOrInsertFunction("entry", builder.getVoidTy());
    auto main_func = llvm::cast<llvm::Function>(main_callee.getCallee());
    auto entry = llvm::BasicBlock::Create(builder.getContext(), "entry", main_func);
    builder.SetInsertPoint(entry);

    // process the module's statements
    for (const auto& statement : module_symbol->sseq_node->children()) {
        gen_statement(statement, builder, *ll_module, *module_symbol->scope_);
    }

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
            auto place = ll_mod.getGlobalVariable(ident->name(), true);
            builder.CreateStore(expr_res, place);
            break;
        }
        case NodeType::proc_call: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(n->children().front());
            std::vector<llvm::Value*> args;
            for (long unsigned int i = 1; i < n->children().size(); i++) {
                args.push_back(eval_expr(n->children().at(i), builder, ll_mod, scope)); // probably need to somehow check pass by val/pass by ref here
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

llvm::Value* Generator::eval_expr(const std::shared_ptr<Node> &n, llvm::IRBuilder<> &builder, llvm::Module &ll_mod, Scope& scope) const {
    scope.symtbl_size();
    switch (n->type()) {
        case NodeType::literal: { // for literals, just return their value
            auto lit = std::dynamic_pointer_cast<LiteralNode>(n);
            return builder.getInt32(lit->value());
        }
        case NodeType::ident: {
            // for identifiers, look up the pointer to the global value and load that value
            //TODO: selectors!
            auto ident = std::dynamic_pointer_cast<IdentNode>(n);
            auto val = ll_mod.getNamedValue(ident->name());
            return builder.CreateLoad(val->getValueType(), val);
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

