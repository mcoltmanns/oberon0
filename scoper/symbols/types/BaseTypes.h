//
// Created by moltmanns on 12/21/24.
//

#ifndef INTEGERTYPE_H
#define INTEGERTYPE_H

#include "Type.h"

// for now we assume all base types take up 1 cell in storage
class IntegerType final : public Type {
public:
    explicit IntegerType() : Type("INTEGER", 1) {
        kind_ = BASE_INT;
    };
    virtual ~IntegerType() = default;

    void print(std::ostream &s, int tabs) override;
};

/*class StringType final : public Type {
public:
    explicit StringType() : Type("STRING", 1) {};
    virtual ~StringType();

    void print(std::ostream &s, int tabs) override;
};*/

// booleans are sort of half-implemented right now. they can be declared, but internally I'm just handling everything as an integer. so you can assign ints to bools and v.v
// you can also do logic ops on ints and math ops on bools
// 0 is false, nonzero is true
class BooleanType final : public Type {
public:
    explicit BooleanType() : Type("BOOLEAN", 1) {
        kind_ = BASE_BOOL;
    };
    virtual ~BooleanType() = default;

    void print(std::ostream &s, int tabs) override;
};

// global basic types - these are static and should NEVER! be declared anywhere else
inline auto BASIC_TYPE_INT = std::make_shared<IntegerType>();
/*static auto BASIC_TYPE_STRING = StringType();*/
inline auto BASIC_TYPE_BOOL = std::make_shared<BooleanType>();


#endif //INTEGERTYPE_H
