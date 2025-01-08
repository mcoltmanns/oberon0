//
// Created by moltmanns on 12/21/24.
//

#ifndef TYPE_H
#define TYPE_H
#include "symbol_table/symbols/Symbol.h"


class Type : public Symbol {
public:
    Type(std::string name, FilePos pos, int size) : Symbol(name, pos, size) {}
    explicit Type(std::string name, int size);

    virtual ~Type(); // these destructors must be defined and out of line, otherwise clang bitches about the vtable (clang really likes to complain!)

    virtual void print(std::ostream &s) override;
};



#endif //TYPE_H
