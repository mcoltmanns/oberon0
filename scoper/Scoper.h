//
// Created by moltmanns on 1/10/25.
// Builds and checks scopes according to the AST
//

#ifndef SCOPER_H
#define SCOPER_H
#include "parser/ast/visitor/NodeVisitor.h"


class Scoper final : public NodeVisitor {
public:
    explicit Scoper(std::shared_ptr<Scope> scope, Logger& logger) : NodeVisitor(std::move(scope), logger) {}
    ~Scoper() noexcept override = default;

    void visit(const std::shared_ptr<Node>& node) override;
};

#endif //SCOPER_H
