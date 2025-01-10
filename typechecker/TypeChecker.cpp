//
// Created by moltmanns on 1/10/25.
//

#include "TypeChecker.h"

#include <ranges>
#include <parser/ast/nodes/IdentNode.h>

#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Variable.h"

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
