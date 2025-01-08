//
// Created by moltmanns on 12/21/24.
//

#include "Procedure.h"

Procedure::Procedure(const std::string &name, const FilePos &pos, Node* procedure_node): Symbol(name, pos, calc_ar_size()), procedure_node_(procedure_node) {
}

void Procedure::print(std::ostream& s) {
    s << "PROCEDURE " << name_ << " with AR SIZE " << size_ << " declared at " << pos_.fileName << ":" << pos_.lineNo << ":" << pos_.charNo;
}

int Procedure::calc_ar_size() {
    return 1;
}
