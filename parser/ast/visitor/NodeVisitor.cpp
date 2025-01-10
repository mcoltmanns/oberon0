/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#include "NodeVisitor.h"

#include <queue>
#include <stack>
#include <utility>

#include "parser/ast/nodes/IdentNode.h"
#include "parser/ast/nodes/LiteralNode.h"
#include "parser/ast/nodes/OperatorNode.h"
#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Module.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Reference.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"


NodeVisitor::NodeVisitor(std::shared_ptr<Scope> scope, Logger& logger): scope_(std::move(scope)), logger_(logger) {
}

// evaluate a constant expression
long int NodeVisitor::evaluate_const_expression(const std::shared_ptr<Node>& node) {
    switch (node->type()) {
        // literals are easy
        case NodeType::literal: {
            return std::dynamic_pointer_cast<LiteralNode>(node)->value();
        }
        // idents can be looked up as constants
        // if the lookup fails, ident was not known at compile time and cannot be used to initialize a constant
        case NodeType::ident: {
            auto ident = std::dynamic_pointer_cast<IdentNode>(node);
            auto ident_sym = scope_->lookup<Constant>(ident->name());
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

// find the type of a node
std::shared_ptr<Type> NodeVisitor::get_type(const std::shared_ptr<Node> &node) {
    switch (node->type()) {
        // lone identifiers can be looked up
        case NodeType::ident: {
            auto ident_node = std::dynamic_pointer_cast<IdentNode>(node);
            if (ident_node->selector()) { // identifier has a selector?
                if (ident_node->selector()->type() == NodeType::sel_index) {
                    // index selector means array
                    return scope_->lookup<ArrayType>(ident_node->name())->base_type(); // so return that array's base type
                }
                // else must be a field
                auto field_name_node = std::dynamic_pointer_cast<IdentNode>(ident_node->selector()->children().front()); // get the field name
                return scope_->lookup<RecordType>(ident_node->name())->fields().at(field_name_node->name()); // return the type of that field name
            }
            // constants are always ints
            if (auto cons = scope_->lookup<Constant>(ident_node->name())) return scope_->lookup<Type>("INTEGER");
            // vars have their type attached
            if (auto var = scope_->lookup<Variable>(ident_node->name())) return var->type();
            // so do refs
            if (auto ref = scope_->lookup<Reference>(ident_node->name())) return scope_->lookup<Type>(ref->referenced_type_name());
            // nothing else is admissible as an expression type
            logger_.error(node->pos(), "Couldn't determine expression type");
            return nullptr;
        }
        case NodeType::literal: { // all literals are integers
            return scope_->lookup<Type>("INTEGER");
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
            return scope_->lookup<Type>("INTEGER"); // otherwise just integer
        }
        default: {
            logger_.error(node->pos(), "Couldn't determine expression type");
            return nullptr;
        }
    }
}

