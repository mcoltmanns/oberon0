//
// Created by moltmanns on 12/21/24.
//

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "Symbol.h"
#include "parser/ast/Node.h"

class Procedure final : public Symbol {
private:
    int calc_ar_size();
    Node* procedure_node_;

public:
    Procedure(const std::string &name, const FilePos &pos, Node *procedure_node);

    void print(std::ostream& s) override;
};



#endif //PROCEDURE_H
