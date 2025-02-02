//
// Created by moltmanns on 12/24/24.
//

#ifndef SYMTABLEGENERATOR_H
#define SYMTABLEGENERATOR_H
#include <utility>
#include <vector>

#include "symbols/Symbol.h"
#include "util/Logger.h"


class Scope {
private:
    int current_offset_ = 0;

public:
    Logger& logger_;
    std::shared_ptr<Scope> outer_;
    std::vector<std::shared_ptr<Symbol>> table_; // many people need to see the symbols, hence shared
    std::string name_;

    explicit Scope(Logger& logger, std::string name) : logger_(logger), name_(std::move(name)) {
        outer_ = nullptr;
    }

    explicit Scope(Logger& logger, std::shared_ptr<Scope> outer, std::string name) : logger_(logger), outer_(std::move(outer)), name_(std::move(name)) {
    }

    ~Scope() = default;

    template<class T>
    std::shared_ptr<T> lookup_by_name(const std::string &name); // pretty proud of this one!
    std::shared_ptr<Symbol> lookup_by_index(int index); // not recursive

    void print(std::ostream& s);
    void print(std::ostream &s, int tabs);

    void add(const std::shared_ptr<Symbol>& sym);
};

#endif //SYMTABLEGENERATOR_H
