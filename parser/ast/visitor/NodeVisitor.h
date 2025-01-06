/*
 * Node visitor for the abstract syntax tree used by the Oberon-0 compiler.
 *
 * What the hell is this supposed to do anyways?
 * we need something to go through the AST once it's loaded
 * use this thing!
 * get semantic information about the nodes we're in
 * keep a history of where we were so we can backtrack (stack! stack!)
 *
 * Created by Michael Grossniklaus on 12/13/21.
 */

#ifndef OBERON0C_NODEVISITOR_H
#define OBERON0C_NODEVISITOR_H
#include <stack>
#include <unordered_map>

#include "parser/ast/Node.h"


class NodeVisitor {
private:
    std::stack<std::shared_ptr<Node>> history_; // to remember where we've been
    // this should be shared because the node visitor doesn't own the tree it's visitng
    std::unordered_map<std::shared_ptr<Node>, bool> visited_; // to remember which nodes we've visited already

public:
    explicit NodeVisitor(const std::shared_ptr<Node> &root);
    ~NodeVisitor() noexcept;

    // ditto for shared pointers here, nodes in the tree still own themselves
    // don't want to transfer ownership with these, just allow looking at them
    std::shared_ptr<Node> current(); // return the node we're at
    std::shared_ptr<Node> next(unsigned long int i);
    std::shared_ptr<Node> back();
};


#endif //OBERON0C_NODEVISITOR_H
