//
// Created by mo on 11/23/24.
//

#include "IdentNode.h"

std::string IdentNode::name() {
    return name_;
}

void IdentNode::print(std::ostream &stream, long unsigned int tabs) const {
    stream << string(tabs, '\t') << "IDENTIFIER at " << pos_.lineNo << ":" << pos_.charNo << " has name " << name_;
    if (selector_block_) {
        stream << " and selectors:" << std::endl;
        selector_block_->print(stream, tabs + 1);
        stream << string(tabs, '\t') << "end of IDENTIFIER " << name_;
    }
    stream << std::endl;
}
