//
// Created by mo on 11/23/24.
//

#include "OperatorNode.h"

std::ostream & operator<<(std::ostream &stream, OperatorType opType) {
    string result;
    switch (opType) {
        case EQ:
            result = "=";
        break;
        case LT:
            result = "<";
        break;
        case GT:
            result = ">";
        break;
        case NEQ:
            result = "#";
        break;
        case LEQ:
            result = "<=";
        break;
        case GEQ:
            result = ">=";
        break;
        case AND:
            result = "AND";
        break;
        case OR:
            result = "OR";
        break;
        case NOT:
            result = "~";
        break;
        case TIMES:
            result = "*";
        break;
        case DIV:
            result = "DIV";
        break;
        case MOD:
            result = "MOD";
        break;
        case PLUS:
            result = "PLUS";
        break;
        case PLUS_UNARY:
            result = "PLUS_UNARY";
        break;
        case MINUS:
            result = "MINUS";
        break;
        case MINUS_UNARY:
            result = "MINUS_UNARY";
        break;
        default:
            result = "UNKNOWN";
    }
    return stream << result;
}

OperatorType OperatorNode::operation() const {
    return operation_;
}

void OperatorNode::print(std::ostream &stream, long unsigned int tabs) const {
    stream << string(tabs, '\t') << "OPERATOR at " << pos_.lineNo << ":" << pos_.charNo << " is " << operation_ << std::endl;
}
