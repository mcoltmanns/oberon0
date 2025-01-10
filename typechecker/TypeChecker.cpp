//
// Created by moltmanns on 1/10/25.
//

#include "TypeChecker.h"

#include <ranges>
#include <stack>
#include <parser/ast/nodes/IdentNode.h>

#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Reference.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"

// find the type of a node
std::shared_ptr<Type> TypeChecker::get_type(const std::shared_ptr<Node> &node) {
    switch (node->type()) {
        // lone identifiers can be looked up
        case NodeType::ident: {
            auto ident_node = std::dynamic_pointer_cast<IdentNode>(node);
            if (ident_node->selector_block()) { // identifier has a selector block?
                // only vars and refs can have selector blocks, so:
                auto selected_var = scope_->lookup_by_name<Variable>(ident_node->name());
                auto selected_ref = scope_->lookup_by_name<Reference>(ident_node->name());
                if (selected_var) {
                    std::shared_ptr<Type> type = selected_var->type();
                    for (const auto& child : ident_node->selector_block()->children()) {
                        if (auto array = std::dynamic_pointer_cast<ArrayType>(type); array && child->type() == NodeType::sel_index) { // type is an array and we are selecting an index and we are using an integer as the index
                            if (get_type(child->children().front())->name() != "INTEGER") {
                                logger_.error(child->children().front()->pos(), "Array indexes must be of type INTEGER");
                            }
                            type = array->base_type();
                        }
                        else if (auto record = std::dynamic_pointer_cast<RecordType>(type); record && child->type() == NodeType::sel_field) { // type is a record and we are selecting a field
                            type = record->fields().at(std::dynamic_pointer_cast<IdentNode>(child->children().front())->name());
                        }
                        else {
                            logger_.error(child->pos(), "Illegal subscript on non-subscriptable type");
                            return nullptr;
                        }
                    }
                    return type;
                }
                if (selected_ref) {
                    std::shared_ptr<Type> type = scope_->lookup_by_name<Type>(selected_ref->referenced_type_name());
                    for (const auto& child : ident_node->selector_block()->children()) {
                        if (auto array = std::dynamic_pointer_cast<ArrayType>(child); array && child->type() == NodeType::sel_index) { // type is an array and we are selecting an index
                            type = array->base_type();
                        }
                        else if (auto record = std::dynamic_pointer_cast<RecordType>(child); record && child->type() == NodeType::sel_field) { // type is a record and we are selecting a field
                            type = record->fields().at(std::dynamic_pointer_cast<IdentNode>(child->children().front())->name());
                        }
                        else {
                            logger_.error(child->pos(), "Illegal subscript on non-subscriptable type");
                            return nullptr;
                        }
                    }
                    return type;
                }
                logger_.error(ident_node->pos(), "Illegal selector on non-selectable symbol");
                return nullptr;
            }
            // constants are always ints
            if (auto cons = scope_->lookup_by_name<Constant>(ident_node->name())) return scope_->lookup_by_name<Type>("INTEGER");
            // vars have their type attached
            if (auto var = scope_->lookup_by_name<Variable>(ident_node->name())) return var->type();
            // so do refs
            if (auto ref = scope_->lookup_by_name<Reference>(ident_node->name())) return scope_->lookup_by_name<Type>(ref->referenced_type_name());
            // nothing else is admissible as an expression type
            logger_.error(node->pos(), "Couldn't determine expression type");
            return nullptr;
        }
        case NodeType::literal: { // all literals are integers
            return scope_->lookup_by_name<Type>("INTEGER");
        }
        case NodeType::expression: {
            /* as outlined in scoper/symbols/types/typerules:
            * T(expression with operators) = T(first non-operator terminal in expression)
            * BUT, if there is more than one non-operator terminal x in the expression and T(x) != INTEGER, this type is unresolvable
            */
            // do a little dfs
            std::shared_ptr<Node> first, second;
            std::unordered_map<std::shared_ptr<Node>, bool> visited;
            std::stack<std::shared_ptr<Node>> node_stack;
            node_stack.push(node);
            while (!node_stack.empty()) {
                auto n = node_stack.top();
                node_stack.pop();
                if (!visited[n]) {
                    visited[n] = true;
                    if (n->children().size() == 0 && (n->type() == NodeType::literal || n->type() == NodeType::ident)) {
                        if (first) {
                            second = n;
                            break;
                        }
                        first = n;
                    }
                    for (const auto& child : n->children()) {
                        if (!visited[child])
                            node_stack.push(child);
                    }
                }
            }
            if (!first) { // no non-op terminals (should never happen)
                logger_.error(node->pos(), "Couldn't determine expression type");
                return nullptr;
            }
            if (!second) return get_type(first); // only one non-op terminal
            // more than one non-op terminal - must be integer or things are bad
            if (get_type(first)->name() != "INTEGER") {
                logger_.error(node->pos(), "Couldn't determine expression type");
                return nullptr;
            }
            return scope_->lookup_by_name<Type>("INTEGER"); // otherwise just integer
        }
        default: {
            logger_.error(node->pos(), "Couldn't determine expression type");
            return nullptr;
        }
    }
}
void TypeChecker::visit(const std::shared_ptr<Node> &node) {
    switch (node->type()) {
        case NodeType::assignment: {
            // assignments are easy: type of left must match type of right
            auto left = std::dynamic_pointer_cast<IdentNode>(node->children().at(0));
            auto right = node->children().at(node->children().size() - 1);
            auto left_type = get_type(left);
            auto right_type = get_type(right);
            if (!left_type || !right_type) {
                logger_.error(node->pos(), "Could not determine type");
            }
            else if (left_type->name() != right_type->name()) {
                logger_.error(node->pos(), "Cannot assign expression of type \"" + right_type->name() + "\" to symbol of type \"" + left_type->name() + "\"");
            }
            break;
        }
        case NodeType::proc_call: {
            // procedure calls aren't too bad either: types of args passed must match types of formal parameters in declaration
            // the first param_count elements of the procedure's scope are where we'll be passing to
            // in a proc_call node: first is procedure name, following are parameters
            auto proc_name = std::dynamic_pointer_cast<IdentNode>(node->children().at(0))->name();
            auto proc_sym = scope_->lookup_by_name<Procedure>(proc_name);
            if (!proc_sym) {
                logger_.error(node->pos(), "Procedure \"" + proc_name + "\" was not declared in this scope");
                break;
            }
            int param_no = 0;
            for (const auto& child : node->children() | std::views::drop(1)) {
                if (param_no >= static_cast<int>(proc_sym->params_.size())) {
                    logger_.error(child->pos(), "Too many parameters in procedure call");
                    break;
                }
                auto passed_type = get_type(child);
                auto receiving_type = scope_->lookup_by_name<Type>(proc_sym->params_.at(proc_sym->scope_->lookup_by_index(param_no)->name()));
                if (receiving_type && passed_type && passed_type->name() != receiving_type->name()) {
                    logger_.error(child->pos(), "Parameter type mismatch in procedure call (passed \"" + passed_type->name() + "\" but expected \"" + receiving_type->name() + "\")");
                }
                param_no++;
            }
            break;
        }
        case NodeType::if_statement: {
            break;
        }
        case NodeType::repeat_statement: {

        }
        case NodeType::while_statement: {
            break;
        }
        default:;
    }
}
