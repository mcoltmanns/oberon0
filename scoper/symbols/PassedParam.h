//
// Created by moltmanns on 1/9/25.
//

#ifndef REFERENCE_H
#define REFERENCE_H
#include <utility>

#include "Procedure.h"
#include "Symbol.h"
#include "types/Type.h"

#define REF_SIZE 1

class PassedParam final : public Symbol {
private:
    int index_;
    std::shared_ptr<Type> type_;
    bool is_reference_;
    Procedure* procedure_;

public:
    PassedParam(std::string name, int index,std::shared_ptr<Type> type, const bool is_reference, FilePos pos, Procedure* procedure) : Symbol(std::move(name), std::move(pos), REF_SIZE), index_(index), type_(std::move(type)), is_reference_(is_reference), procedure_(procedure) {
        kind_ = SymbolKind::PASSED_PARAM;
    }

    std::shared_ptr<Type> type() { return type_; }
    [[nodiscard]] Procedure* procedure() const { return procedure_; }
    [[nodiscard]] bool is_reference() const { return is_reference_; }
    [[nodiscard]] int index() const { return index_; }

    void print(std::ostream &s, int tabs) override;
};



#endif //REFERENCE_H
