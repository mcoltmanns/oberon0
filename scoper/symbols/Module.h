//
// Created by moltmanns on 12/24/24.
//

#ifndef MODULE_H
#define MODULE_H

#include "Symbol.h"
#include "parser/ast/nodes/Node.h"
#include "scoper/Scope.h"

class Module final : public Symbol {
private:
    std::shared_ptr<Node> sseq_node; // which statement sequence does this module execute?
    std::shared_ptr<Scope> scope_;

public:
    Module(std::string name, const FilePos& pos, std::shared_ptr<Node> sseq_node, std::shared_ptr<Scope> module_scope);

    void print(std::ostream &s, int tabs) override;
};



#endif //MODULE_H
