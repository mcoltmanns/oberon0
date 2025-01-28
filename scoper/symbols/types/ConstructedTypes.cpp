//
// Created by moltmanns on 12/21/24.
//

#include "ConstructedTypes.h"

ArrayType::~ArrayType() {

}

DerivedType::~DerivedType() {

}

RecordType::~RecordType() {

}

std::shared_ptr<Type> RecordType::get_field_type_by_name(const std::string &name) {
    for (const auto &[f_name, f_type] : fields_) {
        if (f_name == name) {
            return f_type;
        }
    }
    return nullptr;
}

int RecordType::get_field_index_by_name(const std::string &name) {
    int i = 0;
    for (const auto &[f_name, f_type] : fields_) {
        if (f_name == name) {
            return i;
        }
        i++;
    }
    return -1;
}
