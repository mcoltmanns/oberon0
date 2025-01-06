//
// Created by moltmanns on 12/21/24.
//

#ifndef TYPE_H
#define TYPE_H
#include "symbol_table/symbols/Symbol.h"


class Type : public Symbol {
public:
    Type(std::string name, FilePos pos) : Symbol(name, pos) {}

    explicit Type(const char * str) : Symbol(str) {
        pos_ = {"BASETYPE", 0, 0, 0 };
    };
};



#endif //TYPE_H
