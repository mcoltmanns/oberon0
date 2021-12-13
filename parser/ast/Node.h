/*
 * Header file of the base class of all AST nodes used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 2/2/18.
 */

#ifndef OBERON0C_AST_H
#define OBERON0C_AST_H


#include <list>
#include <string>
#include <ostream>
#include <utility>
#include "../../util/Logger.h"

enum class NodeType : char {
    module
};

class NodeVisitor;

class Node {

private:
    NodeType nodeType_;
    FilePos pos_;

public:
    explicit Node(const NodeType nodeType, FilePos pos) : nodeType_(nodeType), pos_(std::move(pos)) { };
    virtual ~Node();

    [[nodiscard]] NodeType getNodeType() const;
    [[nodiscard]] FilePos pos() const;

    virtual void accept(NodeVisitor &visitor) = 0;

    virtual void print(std::ostream &stream) const = 0;
    friend std::ostream& operator<<(std::ostream &stream, const Node &node);

};

#endif //OBERON0C_AST_H