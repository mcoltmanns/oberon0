/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#include "NodeVisitor.h"


NodeVisitor::NodeVisitor(std::shared_ptr<Scope> scope, Logger& logger): scope_(std::move(scope)), logger_(logger) {
}


void NodeVisitor::visit(const std::shared_ptr<Node> &node) {
    node->print(cout);
}

