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
    int size_; // how many memory units does this thing take up?

    Symbol(std::string name, FilePos pos, const int size) : name_(std::move(name)), pos_(std::move(pos)), size_(size) {}
    explicit Symbol(std::string name, int size) : name_(std::move(name)), pos_(), size_(size) {}

    virtual ~Symbol();

    [[nodiscard]] std::unique_ptr<string> getName() const;

    [[nodiscard]] std::unique_ptr<FilePos> getPos() const;

    virtual void print(std::ostream& s);
};



#endif //SYMBOL_H
