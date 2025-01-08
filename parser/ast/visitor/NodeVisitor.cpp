/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#include "NodeVisitor.h"

#include "parser/ast/IdentNode.h"
#include "symbol_table/symbols/Constant.h"
#include "symbol_table/symbols/Variable.h"
#include "symbol_table/symbols/types/ConstructedTypes.h"


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

void DecNodeVisitor::visit(Node* node) {
    IdentNode* name_node;
    switch (node->type()) {
        case NodeType::dec_const: { // constant declaration
            name_node = dynamic_cast<IdentNode *>(node->children().front().get());
            auto exp_node = node->children().at(1);
            auto sym = Constant(name_node->name(), evaluate_const_expression(exp_node), node->pos());
            scope_.add(std::make_shared<Constant>(sym));
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
                    if (auto base_type_sym = scope_.lookup(base_type_name); !base_type_sym) {
                        stringstream ss;
                        ss << "Unknown type: \"" << base_type_name << "\"";
                        logger_.error(node->pos(), ss.str());
                    }
                    else {
                        auto sym = ArrayType(name_node->name(), length, base_type_name, node->pos(), scope_.get_next_offset());
                        scope_.add(std::make_shared<ArrayType>(sym));
                    }
                    break;
                }
                case NodeType::type_record: {
                    // record type nodes have any number of record_field_list nodes
                    // record_field_lists consist of a series of identifiers and a type name - all identifiers should have that type name
                    // internally, records are an unordered_map of strings to type references
                    auto sym = RecordType(name_node->name(), node->pos(), scope_.get_next_offset());
                    for (auto field_list_node : type_node->children()) {
                        auto field_type = dynamic_cast<Type*>(scope_.lookup(dynamic_cast<IdentNode *>(field_list_node->children().at(1)->children().at(0).get())->name()).get());
                        for (auto ident_node : field_list_node->children().at(0)->children()) {
                            sym.fields().emplace(dynamic_cast<IdentNode *>(ident_node.get())->name(), *field_type);
                        }
                    }
                    scope_.add(std::make_shared<RecordType>(sym));
                    break;
                }
                case NodeType::type_raw: {
                    auto sym = Type(name_node->name(), node->pos(), scope_.get_next_offset());
                    scope_.add(std::make_shared<Type>(sym));
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
                scope_.add(std::make_shared<Variable>(var_sym));
            }
            break;
        }
        case NodeType::dec_proc: {
            // TODO: implement procedures.
            break;
        }
        default: {
            logger_.error(node->pos(), "INTERNAL: expected declaration node");
        }
    }
}

