//
// Created by moltmanns on 12/24/24.
//

#include "Scope.h"

#include <queue>

#include "symbols/Constant.h"
#include "symbols/Module.h"
#include "symbols/Procedure.h"
#include "symbols/PassedParam.h"
#include "symbols/Variable.h"
#include "symbols/types/ConstructedTypes.h"


// lookup a symbol by its name
// you must provide the symbol you are expecting!
template<>
std::shared_ptr<Type> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Type>(entry);
    }
    if (outer_) return outer_->lookup_by_name<Type>(name);
    return nullptr;
}
template<>
std::shared_ptr<Constant> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Constant>(entry);
    }
    if (outer_) return outer_->lookup_by_name<Constant>(name);
    return nullptr;
}
template<>
std::shared_ptr<Module> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Module>(entry);
    }
    if (outer_) return outer_->lookup_by_name<Module>(name);
    return nullptr;
}
template<>
std::shared_ptr<Procedure> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Procedure>(entry);
    }
    if (outer_) return outer_->lookup_by_name<Procedure>(name);
    return nullptr;
}
template<>
std::shared_ptr<PassedParam> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<PassedParam>(entry);
    }
    if (outer_) return outer_->lookup_by_name<PassedParam>(name);
    return nullptr;
}
template<>
std::shared_ptr<Symbol> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Symbol>(entry);
    }
    if (outer_) return outer_->lookup_by_name<Symbol>(name);
    return nullptr;
}
template<>
std::shared_ptr<Variable> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<Variable>(entry);
    }
    if (outer_) return outer_->lookup_by_name<Variable>(name);
    return nullptr;
}
template<>
std::shared_ptr<DerivedType> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<DerivedType>(entry);
    }
    if (outer_) return outer_->lookup_by_name<DerivedType>(name);
    return nullptr;
}
template<>
std::shared_ptr<RecordType> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<RecordType>(entry);
    }
    if (outer_) return outer_->lookup_by_name<RecordType>(name);
    return nullptr;
}
template<>
std::shared_ptr<ArrayType> Scope::lookup_by_name(const std::string &name) {
    for (const auto& entry : table_) {
        if (entry->name() == name) return std::dynamic_pointer_cast<ArrayType>(entry);
    }
    if (outer_) return outer_->lookup_by_name<ArrayType>(name);
    return nullptr;
}

std::shared_ptr<Symbol> Scope::lookup_by_index(const int index) { // not recursive
    if (index < 0 || index >= static_cast<int>(table_.size())) return nullptr;
    return table_.at(static_cast<long unsigned int>(index));
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

// does not check typing
// but does check duplicate symbol uses within current scope
// symbols in outer scopes are simply obscured during lookup
void Scope::add(const std::shared_ptr<Symbol>& sym) {
    for (const auto& entry : table_) {
        if (entry->name() == sym->name()) {
            logger_.error(sym->pos().operator*(), "Name already in use in this scope");
        }
    }
    table_.push_back(sym);
}
