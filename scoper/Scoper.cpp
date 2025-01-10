//
// Created by moltmanns on 1/10/25.
//

#include "Scoper.h"

#include <parser/ast/nodes/IdentNode.h>

#include "parser/ast/nodes/LiteralNode.h"
#include "parser/ast/nodes/OperatorNode.h"
#include "symbols/Constant.h"
#include "symbols/Module.h"
#include "symbols/Procedure.h"
#include "symbols/Reference.h"
#include "symbols/Variable.h"
#include "symbols/types/ConstructedTypes.h"
#include "typechecker/TypeChecker.h"

// evaluate a constant expression
long int Scoper::evaluate_const_expression(const std::shared_ptr<Node>& node) {
    switch (node->type()) {
        // literals are easy
        case NodeType::literal: {
            return std::dynamic_pointer_cast<LiteralNode>(node)->value();
        }
        // idents can be looked up as constants
        // if the lookup fails, ident was not known at compile time and cannot be used to initialize a constant
        case NodeType::ident: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(node);
            auto ident_sym = scope_->lookup_by_name<Constant>(ident->name());
            if (!ident_sym) {
                logger_.error(ident->pos(), "Name \"" + ident->name() + "\" is not a constant and may not be used at constant initialization");
                return 0;
            }
            return ident_sym->value();
        }
        // expressions are a little harder - but not too horrible
        case NodeType::expression: {
            long int res;
            bool had_leading = false;
            // special case for leading sign
            if (node->children().at(0)->type() == NodeType::op) {
                auto lead = std::dynamic_pointer_cast<OperatorNode>(node->children().at(0));
                if (lead->operation() == MINUS) res = -evaluate_const_expression(node->children().at(1));
                else if (lead->operation() == NOT) res = evaluate_const_expression(node->children().at(1)) == 0 ? 1 : 0;
                else res = evaluate_const_expression(node->children().at(1));
                had_leading = true;
            }
            // if there was no leading sign, evaluate the first term
            if (!had_leading) res = evaluate_const_expression(node->children().at(0));
            // if there was a leading sign, the first two children have been eval'd, otherwise only the first has been
            for (long unsigned int i = had_leading ? 2 : 1;  i < node->children().size(); i += 2) { // long unsigned int seems like overkill, but apparently we need it
                auto op = std::dynamic_pointer_cast<OperatorNode>(node->children().at(i)); // first will be the op
                long int term = evaluate_const_expression(node->children().at(i + 1)); // second will be the term - evaluate it
                // given the operator, perform on res
                switch (op->operation()) {
                    // booleans don't really work. for now, the logical ops return 0 (false) or 1 (true) and handle 0 as falsy and non-zero as truthy
                    case EQ: {
                        if (res == term) res = 1;
                        else res = 0;
                        break;
                    }
                    case NEQ: {
                        if (res != term) res = 1;
                        else res = 0;
                        break;
                    }
                    case LT: {
                        if (res < term) res = 1;
                        else res = 0;
                        break;
                    }
                    case GT: {
                        if (res > term) res = 1;
                        else res = 0;
                        break;
                    }
                    case LEQ: {
                        if (res <= term) res = 1;
                        else res = 0;
                        break;
                    }
                    case GEQ: {
                        if (res >= term) res = 1;
                        else res = 0;
                        break;
                    }
                    case AND: {
                        if (res != 0 && term != 0) res = 1;
                        else res = 0;
                        break;
                    }
                    case OR: {
                        if (res != 0 || term != 0) res = 1;
                        else res = 0;
                        break;
                    }
                    // not is a leading operator, and not implemented here
                    case TIMES: {
                        res *= term;
                        break;
                    }
                    case DIV: {
                        res /= term;
                        break;
                    }
                    case MOD: {
                        res %= term;
                        break;
                    }
                    case PLUS: {
                        res += term;
                        break;
                    }
                    case MINUS: {
                        res -= term;
                        break;
                    }
                    default: {
                        logger_.error(op->pos(), "Unsupported operation");
                    }
                }
            }
            return res;
        }
        default: {
            logger_.error(node->pos(), "Unable to evaluate constant");
            return 0;
        }
    }
}

void Scoper::visit(const std::shared_ptr<Node>& node) {
    IdentNode* name_node;
    switch (node->type()) {
        case NodeType::dec_const: { // constant declaration
            name_node = dynamic_cast<IdentNode *>(node->children().front().get());
            auto exp_node = node->children().at(1);
            TypeChecker tc(scope_, logger_);
            auto exp_type = tc.get_type(exp_node);
            if (exp_type) {
                node->print(cout);
                exp_type->print(cout, 0);
            }
            auto sym = Constant(name_node->name(), evaluate_const_expression(exp_node), node->pos());
            scope_->add(std::make_shared<Constant>(sym));
            break;
        }
        case NodeType::dec_type: {
            name_node = dynamic_cast<IdentNode *>(node->children().front().get());
            auto type_node = node->children().at(1);
            // find out the type - dec_types contain identifier, [type_array | type_record | type_raw]
            switch (type_node->type()) {
                case NodeType::type_array: {
                    // expression (length), identifier (base type)
                    // array lengths are known at compile time - any identifiers in these expressions should be constants that have already been declared
                    auto length = evaluate_const_expression(type_node->children().at(0));
                    auto base_type_name = dynamic_cast<IdentNode *>(type_node->children().at(1)->children().at(0).get())->name();
                    auto base_type = scope_->lookup_by_name<Type>(base_type_name);
                    if (!base_type) {
                        stringstream ss;
                        ss << "Unknown type: \"" << base_type_name << "\"";
                        logger_.error(node->pos(), ss.str());
                        break;
                    }
                    auto sym = ArrayType(name_node->name(), length, std::dynamic_pointer_cast<Type>(base_type), node->pos(), length * base_type->size_); // array size is its length * size of its base type
                    scope_->add(std::make_shared<ArrayType>(sym));
                    break;
                }
                case NodeType::type_record: {
                    // record type nodes have any number of record_field_list nodes
                    // record_field_lists consist of a series of identifiers and a type name - all identifiers should have that type name
                    // internally, records are an unordered_map of strings to type references
                    auto sym = RecordType(name_node->name(), node->pos(), 0);
                    for (const auto& field_list_node : type_node->children()) {
                        auto field_type_node = dynamic_cast<IdentNode *>(field_list_node->children().back()->children().front().get());
                        auto field_type = scope_->lookup_by_name<Type>(field_type_node->name());
                        if (!field_type) {
                            logger_.error(node->pos(), "Unknown type: \"" + field_type_node->name() + "\"");
                            break;
                        }
                        for (const auto& ident_node : field_list_node->children().front()->children()) {
                            sym.size_ += field_type->size_; // record size is just the sum of the sizes of its fields
                            sym.fields().emplace(dynamic_cast<IdentNode *>(ident_node.get())->name(), field_type);
                        }
                    }
                    scope_->add(std::make_shared<RecordType>(sym));
                    break;
                }
                case NodeType::type_raw: {
                    auto base_type_name = dynamic_cast<IdentNode*>(type_node->children().front().get())->name();
                    auto base_type = scope_->lookup_by_name<Type>(base_type_name);
                    auto sym = DerivedType(name_node->name(), base_type, node->pos());
                    scope_->add(std::make_shared<DerivedType>(sym));
                    break;
                }
                default: {
                    logger_.error(type_node->pos(), "INTERNAL: expected type node during symbol table construction");
                }
            }
            break;
        }
        case NodeType::dec_var: {
            auto ident_list_node = node->children().front(); // first nlaration is a list of identifiers
            auto type_name_node = dynamic_cast<IdentNode *>(node->children().back()->children().front().get());
            auto var_type = scope_->lookup_by_name<Type>(type_name_node->name());
            if (!var_type) {
                logger_.error(node->pos(), "Unknown type: \"" + type_name_node->name() + "\"");
                break;
            }
            for(const auto& id : ident_list_node->children()) {
                auto var_sym = Variable(dynamic_cast<IdentNode *>(id.get())->name(), var_type, id->pos(), var_type->size_);
                scope_->add(std::make_shared<Variable>(var_sym));
            }
            break;
        }
        case NodeType::dec_proc: {
            auto ident_node = dynamic_cast<IdentNode*>(node->children().front().get()); // first child is the procedure's identifier
            // if the second child is of type FORMAL_PARAMETERS, second child is the procedure's formal params
            // otherwise second child is the procedure's declarations (even declarationless procedures have a declarations node)
            std::shared_ptr<Node> fp_node = nullptr;
            if (node->children().at(1)->type() == NodeType::formal_parameters) {
                fp_node = node->children().at(1);
            }
            std::shared_ptr<Node> decs_node;
            if (fp_node) decs_node = node->children().at(2);
            else decs_node = node->children().at(1);
            // finally, statement sequence node
            std::shared_ptr<Node> sseq_node;
            if (fp_node) sseq_node = node->children().at(3);
            else sseq_node = node->children().at(2);
            // now we have the four parts of a procedure: name, params, declarations, and statements
            // goal is to find out how much space the ar will take
            auto proc_scope = std::make_shared<Scope>(logger_, scope_, ident_node->name()); // create the procedure's scope
            auto proc_sym = Procedure(ident_node->name(), node->pos(), sseq_node, proc_scope); // initialize procedure symbol, adding the statement sequence node
            if (fp_node) { // process the parameters, if they exist
                // remember that parameters are also technically in the scope of the procedure, not the outer (current) scope
                // so we lookup and add to proc_scope, not scope_
                for (const auto& param_list : fp_node->children()) {
                    // within a param list: first node is a list of identifiers, second node is a type_raw containing an identifier name which must be a previously declared type
                    // find out the type name for this set of params
                    auto type_name = dynamic_cast<IdentNode *>(param_list->children().at(1)->children().front().get())->name();
                    auto proc_type = proc_scope->lookup_by_name<Type>(type_name);
                    if (!proc_type) {
                        logger_.error(param_list->pos(), "Unknown type: \"" + type_name + "\"");
                    }
                    else {
                        switch (param_list->type()) {
                            case NodeType::fp_copy: {
                                // add the parameters as variables declared in the function to the procedure scope
                                for (const auto& param_ident_node : param_list->children().front()->children()) {
                                    auto ident = dynamic_cast<IdentNode*>(param_ident_node.get());
                                    auto sym = std::make_shared<Variable>(ident->name(), proc_type, ident->pos(), proc_type->size_); // variable named foo of type bar, declared at pos, copy vars have the same size as their base types
                                    proc_scope->add(sym);
                                    proc_sym.params_.emplace(ident->name(), type_name);
                                }
                                break;
                            }
                            // referenced parameters
                            case NodeType::fp_reference: {
                                // add the parameters as references to a type declared in the function to the procedure scope
                                for (const auto& param_ident_node : param_list->children().front()->children()) {
                                    auto ident = dynamic_cast<IdentNode *>(param_ident_node.get());
                                    auto sym = std::make_shared<Reference>(ident->name(), type_name, ident->pos()); // referenced named foo to a value of type bar, declared at pos
                                    proc_scope->add(sym);
                                    proc_sym.params_.emplace(ident->name(), type_name);
                                }
                                break;
                            }
                            default: {
                                logger_.error(param_list->pos(), "INTERNAL: expected parameter node during procedure AR construction");
                            }
                        }
                    }
                }
            }
            // process declarations
            auto nv = Scoper(proc_scope, logger_); // operating in the procedure scope, so we need a new visitor
            for (const auto& dec : decs_node->children()) nv.visit(dec);
            proc_sym.size_ += proc_scope->symtbl_size(); // add the size of the procedure's symbol table to the procedure size (AR size)
            // add procedure to scope (statements were added at proc_sym init)
            scope_->add(std::make_shared<Procedure>(proc_sym));
            break;
        }
        case NodeType::module: {
            // modules are fairly simple: just make sure the first and last identnodes are the same, then add declarations to scope and link in statement sequence
            auto ident_node = dynamic_cast<IdentNode*>(node->children().front().get());
            auto close_node = dynamic_cast<IdentNode*>(node->children().back().get());
            if (ident_node->name() != close_node->name()) {
                logger_.error(close_node->pos(), "Unknown symbol \"" + close_node->name() + "\"");
                break;
            }
            auto decs_node = node->children().at(1); // find declarations
            auto sseq_node = node->children().at(2); // find statement sequence
            auto mod_scope = std::make_shared<Scope>(logger_, scope_, ident_node->name()); // initialize module scope
            auto nv = Scoper(mod_scope, logger_); // init visitor to build module scope
            for (const auto& dec : decs_node->children()) nv.visit(dec); // add declarations to module scope
            auto mod_sym = std::make_shared<Module>(ident_node->name(), ident_node->pos(), sseq_node, mod_scope); // init module symbol
            scope_->add(mod_sym); // add module symbol to scope
            break;
        }
        default: {
            logger_.error(node->pos(), "INTERNAL: expected declaration node");
        }
    }
}
