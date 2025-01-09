//
// Created by mo on 11/23/24.
//

#ifndef LITERALNODE_H
#define LITERALNODE_H
#include "Node.h"


class LiteralNode final : public Node {
private:
    long value_;

public:
    LiteralNode(const long value, const FilePos &pos) : Node(NodeType::literal, pos), value_(value) {};

    long value() const;

    void print(std::ostream &stream, long unsigned int tabs = 0) const override;
};



#endif //LITERALNODE_H
