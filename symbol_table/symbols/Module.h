//
// Created by moltmanns on 12/24/24.
//

#ifndef MODULE_H
#define MODULE_H

#include "Symbol.h"


class Module final : public Symbol {
public:
    Module(std::string name, FilePos pos, int offset) : Symbol(std::move(name), std::move(pos), offset) {}

    void print(std::ostream &s) override;
};



#endif //MODULE_H
