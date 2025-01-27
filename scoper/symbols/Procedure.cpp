//
// Created by moltmanns on 12/21/24.
//

#include "Procedure.h"

#include <utility>

Procedure::Procedure(std::string name, const FilePos &pos, std::shared_ptr<Node> sseq_node, std::shared_ptr<Scope> procedure_scope): Symbol(std::move(name), pos, 1), sseq_node_(std::move(sseq_node)), scope_(std::move(procedure_scope)) {
    kind_ = PROCEDURE;
}

void Procedure::print(std::ostream& s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "PROCEDURE " << name_ << " with AR SIZE " << size_ << " and offset " << offset_ << " declared at " << declared_at_.fileName << ":" << declared_at_.lineNo << ":" << declared_at_.charNo << std::endl;
    scope_->print(s, tabs + 1);
}
