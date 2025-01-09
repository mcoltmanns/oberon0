//
// Created by moltmanns on 12/24/24.
//

#ifndef SYMTABLEGENERATOR_H
#define SYMTABLEGENERATOR_H
#include <map>
#include <utility>
#include <vector>

#include "symbols/Symbol.h"
#include "symbols/types/BaseTypes.h"
#include "util/Logger.h"


class Scope {
private:
    std::vector<std::shared_ptr<Symbol>> table_; // many people need to see the symbols, hence shared
    int current_offset_ = 0;

public:
    Logger& logger_;
    std::shared_ptr<Scope> outer_;
    std::string name_;

    explicit Scope(Logger& logger, std::string name) : logger_(logger), name_(std::move(name)) {
        outer_ = nullptr;
    }

    explicit Scope(Logger& logger, std::shared_ptr<Scope> outer, std::string name) : logger_(logger), outer_(std::move(outer)), name_(std::move(name)) {
    }

    ~Scope() = default;

    template<class T>
    std::shared_ptr<T> lookup(const std::string &name); // pretty proud of this one!

    void print(std::ostream& s);
    void print(std::ostream &s, int tabs);

    int get_next_offset() const;

    void add(const std::shared_ptr<Symbol>& sym);

    int symtbl_size() const;
};

#endif //SYMTABLEGENERATOR_H
