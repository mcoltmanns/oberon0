//
// Created by mo on 11/21/24.
//

#include "Parser.h"
#include "global.h"


const string Parser::ident() {
    return to_string(accept(TokenType::const_ident));
}

// parse and consume any whole number token into a long integer
// if token was a number, writes its value to parameter out
// logs error, does nothing with out, and returns false if token wasn't a number
bool Parser::number(long * out) {
    if(not expect(TokenType::byte_literal)
        and not expect(TokenType::short_literal)
        and not expect(TokenType::int_literal)
        and not expect(TokenType::long_literal)) {
        stringstream ss;
        ss << "expected a whole number but got a " << scanner_.peek()->type() << " instead";
        logger_.error(scanner_.peek()->start(), ss.str());
        return false;
    }
    // holy jank! there must be a better way
    *out = stol(to_string(scanner_.next()));
    return true;
}

// expect a certain token type
// if ok, return true
// if bad, return false
bool Parser::expect(TokenType token) {
    return scanner_.peek()->type() == token;
}

// accept a certain token type
// if ok, consume and return the token
// if bad, log an error, do not consume, and return nullptr
unique_ptr<const Token> Parser::accept(TokenType token) {
    if(scanner_.peek()->type() == token) {
        return scanner_.next();
    }
    stringstream ss;
    ss << "expected " << token << " but got " << scanner_.peek()->type() << " instead";
    logger_.error(scanner_.peek()->start(), ss.str());
    return nullptr;
}

void Parser::parse() {
    while(expect(TokenType::kw_module)) {
        module();
    }
    accept(TokenType::eof);
}

void Parser::module() {
    accept(TokenType::kw_module);
    auto open = ident();
    accept(TokenType::semicolon);
    declarations();
    if(expect(TokenType::kw_begin)) {
        accept(TokenType::kw_begin);
        statementSequence();
    }
    accept(TokenType::kw_end);
    auto close = ident();
    accept(TokenType::period);
}

void Parser::declarations() {
    bool empty = true; // apply e-prod?
    if(expect(TokenType::kw_const)) {
        empty = false;
        accept(TokenType::kw_const);
        while(expect(TokenType::const_ident)) {
            ident();
            accept(TokenType::op_eq);
            expression();
            accept(TokenType::semicolon);
        }
    }
    if(expect(TokenType::kw_type)) {
        empty = false;
        accept(TokenType::kw_type);
        while(expect(TokenType::const_ident)) {
            ident();
            accept(TokenType::op_eq);
            type();
            accept(TokenType::semicolon);
        }
    }
    if(expect(TokenType::kw_var)) {
        empty = false;
        accept(TokenType::kw_var);
        while(expect(TokenType::const_ident)) {
            identList();
            accept(TokenType::colon);
            type();
            accept(TokenType::semicolon);
        }
    }
    while(expect(TokenType::kw_procedure)) {
        empty = false;
        accept(TokenType::kw_procedure);
        ident();
        if(expect(TokenType::lparen)) {
            formalParameters();
        }
        accept(TokenType::semicolon);
        declarations();
        if(expect(TokenType::kw_begin)) {
            accept(TokenType::kw_begin);
            statementSequence();
        }
        accept(TokenType::kw_end);
        ident();
        accept(TokenType::semicolon);
    }
    // applying e-prod? check follow
    if(empty and not expect(TokenType::kw_begin) and not expect(TokenType::kw_end)) {
        logger_.error(scanner_.peek()->start(), "unexpected token after empty declarations");
    }
}

void Parser::formalParameters() {
    accept(TokenType::lparen);
    if(expect(TokenType::kw_var) or expect(TokenType::const_ident)) {
        if(expect(TokenType::kw_var)) {
            accept(TokenType::kw_var);
        }
        identList();
        accept(TokenType::colon);
        type();
        while(expect(TokenType::semicolon)) {
            accept(TokenType::semicolon);
            if(expect(TokenType::kw_var)) {
                accept(TokenType::kw_var);
            }
            identList();
            accept(TokenType::colon);
            type();
        }
    }
    accept(TokenType::rparen);
}

void Parser::type() {
    if(expect(TokenType::kw_array)) {
        accept(TokenType::kw_array);
        expression();
        accept(TokenType::kw_of);
        type();
    }
    else if(expect(TokenType::kw_record)) {
        accept(TokenType::kw_record);
        if(expect(TokenType::const_ident)) {
            identList();
            accept(TokenType::colon);
            type();
        }
        while(expect(TokenType::semicolon)) {
            accept(TokenType::semicolon);
            if(expect(TokenType::const_ident)) {
                identList();
                accept(TokenType::colon);
                type();
            }
        }
        accept(TokenType::kw_end);
    }
    else {
        ident();
    }
}

void Parser::identList() {
    ident();
    while(expect(TokenType::comma)) {
        accept(TokenType::comma);
        ident();
    }
}

void Parser::statementSequence() {
    statement();
    while(expect(TokenType::semicolon)) {
        accept(TokenType::semicolon);
        statement();
    }
}

void Parser::statement() {
    if(expect(TokenType::const_ident)) {
        assignmentOrProcedureCall();
    }
    else if(expect(TokenType::kw_if)) {
        ifStatement();
    }
    else if(expect(TokenType::kw_while)) {
        whileStatement();
    }
    else if(expect(TokenType::kw_repeat)) {
        repeatStatement();
    }
    // empty?
    else if(not expect(TokenType::semicolon) and not expect(TokenType::kw_end) and not expect(TokenType::kw_elsif) and not expect(TokenType::kw_else) and not expect(TokenType::kw_until)) {
        logger_.error(scanner_.peek()->start(), "unexpected token after statement");
    }
}

void Parser::assignmentOrProcedureCall() {
    ident();
    selector();
    if(expect(TokenType::op_becomes)) {
        // this is an assignment
        accept(TokenType::op_becomes);
        expression();
    }
    else {
        // this is a procedure call
        // might there be parameters?
        if(expect(TokenType::lparen)) {
            // there could be
            accept(TokenType::lparen);
            // not done immediately? there are parameters
            if(not expect(TokenType::rparen)) {
                expression();
                while(expect(TokenType::comma)) {
                    accept(TokenType::comma);
                    expression();
                }
            }
            accept(TokenType::rparen);
        }
    }
}

void Parser::ifStatement() {
    accept(TokenType::kw_if);
    expression();
    accept(TokenType::kw_then);
    statementSequence();
    while(expect(TokenType::kw_elsif)) {
        accept(TokenType::kw_elsif);
        expression();
        accept(TokenType::kw_then);
        statementSequence();
    }
    if(expect(TokenType::kw_else)) {
        accept(TokenType::kw_else);
        statementSequence();
    }
    accept(TokenType::kw_end);
}

void Parser::whileStatement() {
    accept(TokenType::kw_while);
    expression();
    accept(TokenType::kw_do);
    statementSequence();
    accept(TokenType::kw_end);
}

void Parser::repeatStatement() {
    accept(TokenType::kw_repeat);
    statementSequence();
    accept(TokenType::kw_until);
    expression();
}

void Parser::expression() {
    simpleExpression();
    if(expect(TokenType::op_eq)) {
        accept(TokenType::op_eq);
        simpleExpression();
    }
    else if(expect(TokenType::op_neq)) {
        accept(TokenType::op_neq);
        simpleExpression();
    }
    else if(expect(TokenType::op_lt)) {
        accept(TokenType::op_lt);
        simpleExpression();
    }
    else if(expect(TokenType::op_leq)) {
        accept(TokenType::op_leq);
        simpleExpression();
    }
    else if(expect(TokenType::op_gt)) {
        accept(TokenType::op_gt);
        simpleExpression();
    }
    else if(expect(TokenType::op_geq)) {
        accept(TokenType::op_geq);
        simpleExpression();
    }
}

void Parser::simpleExpression() {
    if(expect(TokenType::op_plus)) {
        accept(TokenType::op_plus);
    }
    else if(expect(TokenType::op_minus)) {
        accept(TokenType::op_minus);
    }
    term();
    while(expect(TokenType::op_plus) or expect(TokenType::op_minus) or expect(TokenType::op_or)) {
        if(expect(TokenType::op_plus)) {
            accept(TokenType::op_plus);
            term();
        }
        else if(expect(TokenType::op_minus)) {
            accept(TokenType::op_minus);
            term();
        }
        else if(expect(TokenType::op_or)) {
            accept(TokenType::op_or);
            term();
        }
    }
}

void Parser::term() {
    factor();
    while(expect(TokenType::op_times) or expect(TokenType::op_div) or expect(TokenType::op_mod) or expect(TokenType::op_and)) {
        if(expect(TokenType::op_times)) {
            accept(TokenType::op_times);
            factor();
        }
        else if(expect(TokenType::op_div)) {
            accept(TokenType::op_div);
            factor();
        }
        else if(expect(TokenType::op_mod)) {
            accept(TokenType::op_mod);
            factor();
        }
        else if(expect(TokenType::op_and)) {
            accept(TokenType::op_and);
            factor();
        }
    }
}

void Parser::factor() {
    if(expect(TokenType::const_ident)) {
        ident();
        selector();
    }
    else if(expect(TokenType::lparen)) {
        accept(TokenType::lparen);
        expression();
        accept(TokenType::rparen);
    }
    else if(expect(TokenType::op_not)) {
        accept(TokenType::op_not);
        factor();
    }
    else {
        long val;
        number(&val);
    }
}

void Parser::selector() {
    bool empty = true;
    while(expect(TokenType::period) or expect(TokenType::lbrack)) {
        empty = false;
        if(expect(TokenType::period)) {
            accept(TokenType::period);
            ident();
        }
        else {
            accept(TokenType::lbrack);
            expression();
            accept(TokenType::rbrack);
        }
    }
    if(empty
        and not expect(TokenType::op_becomes)
        and not expect(TokenType::lparen)
        and not expect(TokenType::op_times)
        and not expect(TokenType::op_div)
        and not expect(TokenType::op_mod)
        and not expect(TokenType::op_and)
        and not expect(TokenType::op_plus)
        and not expect(TokenType::op_minus)
        and not expect(TokenType::op_or)
        and not expect(TokenType::op_eq)
        and not expect(TokenType::op_neq)
        and not expect(TokenType::op_lt)
        and not expect(TokenType::op_leq)
        and not expect(TokenType::op_gt)
        and not expect(TokenType::op_geq)
        and not expect(TokenType::semicolon)
        and not expect(TokenType::kw_end)
        and not expect(TokenType::kw_elsif)
        and not expect(TokenType::kw_else)
        and not expect(TokenType::kw_until)
        and not expect(TokenType::kw_do)
        and not expect(TokenType::kw_then)
        and not expect(TokenType::kw_of)
        and not expect(TokenType::rparen)
        and not expect(TokenType::rbrack)
        and not expect(TokenType::comma)) {
        logger_.error(scanner_.peek()->start(), "unexpected token after selector");
    }
}
