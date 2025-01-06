//
// Created by moltmanns on 12/21/24.
//

#ifndef INTEGERTYPE_H
#define INTEGERTYPE_H
#include "Type.h"


class IntegerType : public Type {
public:
    explicit IntegerType() : Type("integer") {};
};

class StringType : public Type {
public:
    explicit StringType() : Type("string") {};
};

class BooleanType : public Type {
public:
    explicit BooleanType() : Type("boolean") {};
};


#endif //INTEGERTYPE_H
