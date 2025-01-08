/*
 * Global definitions.
 *
 * Created by Michael Grossniklaus on 4/7/20.
 */

#ifndef OBERON_LLVM_GLOBAL_H
#define OBERON_LLVM_GLOBAL_H


#include <sstream>
#include <string>

using std::streampos;
using std::string;
using std::stringstream;

struct FilePos {
    string fileName;
    int lineNo, charNo;
    streampos offset;
};

template <typename T>
static string to_string(T obj) {
    stringstream stream;
    stream << obj;
    return stream.str();
}

template <typename T>
static string to_string(T *obj) {
    stringstream stream;
    stream << *obj;
    return stream.str();
}

// both doublewords, for simplicity's sake
#define DATA_WIDTH 64 // how many bits in a value?
#define ADDR_WIDTH 64 // how many bits in an address?

#endif //OBERON_LLVM_GLOBAL_H
