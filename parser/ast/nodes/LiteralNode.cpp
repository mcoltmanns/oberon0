//
// Created by mo on 11/23/24.
//

#include "LiteralNode.h"

void LiteralNode::print(std::ostream &stream, long unsigned int tabs) const {
    stream << string(tabs, '\t') << "LITERAL at " << pos_.lineNo << ":" << pos_.charNo << " has value: " << value_ << std::endl;
}
