//
// Created by moltmanns on 12/21/24.
//

#ifndef CONSTANT_H
#define CONSTANT_H

#include "Symbol.h"


class Constant final : public Symbol {
private:
    long value_;

public:
    Constant(std::string name, const long value, FilePos pos) : Symbol(std::move(name), std::move(pos)), value_(value) {}

    [[nodiscard]] long getValue() const { return value_; }
    void print(std::ostream &s) override;
};


#endif //CONSTANT_H
