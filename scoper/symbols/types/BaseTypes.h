//
// Created by moltmanns on 12/21/24.
//

#ifndef INTEGERTYPE_H
#define INTEGERTYPE_H
#include "Type.h"

// for now we assume all base types take up 1 cell in storage
class IntegerType final : public Type {
public:
    explicit IntegerType() : Type("INTEGER", 1) {};
    virtual ~IntegerType();

    void print(std::ostream &s, int tabs) override;
};

class StringType final : public Type {
public:
    explicit StringType() : Type("STRING", 1) {};
    virtual ~StringType();

    void print(std::ostream &s, int tabs) override;
};

class BooleanType final : public Type {
public:
    explicit BooleanType() : Type("BOOLEAN", 1) {};
    virtual ~BooleanType();

    void print(std::ostream &s, int tabs) override;
};

// global basic types - these are static and should NEVER! be declared anywhere else
static auto BASIC_TYPE_INT = IntegerType();
static auto BASIC_TYPE_STRING = StringType();
static auto BASIC_TYPE_BOOL = BooleanType();


#endif //INTEGERTYPE_H
