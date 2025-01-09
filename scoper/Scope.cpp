//
// Created by moltmanns on 12/24/24.
//

#include "Scope.h"

#include <queue>

#include "symbols/Constant.h"
#include "symbols/Module.h"
#include "symbols/Procedure.h"
#include "symbols/Reference.h"
#include "symbols/Variable.h"
#include "symbols/types/BaseTypes.h"
#include "symbols/types/ConstructedTypes.h"


// lookup a symbol by its name
// you must provide the symbol you are expecting!
template<>
std::shared_ptr<Type> Scope::lookup(const std::string &name) {
    if (name == "INTEGER") return std::make_shared<Type>(BASIC_TYPE_INT);
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Type>(entry);
    }
    if (outer_) return outer_->lookup<Type>(name);
    return nullptr;
}
template<>
std::shared_ptr<Constant> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Constant>(entry);
    }
    if (outer_) return outer_->lookup<Constant>(name);
    return nullptr;
}
template<>
std::shared_ptr<Module> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Module>(entry);
    }
    if (outer_) return outer_->lookup<Module>(name);
    return nullptr;
}
template<>
std::shared_ptr<Procedure> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Procedure>(entry);
    }
    if (outer_) return outer_->lookup<Procedure>(name);
    return nullptr;
}
template<>
std::shared_ptr<Reference> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Reference>(entry);
    }
    if (outer_) return outer_->lookup<Reference>(name);
    return nullptr;
}
template<>
std::shared_ptr<Symbol> Scope::lookup(const std::string &name) {
    if (name == "INTEGER") return std::make_shared<Symbol>(BASIC_TYPE_INT);
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Symbol>(entry);
    }
    if (outer_) return outer_->lookup<Symbol>(name);
    return nullptr;
}
template<>
std::shared_ptr<Variable> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Variable>(entry);
    }
    if (outer_) return outer_->lookup<Variable>(name);
    return nullptr;
}
template<>
std::shared_ptr<DerivedType> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<DerivedType>(entry);
    }
    if (outer_) return outer_->lookup<DerivedType>(name);
    return nullptr;
}
template<>
std::shared_ptr<RecordType> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<RecordType>(entry);
    }
    if (outer_) return outer_->lookup<RecordType>(name);
    return nullptr;
}
template<>
std::shared_ptr<ArrayType> Scope::lookup(const std::string &name) {
    for (auto entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<ArrayType>(entry);
    }
    if (outer_) return outer_->lookup<ArrayType>(name);
    return nullptr;
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
// but does check duplicate symbol uses within current scope
// symbols in outer scopes are simply obscured during lookup
void Scope::add(const std::shared_ptr<Symbol>& sym) {
    for (auto entry : table_) {
        if (entry->name() == sym->name()) {
            logger_.error(sym->pos().operator*(), "Name already in use in this scope");
        }
    }
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

