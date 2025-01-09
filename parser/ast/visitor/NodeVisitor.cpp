/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#include "NodeVisitor.h"

#include <boost/convert/parameters.hpp>

#include "parser/ast/nodes/IdentNode.h"
#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Module.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Reference.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"


NodeVisitor::NodeVisitor(Logger& logger): logger_(logger) {
}

NodeVisitor::~NodeVisitor() noexcept = default;

long int NodeVisitor::evaluate_const_expression(const std::shared_ptr<Node>& exp_node) {
    // parens are already take care of
    // every expression node will have in its top level only operators of equal precedence
    // read the next child
    // if it's an expression, literal, or identifier, lookup/evaluate if needed and add to output queue
    // if it's an operator, add to operator stack (given expression contains operators of equal precedence)
    // when no more children:
    // pop from operator stack to back of output queue until operator stack empty
    // when operator stack empty:
    // pop front of output queue into t1, and while not empty:
    //  look at front
    //      if operator, pop, apply to t1 if possible and put t1 back at front of queue
    //      if number:
    //          pop front into t2
    //          look at front
    //          if operator, pop, apply to t1 and t2 and put result back at front of queue
    //          else throw error
    // return t1
    return static_cast<long int>(exp_node->children().size());
}


DecNodeVisitor::~DecNodeVisitor() noexcept {
}

void DecNodeVisitor::visit(std::shared_ptr<Node> node) {
    IdentNode* name_node;
    switch (node->type()) {
        case NodeType::dec_const: { // constant declaration
            name_node = dynamic_cast<IdentNode *>(node->children().front().get());
            auto exp_node = node->children().at(1);
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
                    auto base_type = std::dynamic_pointer_cast<Type>(scope_->lookup(base_type_name));
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
                        if (!scope_->lookup(field_type_node->name())) {
                            logger_.error(node->pos(), "Unknown type: \"" + field_type_node->name() + "\"");
                            break;
                        }
                        const std::shared_ptr<Type> field_type = std::dynamic_pointer_cast<Type>(scope_->lookup(field_type_node->name())); // TODO: refactor all smart pointer casts to use this! do not create new pointers from dynamic casts!
                        for (const auto& ident_node : field_list_node->children().front()->children()) {
                            sym.size_ += field_type->size_; // record size is just the sum of the sizes of its fields
                            sym.fields().emplace(dynamic_cast<IdentNode *>(ident_node.get())->name(), field_type);
                        }
                    }
                    scope_->add(std::make_shared<RecordType>(sym));
                    break;
                }
                case NodeType::type_raw: {
                    auto sym = Type(name_node->name(), node->pos(), scope_->get_next_offset());
                    scope_->add(std::make_shared<Type>(sym));
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
            auto type_name_node = node->children().back()->children().front(); // second node's the type all those identifiers will have
            for(const auto& id : ident_list_node->children()) {
                auto var_sym = Variable(dynamic_cast<IdentNode *>(id.get())->name(), dynamic_cast<IdentNode*>(type_name_node.get())->name(), id->pos());
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
                    if (!proc_scope->lookup(type_name)) {
                        logger_.error(param_list->pos(), "Unknown type: \"" + type_name + "\"");
                    }
                    else {
                        switch (param_list->type()) {
                            case NodeType::fp_copy: {
                                // add the parameters as variables declared in the function to the procedure scope
                                for (const auto& param_ident_node : param_list->children().front()->children()) {
                                    auto ident = dynamic_cast<IdentNode*>(param_ident_node.get());
                                    auto sym = std::make_shared<Variable>(ident->name(), type_name, ident->pos()); // variable named foo of type bar, declared at pos
                                    proc_scope->add(sym);
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
            auto nv = DecNodeVisitor(proc_scope, logger_); // operating in the procedure scope, so we need a new visitor
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
            auto nv = DecNodeVisitor(mod_scope, logger_); // init visitor to build module scope
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

UseNodeVisitor::~UseNodeVisitor() noexcept {
}

void UseNodeVisitor::visit(std::shared_ptr<Node> node) {
    NodeVisitor::visit(node);
}

