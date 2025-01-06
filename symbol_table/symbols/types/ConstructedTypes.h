//
// Created by moltmanns on 12/21/24.
//

#ifndef CONSTRUCTEDTYPES_H
#define CONSTRUCTEDTYPES_H
#include <unordered_map>

#include "Type.h"

class RecordType : public Type {
private:
    std::unordered_map<std::string, Type&> fields_;

public:
    RecordType(std::string name, FilePos pos, int offset) : Type(name, pos, offset) {}

    std::unordered_map<std::string, Type&>& fields() { return fields_; }
};

class ArrayType : public Type {
private:
    int length_;
    std::string base_type_name_;

public:
    ArrayType(std::string name, int length, std::string base_type_name, FilePos pos, int offset) : Type(name, pos, offset), length_(length), base_type_name_(std::move(base_type_name)) {}

    int length() const { return length_; }
    std::string base_type() const { return base_type_name_; } // FIXME: unsafe pointers
};

#endif //CONSTRUCTEDTYPES_H
