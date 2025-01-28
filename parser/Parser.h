/*
 * Parser of the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 2/2/18.
 */

#ifndef OBERON0C_PARSER_H
#define OBERON0C_PARSER_H


#include <string>

#include "ast/nodes/IdentNode.h"
#include "ast/nodes/LiteralNode.h"
#include "scanner/Scanner.h"
#include "ast/nodes/Node.h"

using std::string;

class Parser {

private:
    Scanner &scanner_;
    Logger &logger_;

    unique_ptr<IdentNode> ident();

    unique_ptr<LiteralNode> number();

    unique_ptr<LiteralNode> boolean();

    std::unique_ptr<Node> module();

    std::unique_ptr<Node> declarations();

    std::unique_ptr<Node> formalParameters();

    std::unique_ptr<Node> type();

    std::unique_ptr<Node> identList();

    std::unique_ptr<Node> statementSequence();

    std::unique_ptr<Node> statement();

    std::unique_ptr<Node> assignmentOrProcedureCall();

    std::unique_ptr<Node> ifStatement();

    std::unique_ptr<Node> whileStatement();

    std::unique_ptr<Node> repeatStatement();

    std::unique_ptr<Node> expression();

    std::unique_ptr<Node> simpleExpression();

    std::unique_ptr<Node> term();

    std::unique_ptr<Node> factor();

    std::unique_ptr<Node> selector();

public:
    explicit Parser(Scanner &scanner, Logger &logger) : scanner_(scanner), logger_(logger) {};
    ~Parser() = default;

    std::shared_ptr<Node> parse();

    bool expect(TokenType token);

    unique_ptr<const Token> accept(TokenType token);

    unique_ptr<const IntLiteralToken> expectInt();
};


#endif //OBERON0C_PARSER_H
