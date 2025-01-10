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
    virtual ~NodeVisitor() noexcept = default;

    virtual void visit(const std::shared_ptr<Node>& node);
};


#endif //OBERON0C_NODEVISITOR_H
