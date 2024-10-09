/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#ifndef OBERON0C_NODEVISITOR_H
#define OBERON0C_NODEVISITOR_H


class NodeVisitor {

public:
    explicit NodeVisitor() = default;
    virtual ~NodeVisitor() noexcept;

};


#endif //OBERON0C_NODEVISITOR_H
