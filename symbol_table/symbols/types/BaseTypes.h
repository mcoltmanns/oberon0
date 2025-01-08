//
// Created by moltmanns on 12/21/24.
//

#ifndef INTEGERTYPE_H
#define INTEGERTYPE_H
#include "Type.h"

// for now we assume all base types take up 1 cell in storage
class IntegerType : public Type {
public:
    explicit IntegerType() : Type("INTEGER", 1) {};
    virtual ~IntegerType();

    void print(std::ostream& s) override;
};

class StringType : public Type {
public:
    explicit StringType() : Type("STRING", 1) {};
    virtual ~StringType();

    void print(std::ostream& s) override;
};

class BooleanType : public Type {
public:
    explicit BooleanType() : Type("BOOLEAN", 1) {};
    virtual ~BooleanType();

    void print(std::ostream& s) override;
};


#endif //INTEGERTYPE_H
