//
// Created by moltmanns on 12/21/24.
//

#ifndef INTEGERTYPE_H
#define INTEGERTYPE_H

#include "Type.h"

// for now we assume all base types take up 1 cell in storage
class IntegerType final : public Type {
public:
    explicit IntegerType() : Type("INTEGER") {
    };

    ~IntegerType() override = default;

    void print(std::ostream &s, int tabs) override;
};

/*class StringType final : public Type {
public:
    explicit StringType() : Type("STRING", 1) {};
    virtual ~StringType();

    void print(std::ostream &s, int tabs) override;
};*/

// 0 is false, nonzero is true
class BooleanType final : public Type {
public:
    explicit BooleanType() : Type("BOOLEAN") {
    };
    ~BooleanType() override = default;

    void print(std::ostream &s, int tabs) override;
};

// global basic types - these are static and should NEVER! be declared anywhere else
inline auto BASIC_TYPE_INT = std::make_shared<IntegerType>();
/*static auto BASIC_TYPE_STRING = StringType();*/
inline auto BASIC_TYPE_BOOL = std::make_shared<BooleanType>();


#endif //INTEGERTYPE_H
