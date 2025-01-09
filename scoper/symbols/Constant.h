//
// Created by moltmanns on 12/21/24.
//

#ifndef CONSTANT_H
#define CONSTANT_H

#include "Symbol.h"

#define CONSTANT_SIZE 0 // constants take up no space in the AR because they are eval'd at compile time

class Constant final : public Symbol {
private:
    long value_;

public:
    Constant(std::string name, const long value, FilePos pos) : Symbol(std::move(name), std::move(pos), CONSTANT_SIZE), value_(value) {}

    [[nodiscard]] long getValue() const { return value_; }
    void print(std::ostream &s, int tabs) override;
};


#endif //CONSTANT_H
