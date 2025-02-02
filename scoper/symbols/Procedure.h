//
// Created by moltmanns on 12/21/24.
//

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>

#include "Symbol.h"
#include "parser/ast/nodes/Node.h"
#include "scoper/Scope.h"

class Procedure final : public Symbol {
public:
    std::shared_ptr<Node> sseq_node_; // which statement sequence does this procedure execute?
    std::shared_ptr<Scope> scope_; // first n elements of procedure scopes are symbols of the params - find reference/copy info here
    std::vector<std::pair<string, string>> params_; // list of parameter names and their type names
    llvm::FunctionType* llvm_sig;
    llvm::Function* llvm_function;

    Procedure(std::string name, const FilePos &pos, std::shared_ptr<Node> sseq_node, std::shared_ptr<Scope> procedure_scope);

    void print(std::ostream &s, int tabs) override;
};

// global external procedures - will be made available at linkage from c stdlib
// these are static and should NEVER! be declared anywhere else!
// TODO declared here but fully set up elsewhere. pretty awful and spaghetti
inline auto EXT_PROCEDURE_PUTCHAR = std::make_shared<Procedure>("PutChar", FilePos("EXTERN", 0, 0, 0), nullptr, nullptr);

#endif //PROCEDURE_H
