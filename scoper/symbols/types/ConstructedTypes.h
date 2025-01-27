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
    std::unordered_map<std::string, std::shared_ptr<Type>> fields_;

public:
    RecordType(std::string name, FilePos pos, const int size) : Type(std::move(name), std::move(pos), size) {
        kind_ = RECORD_TYPE;
    }
    ~RecordType() override;

    std::unordered_map<std::string, std::shared_ptr<Type>>& fields() { return fields_; }
};

class ArrayType final : public Type {
private:
    int length_; // how many things are in this array. NOT the same as size!
    std::shared_ptr<Type> base_type_;

public:
    ArrayType(std::string name, int length, std::shared_ptr<Type> base_type, FilePos pos, int size) : Type(name, pos, size), length_(length), base_type_(std::move(base_type)) {
        kind_ = ARRAY_TYPE;
    }

    ~ArrayType() override;

    int length() const { return length_; }
    std::shared_ptr<Type> base_type() const { return base_type_; }
};

// for things like TYPE DERIVED = INTEGER
// not sure why you'd do this but it's allowed
class DerivedType final : public Type {
private:
    std::shared_ptr<Type> base_type_;

public:
    DerivedType(std::string name, const std::shared_ptr<Type> &base_type, FilePos pos) : Type(std::move(name), std::move(pos), base_type->size_) {
        kind_ = DERIVED_TYPE;
        base_type_ = std::move(base_type);
    }

    ~DerivedType() override;

    std::shared_ptr<Type> base_type() const { return base_type_; }
};

#endif //CONSTRUCTEDTYPES_H
