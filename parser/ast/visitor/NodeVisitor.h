/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#ifndef OBERON0C_NODEVISITOR_H
#define OBERON0C_NODEVISITOR_H

#include <utility>

#include "parser/ast/nodes/Node.h"
#include "scoper/Scope.h"
#include "util/Logger.h"


class NodeVisitor {
protected:
    std::shared_ptr<Scope> scope_;
    Logger& logger_;

public:
    explicit NodeVisitor(std::shared_ptr<Scope> scope, Logger &logger);

    virtual ~NodeVisitor() noexcept;

    long int evaluate_const_expression(const std::shared_ptr<Node> &exp_node);

    virtual void visit(std::shared_ptr<Node> node) { logger_.warning(node->pos().fileName, "Visiting node"); }

    std::shared_ptr<Type> get_type(const std::shared_ptr<Node> &node);
};

class Scoper final : public NodeVisitor {
public:
    explicit Scoper(std::shared_ptr<Scope> scope, Logger& logger) : NodeVisitor(scope, logger) {}
    ~Scoper() noexcept override;

    void visit(std::shared_ptr<Node> node) override;
};

/*
class Blocker final : public NodeVisitor {
public:
    explicit Blocker(std::shared_ptr<Scope> scope, Logger& logger) : NodeVisitor(scope, logger) {}
    ~Blocker() noexcept override;

    void visit(std::shared_ptr<Node> node) override;
};
*/

#endif //OBERON0C_NODEVISITOR_H
