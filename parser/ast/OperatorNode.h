//
// Created by mo on 11/23/24.
//

#ifndef OPERATORNODE_H
#define OPERATORNODE_H
#include "Node.h"

enum OperatorType : char { EQ, NEQ, LT, LEQ, GT, GEQ, AND, OR, NOT, TIMES, DIV, MOD, PLUS, MINUS };
std::ostream& operator<<(std::ostream &stream, OperatorType opType);

class OperatorNode final : public Node {

private:
    OperatorType operation_;

public:
    OperatorNode(const OperatorType operation, const FilePos &pos) : Node(NodeType::op, pos), operation_(operation) {};

    OperatorType operation() const;

    void print(std::ostream &stream, long unsigned int tabs = 0) const override;
};



#endif //OPERATORNODE_H
