//
// Created by moltmanns on 12/21/24.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include <memory>
#include <string>
#include <utility>

#include "global.h"


class Symbol {
protected:
    std::string name_;
    FilePos pos_;

public:
    Symbol(std::string name, FilePos pos) : name_(std::move(name)), pos_(std::move(pos)) {}
    Symbol(std::string name) : name_(std::move(name)), pos_() {}
    virtual ~Symbol() = default;

    [[nodiscard]] std::unique_ptr<string> getName() const;

    [[nodiscard]] std::unique_ptr<FilePos> getPos() const;

    virtual void print(std::ostream& s);
};



#endif //SYMBOL_H
