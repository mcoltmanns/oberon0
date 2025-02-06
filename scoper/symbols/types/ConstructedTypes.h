//
// Created by moltmanns on 12/21/24.
//

#ifndef CONSTRUCTEDTYPES_H
#define CONSTRUCTEDTYPES_H
#include <unordered_map>
#include <utility>

#include "Type.h"

class RecordType final : public Type {
private:
    std::vector<std::pair<std::string, std::shared_ptr<Type>>> fields_; // vector of fields - their names and their types

public:
    RecordType(std::string name, FilePos pos) : Type(std::move(name), std::move(pos)) {
    }
    ~RecordType() override;

    std::vector<std::pair<std::string, std::shared_ptr<Type>>> &fields() { return fields_; }
    std::shared_ptr<Type> get_field_type_by_name(const std::string &name);
    int get_field_index_by_name(const std::string &name);
};

class ArrayType final : public Type {
private:
    int length_; // how many things are in this array. NOT the same as size!
    std::shared_ptr<Type> base_type_;

public:
    ArrayType(const std::string& name, const int length, std::shared_ptr<Type> base_type, const FilePos &pos) : Type(name, pos), length_(length), base_type_(std::move(base_type)) {
    }

    ~ArrayType() override;

    [[nodiscard]] int length() const { return length_; }
    [[nodiscard]] std::shared_ptr<Type> base_type() const { return base_type_; }
};

// for things like TYPE DERIVED = INTEGER
// not sure why you'd do this but it's allowed
class DerivedType final : public Type {
private:
    std::shared_ptr<Type> base_type_;

public:
    DerivedType(const std::string& name, const std::shared_ptr<Type> &base_type, const FilePos& pos) : Type(name, pos) {
        base_type_ = base_type;
    }

    ~DerivedType() override;

    [[nodiscard]] std::shared_ptr<Type> base_type() const { return base_type_; }
};

#endif //CONSTRUCTEDTYPES_H
