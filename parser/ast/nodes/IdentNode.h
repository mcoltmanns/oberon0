//
// Created by mo on 11/23/24.
//

#ifndef IDENTNODE_H
#define IDENTNODE_H
#include "Node.h"


class IdentNode final : public Node {
private:
    std::string name_;
    std::shared_ptr<Node> selector_ = nullptr;

public:
    IdentNode(const std::string &name, const FilePos &pos) : Node(NodeType::ident, pos), name_(name) {};

    std::string name();

    std::shared_ptr<Node> selector() { return selector_; };
    void set_selector(const std::shared_ptr<Node> &selector) { selector_ = selector; }

    void print(std::ostream &stream, long unsigned int tabs = 0) const override;
};



#endif //IDENTNODE_H
