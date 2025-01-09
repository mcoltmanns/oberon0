//
// Created by moltmanns on 12/21/24.
//

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "Symbol.h"
#include "parser/ast/nodes/Node.h"
#include "scoper/Scope.h"

class Procedure final : public Symbol {
private:
    std::shared_ptr<Node> sseq_node_; // which statement sequence does this procedure execute?
    std::shared_ptr<Scope> scope_;

public:
    Procedure(std::string name, const FilePos &pos, std::shared_ptr<Node> sseq_node, std::shared_ptr<Scope> procedure_scope);

    void print(std::ostream &s, int tabs) override;
};



#endif //PROCEDURE_H
