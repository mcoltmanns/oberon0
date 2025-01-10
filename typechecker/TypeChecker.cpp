//
// Created by moltmanns on 1/10/25.
//

#include "TypeChecker.h"

#include <parser/ast/nodes/IdentNode.h>

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
        default:;
    }
}
