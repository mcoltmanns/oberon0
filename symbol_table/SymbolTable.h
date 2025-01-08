//
// Created by moltmanns on 12/24/24.
//

#ifndef SYMTABLEGENERATOR_H
#define SYMTABLEGENERATOR_H
#include <map>
#include <vector>

#include "symbols/Symbol.h"
#include "symbols/types/BaseTypes.h"
#include "util/Logger.h"


class SymbolTable {
private:
    std::vector<std::shared_ptr<Symbol>> table_; // many people need to see the symbols, hence shared
    int current_offset_ = 0;

public:
    Logger& logger_;
    SymbolTable* outer_;

    explicit SymbolTable(Logger& logger) : logger_(logger) {
        outer_ = nullptr;
    }
    explicit SymbolTable(Logger& logger, SymbolTable* outer) : SymbolTable(logger) {
        outer_ = outer;
    }
    ~SymbolTable() = default;


    std::shared_ptr<Symbol> lookup(const std::string &name);

    std::shared_ptr<Symbol> find_by_name(const std::string &name);

    void print(std::ostream& s);
    int get_next_offset() const;

    void add(std::shared_ptr<Symbol> sym);

    int get_offset(const string &name) const;
};



#endif //SYMTABLEGENERATOR_H
