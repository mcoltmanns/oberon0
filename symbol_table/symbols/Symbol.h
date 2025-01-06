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
    int offset_;

public:
    Symbol(std::string name, FilePos pos, const int offset) : name_(std::move(name)), pos_(std::move(pos)), offset_(offset) {}
    Symbol(std::string name) : name_(std::move(name)), pos_(), offset_() {}
    virtual ~Symbol() = default;

    [[nodiscard]] std::unique_ptr<string> getName() const;

    [[nodiscard]] std::unique_ptr<FilePos> getPos() const;

    virtual void print(std::ostream& s);
};



#endif //SYMBOL_H
