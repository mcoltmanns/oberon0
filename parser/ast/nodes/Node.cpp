/*
 * Base class of all AST nodes used by the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 2/2/18.
 */

#include "Node.h"

Node::~Node() = default;

NodeType Node::type() const {
    return nodeType_;
}

FilePos Node::pos() const {
    return pos_;
}

std::vector<std::shared_ptr<Node>> const &Node::children() const {
    return children_;
}

void Node::append_child(std::unique_ptr<Node> child) {
    children_.push_back(std::move(child));
}

void Node::prepend_child(std::unique_ptr<Node> child) {
    children_.insert(children().begin(), std::move(child));
}

void Node::print(std::ostream &stream, long unsigned int tabs) const {
    stream << string(tabs, '\t') << nodeType_ << "\n";
    for (const auto &child : children_) {
        child->print(stream, tabs + 1);
    }
    stream << string(tabs, '\t') << "end of " << nodeType_ << std::endl;
}

std::ostream& operator<<(std::ostream &stream, const Node &node) {
    node.print(stream);
    return stream;
}

std::ostream& operator<<(std::ostream &stream, const NodeType nodeType) {
    string result;
    switch (nodeType) {
        case NodeType::ident:
            result = "IDENT";
        break;
        case NodeType::literal:
            result = "LITERAL";
        break;
        case NodeType::module:
            result = "MODULE";
        break;
        case NodeType::op:
            result = "OPERATOR";
        break;
        case NodeType::declarations:
            result = "DECLARATIONS";
        break;
        case NodeType::dec_const:
            result = "DEC_CONST";
        break;
        case NodeType::dec_type:
            result = "DEC_TYPE";
        break;
        case NodeType::dec_proc:
            result = "DEC_PROC";
        break;
        case NodeType::dec_var:
            result = "DEC_VAR";
        break;
        case NodeType::formal_parameters:
            result = "FORMAL_PARAMETERS";
        break;
        case NodeType::fp_copy:
            result = "FP_COPY";
        break;
        case NodeType::fp_reference:
            result = "FP_REFERENCE";
        break;
        case NodeType::type_array:
            result = "TYPE_ARRAY";
        break;
        case NodeType::type_record:
            result = "TYPE_RECORD";
        break;
        case NodeType::record_field_list:
            result = "RECORD_FIELD_LIST";
        break;
        case NodeType::type_raw:
            result = "TYPE_RAW";
        break;
        case NodeType::ident_list:
            result = "IDENT_LIST";
        break;
        case NodeType::statement_seq:
            result = "STATEMENT_SEQ";
        break;
        case NodeType::assignment:
            result = "ASSIGNMENT";
        break;
        case NodeType::proc_call:
            result = "PROC_CALL";
        break;
        case NodeType::if_alt:
            result = "IF_ALT";
        break;
        case NodeType::if_default:
            result = "IF_DEFAULT";
        break;
        case NodeType::if_statement:
            result = "IF_STATEMENT";
        break;
        case NodeType::repeat_statement:
            result = "REPEAT_STATEMENT";
        break;
        case NodeType::while_statement:
            result = "WHILE_STATEMENT";
        break;
        case NodeType::expression:
            result = "EXPRESSION";
        break;
        case NodeType::selector_block:
            result = "SELECTOR_BLOCK";
        break;
        case NodeType::sel_field:
            result = "SEL_FIELD";
        break;
        case NodeType::sel_index:
            result = "SEL_INDEX";
        break;
        default:
            result = "UNKNOWN";
        break;
    }
    stream << result;
    return stream;
}
