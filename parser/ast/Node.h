/*
 * Base class of all AST nodes used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 2/2/18.
 */

#ifndef OBERON0C_AST_H
#define OBERON0C_AST_H


#include <memory>
#include <utility>
#include <vector>

#include "../../util/Logger.h"

enum class NodeType : char {
    op, ident, literal, module, declarations, dec_const, dec_type, dec_var, dec_proc, unknown, formal_parameters,
    fp_reference, fp_copy, type_array, type_record, record_field_list, type_raw, ident_list, statement_seq, assignment,
    proc_call, if_statement, if_alt, if_default, while_statement, repeat_statement, expression, selector_block,
    sel_field, sel_index
};
std::ostream& operator<<(std::ostream &stream, NodeType nodeType);

class NodeVisitor;

class Node {

private:
    std::vector<std::shared_ptr<Node>> children_ {}; // order matters!
    // and this should be private because the only nodes that inherit this class are terminals, which have no children
    // and should be shared because the syntax tree might need to be accessed by multiple things

protected:
    NodeType nodeType_;
    FilePos pos_;

public:
    explicit Node(const NodeType nodeType, FilePos pos) : nodeType_(nodeType), pos_(std::move(pos)) { }
    virtual ~Node();

    [[nodiscard]] NodeType type() const;
    [[nodiscard]] FilePos pos() const;

    [[nodiscard]] std::vector<std::shared_ptr<Node>> const &children() const;
    void addChild(std::unique_ptr<Node> child);

    //virtual void accept(NodeVisitor &visitor) = 0;

    friend std::ostream& operator<<(std::ostream &stream, const Node &node);
    virtual void print(std::ostream &stream, long unsigned int tabs = 0) const;
};

#endif //OBERON0C_AST_H
