//
// Created by moltmanns on 12/24/24.
//

#include "SymbolTable.h"

#include <queue>

#include "symbols/Constant.h"
#include "symbols/Procedure.h"
#include "symbols/types/BaseTypes.h"

// return shared ptr to symbol in table
// return nullptr if symbol not present
// casts symbols back down to their actual classes, not just generic Symbol
std::shared_ptr<Symbol> SymbolTable::lookup(const std::string &name) {
    // if we looked up a base type, return a pointer to the static base type
    if (name == "INTEGER") return std::make_shared<Symbol>(BASIC_TYPE_INT);
    if (name == "STRING") return std::make_shared<Symbol>(BASIC_TYPE_STRING);
    if (name == "BOOLEAN") return std::make_shared<Symbol>(BASIC_TYPE_BOOL);

    auto entry = find_by_name(name); // find the entry by name
    if (entry == nullptr) return nullptr; // if it isn't there, null
    return entry;
}

std::shared_ptr<Symbol> SymbolTable::find_by_name(const std::string &name) {
    for (auto entry : table_) {
        if (entry->getName().operator*() == name) return entry;
    }
    return nullptr; // found nothing?
}

/*
 * what does generating the symbol table entail?
 * - go node by node
 * - keep a stack? queue? of nodes to process
 * - look at current node
 * - if it's a symbol declaration, figure out type and add to table
 * - if it's a symbol use, look for that symbol in the table. if lookup fails or is out of scope, log the error
 * - go into children and repeat
 * - enter a new scope when we enter a module or procedure, and leave that scope when we leave the module/procedure
 */

void SymbolTable::print(std::ostream &s) {
    for(const auto& entry : table_) {
        entry->print(s);
        s << std::endl;
    }
}

int SymbolTable::get_next_offset() const {
    return current_offset_;
}

void SymbolTable::add(std::shared_ptr<Symbol> sym) {
    table_.push_back(sym);
    current_offset_ += sym->size_;
}

// get the offset of a symbol in this scope (second part of static coord)
// offsets are just the sums of all sizes of previous symbol table elements
int SymbolTable::get_offset(const string& name) const {
    int offset = 0;
    for (const auto& entry : table_) {
        if (entry->getName().operator*() == name) break;
        offset += entry->size_;
    }
    return offset;
}
