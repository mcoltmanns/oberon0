//
// Created by moltmanns on 12/24/24.
//

#ifndef MODULE_H
#define MODULE_H

#include "Symbol.h"

#define MODULE_SIZE 1

// still a little unsure how to handle modules.
// right now i'm thinking all they are are scopes with code, so are they even necessary to have in the symbol table?
// maybe it's best to just rename symbolTable to Scope and treat outermost scopes as modules
// or - rename SymbolTable to Module and have module class contain its outermost scope (top-level symbol table), and the position of its executable code
// going to sleep on it! TODO
class Module final : public Symbol {
public:
    Module(std::string name, FilePos pos) : Symbol(std::move(name), std::move(pos), MODULE_SIZE) {}

    void print(std::ostream &s) override;
};



#endif //MODULE_H
