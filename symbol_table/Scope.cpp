//
// Created by moltmanns on 12/24/24.
//

#include "Scope.h"

#include <queue>

#include "symbols/Constant.h"
#include "symbols/Module.h"
#include "symbols/Procedure.h"
#include "symbols/types/BaseTypes.h"

// return shared ptr to symbol in table
// return nullptr if symbol not present
std::shared_ptr<Symbol> Scope::lookup(const std::string &name) {
    // if we looked up a base type, return a pointer to the static base type
    if (name == "INTEGER") return std::make_shared<Type>(BASIC_TYPE_INT);
    if (name == "STRING") return std::make_shared<Type>(BASIC_TYPE_STRING);
    if (name == "BOOLEAN") return std::make_shared<Type>(BASIC_TYPE_BOOL);

    return find_by_name(name);
}

// return shared ptr to symbol in table
// recurses upwards into all visible scopes
std::shared_ptr<Symbol> Scope::find_by_name(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return entry;
    }
    if (outer_) return outer_->find_by_name(name);
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

void Scope::print(std::ostream &s) {
    for(const auto& entry : table_) {
        entry->print(s, 0);
    }
    s << std::endl;
}

void Scope::print(std::ostream &s, int tabs) {
    for (const auto& entry : table_) {
        for (int i = 0; i < tabs; i++) s << "\t";
        entry->print(s, tabs + 1);
    }
    s << std::endl;
}

int Scope::get_next_offset() const {
    return current_offset_;
}

// does not check typing
void Scope::add(const std::shared_ptr<Symbol>& sym) {
    table_.push_back(sym);
    sym->offset_ = current_offset_; // set the offset (second part of static coord)
    current_offset_ += sym->size_;
}

int Scope::symtbl_size() const {
    int size = 0;
    for (const auto& entry : table_) {
        size += entry->size_;
    }
    return size;
}

