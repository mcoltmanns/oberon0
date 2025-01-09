//
// Created by moltmanns on 12/21/24.
//

#ifndef CONSTRUCTEDTYPES_H
#define CONSTRUCTEDTYPES_H
#include <unordered_map>
#include <utility>

#include "Type.h"

class RecordType : public Type {
private:
    std::unordered_map<std::string, std::shared_ptr<Type>> fields_;

public:
    RecordType(std::string name, FilePos pos, const int size) : Type(std::move(name), std::move(pos), size) {}
    ~RecordType() override;

    std::unordered_map<std::string, std::shared_ptr<Type>>& fields() { return fields_; }
};

class ArrayType : public Type {
private:
    int length_; // how many things are in this array. NOT the same as size!
    std::shared_ptr<Type> base_type_;

public:
    ArrayType(std::string name, int length, std::shared_ptr<Type> base_type, FilePos pos, int size) : Type(name, pos, size), length_(length), base_type_(std::move(base_type)) {}
    virtual ~ArrayType();

    int length() const { return length_; }
    std::shared_ptr<Type> base_type() const { return base_type_; }
};

#endif //CONSTRUCTEDTYPES_H
