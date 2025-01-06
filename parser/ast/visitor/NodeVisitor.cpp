/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#include "NodeVisitor.h"


NodeVisitor::NodeVisitor(const std::shared_ptr<Node> &root) {
    history_.push(root);
}

NodeVisitor::~NodeVisitor() noexcept = default;

std::shared_ptr<Node> NodeVisitor::current() {
    return history_.top();
}

std::shared_ptr<Node> NodeVisitor::next(unsigned long int i) {
    auto top = history_.top();
    history_.pop();
    history_.push(top->children().at(i));
    return top;
}


