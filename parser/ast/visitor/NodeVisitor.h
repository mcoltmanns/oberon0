/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#ifndef OBERON0C_NODEVISITOR_H
#define OBERON0C_NODEVISITOR_H

#include "parser/ast/Node.h"
#include "symbol_table/SymbolTable.h"
#include "util/Logger.h"


class NodeVisitor {
protected:
    Logger& logger_;

public:
    explicit NodeVisitor(Logger &logger);

    virtual ~NodeVisitor() noexcept;

    long int evaluate_const_expression(const std::shared_ptr<Node> &exp_node);

    virtual void visit(Node* node) { logger_.warning(node->pos().fileName, "Visiting node"); }
};

class DecNodeVisitor final : public NodeVisitor {
private:
    SymbolTable& scope_;

public:
    explicit DecNodeVisitor(SymbolTable& scope, Logger& logger) : NodeVisitor(logger), scope_(scope) {}

    ~DecNodeVisitor() noexcept override;

    void visit(Node* node) override;
};


#endif //OBERON0C_NODEVISITOR_H
