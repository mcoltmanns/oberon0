//
// Created by moltmanns on 12/21/24.
//

#ifndef CONSTANT_H
#define CONSTANT_H

#include <llvm/ADT/APInt.h>

#include "Symbol.h"

class Constant final : public Symbol {
private:
    long value_;

public:
    Constant(std::string name, const long value, FilePos pos) : Symbol(std::move(name), std::move(pos)), value_(value) {
    }

    [[nodiscard]] long value() const { return value_; }
    [[nodiscard]] llvm::APInt toAPInt(const uint64_t width) const { return llvm::APInt(width, static_cast<long unsigned int>(value_), true); };
    void print(std::ostream &s, int tabs) override;
};


#endif //CONSTANT_H
