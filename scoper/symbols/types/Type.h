//
// Created by moltmanns on 12/21/24.
//

#ifndef TYPE_H
#define TYPE_H
#include <llvm/IR/Type.h>

#include "scoper/symbols/Symbol.h"


class Type : public Symbol {
public:
    Type(std::string name, FilePos pos, int size) : Symbol(name, pos, size) {
        kind_ = SymbolKind::TYPE;
    }
    explicit Type(const std::string &name, int size);

    virtual ~Type(); // these destructors must be defined and out of line, otherwise clang bitches about the vtable (clang really likes to complain!)

    void print(std::ostream &s, int tabs) override;
};



#endif //TYPE_H
