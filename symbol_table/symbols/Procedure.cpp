//
// Created by moltmanns on 12/21/24.
//

#include "Procedure.h"

Procedure::Procedure(const std::string &name, const FilePos &pos, const SymbolTable* parent_scope, const Node& procedure_node): Symbol(name, pos),
    parent_scope_(parent_scope), params_(parent_scope->logger_), locals_(parent_scope->logger_) {
    // fill maps with requisite info
    // procedure node's chidren are: identifier, parameters
    printf("%lu", procedure_node.children().size());
}

void Procedure::print(std::ostream& s) {
    s << "Procedure " << name_ << " declared at " << pos_.fileName << ":" << pos_.lineNo << ":" << pos_.charNo;
}
