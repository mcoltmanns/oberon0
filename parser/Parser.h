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

    bool number(long * out);

    void module();

    void declarations();

    void formalParameters();

    void type();

    void identList();

    void statementSequence();

    void statement();

    void assignmentOrProcedureCall();

    void ifStatement();

    void whileStatement();

    void repeatStatement();

    void expression();

    void simpleExpression();

    void term();

    void factor();

    void selector();

public:
    explicit Parser(Scanner &scanner, Logger &logger) : scanner_(scanner), logger_(logger) {};
    ~Parser() = default;
    void parse();

    bool expect(TokenType token);

    unique_ptr<const Token> accept(TokenType token);

    unique_ptr<const IntLiteralToken> expectInt();
};


#endif //OBERON0C_PARSER_H
