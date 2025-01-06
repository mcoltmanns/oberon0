//
// Created by moltmanns on 12/21/24.
//

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "Symbol.h"
#include "symbol_table/SymbolTable.h"

// procedures should have two symbol tables: one for parameters, and one for local vars
// and a reference to their parent scope/symbol table
class Procedure final : public Symbol {
private:
    const SymbolTable *parent_scope_;
    SymbolTable params_;
    SymbolTable locals_;

public:
    Procedure(const std::string &name, const FilePos &pos, const SymbolTable *parent_scope, const Node& procedure_node);

    void print(std::ostream& s) override;
};



#endif //PROCEDURE_H
