/*
 * Parser of the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 2/2/18.
 */

#ifndef OBERON0C_PARSER_H
#define OBERON0C_PARSER_H


#include <string>
#include "scanner/Scanner.h"
#include "ast/Node.h"

using std::string;

class Parser {

private:
    Scanner &scanner_;
    Logger &logger_;

    const string ident();

    void module();

public:
    explicit Parser(Scanner &scanner, Logger &logger) : scanner_(scanner), logger_(logger) {};
    ~Parser() = default;
    void parse();

};


#endif //OBERON0C_PARSER_H
