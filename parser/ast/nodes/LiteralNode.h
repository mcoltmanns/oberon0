//
// Created by mo on 11/23/24.
//

#ifndef LITERALNODE_H
#define LITERALNODE_H
#include "Node.h"


class LiteralNode final : public Node {
private:
    long value_;
    bool boolean_;

public:
    LiteralNode(const long value, const bool boolean, const FilePos &pos) : Node(NodeType::literal, pos), value_(value), boolean_(boolean) {};

    [[nodiscard]] long value() const { return value_; };
    [[nodiscard]] bool is_bool() const { return boolean_; }

    void print(std::ostream &stream, long unsigned int tabs = 0) const override;
};



#endif //LITERALNODE_H
