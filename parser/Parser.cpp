//
// Created by mo on 11/21/24.
//

#include "Parser.h"

#include <cstring>
#include <valarray>

#include "global.h"
#include "ast/nodes/IdentNode.h"
#include "ast/nodes/LiteralNode.h"
#include "ast/nodes/OperatorNode.h"
#include "ast/visitor/NodeVisitor.h"
#include "scanner/IdentToken.h"
#include "scoper/Scope.h"
#include "scoper/symbols/Module.h"
#include "scoper/symbols/Procedure.h"


unique_ptr<IdentNode> Parser::ident() {
    auto start = scanner_.peek()->start();
    const auto token = accept(TokenType::const_ident);
    const Token* tokenPtr = token.get(); // token is IdentToken : Token
    // this is dangerous! but dynamic_cast causes sigsegv.
    // even in this form not totally safe. crashes in funny ways sometimes if compilation fails
    // we can be sure that we always end up with a value field, because the accept() call guarantees it
    string name = static_cast<const IdentToken*>(tokenPtr)->value();
    return std::make_unique<IdentNode>(name, start);
}

// parse and consume any whole number token into a literalNode
// if token was a number, writes its value to parameter out
// if not, return nullptr and log error
unique_ptr<LiteralNode> Parser::number() {
    auto start = scanner_.peek()->start();
    if(not expect(TokenType::byte_literal)
        and not expect(TokenType::short_literal)
        and not expect(TokenType::int_literal)
        and not expect(TokenType::long_literal)) {
        stringstream ss;
        ss << "expected a whole number but got a " << scanner_.peek()->type() << " instead";
        logger_.error(scanner_.peek()->start(), ss.str());
        return nullptr;
    }
    // again, very dangerous! but like in ident(), dynamic cast does not work
    // we throw every number into a long because this is a modern compiler!
    // with access to modern hardware on which we can store as many longs as we want!
    const auto token = scanner_.next();
    const Token* tokenPtr = token.get();
    long val = static_cast<const LiteralToken<long>*>(tokenPtr)->value();
    return std::make_unique<LiteralNode>(val, start);
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
    //return nullptr;
    printf("Compilation failed!\n");
    exit(EXIT_FAILURE); // this is lazy. in theory returning nullptrs would allow us to continue looking for errors but that needs more null checks. TODO later!
}

void Parser::parse() {
    /*while(expect(TokenType::kw_module)) {
        module()->print(cout);
    }
    accept(TokenType::eof);*/
    auto mod = std::make_shared<Node>(*module());
    mod->print(cout);
    auto table = std::make_shared<Scope>(logger_, "outermost");
    auto scoper = Scoper(table, logger_);
    scoper.visit(mod);
    auto module_scope = table->lookup<Module>("Sort0")->scope_;
    auto init_proc = module_scope->lookup<Procedure>("Init");
    table->print(cout);
}

std::unique_ptr<Node> Parser::module() {
    auto result = std::make_unique<Node>(NodeType::module, scanner_.peek()->start()); // peek then accept - this makes it so that we never try to dereference a nullptr
    accept(TokenType::kw_module);
    result->append_child(ident());
    accept(TokenType::semicolon);
    result->append_child(declarations());
    if(expect(TokenType::kw_begin)) {
        accept(TokenType::kw_begin);
        result->append_child(statementSequence());
    }
    accept(TokenType::kw_end);
    result->append_child(ident());
    accept(TokenType::period);
    return result;
}

std::unique_ptr<Node> Parser::declarations() {
    bool empty = true; // apply e-prod?
    auto result = std::make_unique<Node>(NodeType::declarations, scanner_.peek()->start());
    if(expect(TokenType::kw_const)) {
        empty = false;
        accept(TokenType::kw_const);
        while(expect(TokenType::const_ident)) {
            auto constant = std::make_unique<Node>(NodeType::dec_const, scanner_.peek()->start());
            constant->append_child(ident());
            accept(TokenType::op_eq);
            constant->append_child(expression());
            accept(TokenType::semicolon);
            result->append_child(std::move(constant));
        }
    }
    if(expect(TokenType::kw_type)) {
        empty = false;
        accept(TokenType::kw_type);
        while(expect(TokenType::const_ident)) {
            auto typedec = std::make_unique<Node>(NodeType::dec_type, scanner_.peek()->start());
            typedec->append_child(ident());
            accept(TokenType::op_eq);
            typedec->append_child(type());
            accept(TokenType::semicolon);
            result->append_child(std::move(typedec));
        }
    }
    if(expect(TokenType::kw_var)) {
        empty = false;
        accept(TokenType::kw_var);
        while(expect(TokenType::const_ident)) {
            auto var = std::make_unique<Node>(NodeType::dec_var, scanner_.peek()->start());
            var->append_child(identList());
            accept(TokenType::colon);
            var->append_child(type());
            accept(TokenType::semicolon);
            result->append_child(std::move(var));
        }
    }
    while(expect(TokenType::kw_procedure)) {
        auto proc = std::make_unique<Node>(NodeType::dec_proc, accept(TokenType::kw_procedure)->start());
        empty = false;
        proc->append_child(ident());
        if(expect(TokenType::lparen)) {
            proc->append_child(formalParameters());
        }
        accept(TokenType::semicolon);
        proc->append_child(declarations());
        if(expect(TokenType::kw_begin)) {
            accept(TokenType::kw_begin);
            proc->append_child(statementSequence());
        }
        accept(TokenType::kw_end);
        proc->append_child(ident());
        accept(TokenType::semicolon);
        result->append_child(std::move(proc));
    }
    // applying e-prod? check follow
    if(empty and not expect(TokenType::kw_begin) and not expect(TokenType::kw_end)) {
        logger_.error(scanner_.peek()->start(), "unexpected token after empty declarations");
    }
    return result;
}

std::unique_ptr<Node> Parser::formalParameters() {
    auto result = std::make_unique<Node>(NodeType::formal_parameters, accept(TokenType::lparen)->start());
    if(expect(TokenType::kw_var) or expect(TokenType::const_ident)) {
        unique_ptr<Node> param;
        if(expect(TokenType::kw_var)) {
            param = std::make_unique<Node>(NodeType::fp_reference, accept(TokenType::kw_var)->start());
        }
        else param = std::make_unique<Node>(NodeType::fp_copy, scanner_.peek()->start());
        param->append_child(identList());
        accept(TokenType::colon);
        param->append_child(type());
        result->append_child(std::move(param));
        while(expect(TokenType::semicolon)) {
            accept(TokenType::semicolon);
            unique_ptr<Node> param;
            if(expect(TokenType::kw_var)) {
                param = std::make_unique<Node>(NodeType::fp_reference, accept(TokenType::kw_var)->start());
            }
            else param = std::make_unique<Node>(NodeType::fp_copy, scanner_.peek()->start());
            param->append_child(identList());
            accept(TokenType::colon);
            param->append_child(type());
            result->append_child(std::move(param));
        }
    }
    accept(TokenType::rparen);
    return result;
}

std::unique_ptr<Node> Parser::type() {
    unique_ptr<Node> result;
    if(expect(TokenType::kw_array)) {
        result = std::make_unique<Node>(NodeType::type_array, accept(TokenType::kw_array)->start());
        result->append_child(expression());
        accept(TokenType::kw_of);
        result->append_child(type());
    }
    else if(expect(TokenType::kw_record)) {
        result = std::make_unique<Node>(NodeType::type_record, accept(TokenType::kw_record)->start());
        if(expect(TokenType::const_ident)) {
            auto fields = std::make_unique<Node>(NodeType::record_field_list, scanner_.peek()->start());
            fields->append_child(identList());
            accept(TokenType::colon);
            fields->append_child(type());
            result->append_child(std::move(fields));
        }
        while(expect(TokenType::semicolon)) {
            accept(TokenType::semicolon);
            if(expect(TokenType::const_ident)) {
                auto fields = std::make_unique<Node>(NodeType::record_field_list, scanner_.peek()->start());
                fields->append_child(identList());
                accept(TokenType::colon);
                fields->append_child(type());
                result->append_child(std::move(fields));
            }
        }
        accept(TokenType::kw_end);
    }
    else {
        result = std::make_unique<Node>(NodeType::type_raw, scanner_.peek()->start());
        result->append_child(ident());
    }
    return result;
}

std::unique_ptr<Node> Parser::identList() {
    auto result = std::make_unique<Node>(NodeType::ident_list, scanner_.peek()->start());
    result->append_child(ident());
    while(expect(TokenType::comma)) {
        accept(TokenType::comma);
        result->append_child(ident());
    }
    return result;
}

std::unique_ptr<Node> Parser::statementSequence() {
    auto result = std::make_unique<Node>(NodeType::statement_seq, scanner_.peek()->start());
    result->append_child(statement());
    while(expect(TokenType::semicolon)) {
        accept(TokenType::semicolon);
        result->append_child(statement());
    }
    return result;
}

std::unique_ptr<Node> Parser::statement() {
    if(expect(TokenType::const_ident)) {
        return assignmentOrProcedureCall();
    }
    else if(expect(TokenType::kw_if)) {
        return ifStatement();
    }
    else if(expect(TokenType::kw_while)) {
        return whileStatement();
    }
    else if(expect(TokenType::kw_repeat)) {
        return repeatStatement();
    }
    // empty?
    else if(not expect(TokenType::semicolon) and not expect(TokenType::kw_end) and not expect(TokenType::kw_elsif) and not expect(TokenType::kw_else) and not expect(TokenType::kw_until)) {
        logger_.error(scanner_.peek()->start(), "unexpected token after statement");
    }
    return std::make_unique<Node>(NodeType::unknown, scanner_.peek()->start());
}

std::unique_ptr<Node> Parser::assignmentOrProcedureCall() {
    auto start = scanner_.peek()->start();
    std::unique_ptr<Node> identifier = ident();
    auto sel = selector();
    std::unique_ptr<Node> result;
    if(expect(TokenType::op_becomes)) {
        // this is an assignment
        accept(TokenType::op_becomes);
        result = std::make_unique<Node>(NodeType::assignment, start);
        result->append_child(std::move(identifier));
        if (sel != nullptr) result->append_child(std::move(sel));
        result->append_child(expression());
    }
    else {
        // this is a procedure call
        // might there be parameters?
        result = std::make_unique<Node>(NodeType::proc_call, start);
        result->append_child(std::move(identifier));
        if (sel != nullptr) result->append_child(std::move(sel));
        if(expect(TokenType::lparen)) {
            // there could be
            accept(TokenType::lparen);
            // not done immediately? there are parameters
            if(not expect(TokenType::rparen)) {
                result->append_child(expression());
                while(expect(TokenType::comma)) {
                    accept(TokenType::comma);
                    result->append_child(expression());
                }
            }
            accept(TokenType::rparen);
        }
    }
    return result;
}

std::unique_ptr<Node> Parser::ifStatement() {
    auto result = std::make_unique<Node>(NodeType::if_statement, accept(TokenType::kw_if)->start());
    result->append_child(expression());
    accept(TokenType::kw_then);
    result->append_child(statementSequence());
    while(expect(TokenType::kw_elsif)) {
        auto elsif = std::make_unique<Node>(NodeType::if_alt, accept(TokenType::kw_elsif)->start());
        expression();
        elsif->append_child(expression());
        accept(TokenType::kw_then);
        elsif->append_child(statementSequence());
        result->append_child(std::move(elsif));
    }
    if(expect(TokenType::kw_else)) {
        auto def = std::make_unique<Node>(NodeType::if_default, accept(TokenType::kw_else)->start());
        def->append_child(statementSequence());
        statementSequence();
        result->append_child(std::move(def));
    }
    accept(TokenType::kw_end);
    return result;
}

std::unique_ptr<Node> Parser::whileStatement() {
    auto result = std::make_unique<Node>(NodeType::while_statement, accept(TokenType::kw_while)->start());
    result->append_child(expression());
    accept(TokenType::kw_do);
    result->append_child(statementSequence());
    accept(TokenType::kw_end);
    return result;
}

std::unique_ptr<Node> Parser::repeatStatement() {
    const auto result = std::make_unique<Node>(NodeType::repeat_statement, accept(TokenType::kw_repeat)->start());
    result->append_child(statementSequence());
    accept(TokenType::kw_until);
    result->append_child(expression());
    return std::make_unique<Node>(NodeType::unknown, scanner_.peek()->start());
}

std::unique_ptr<Node> Parser::expression() {
    auto result = std::make_unique<Node>(NodeType::expression, scanner_.peek()->start());
    auto left = simpleExpression();
    unique_ptr<OperatorNode> op = nullptr;
    unique_ptr<Node> right = nullptr;
    if(expect(TokenType::op_eq)) {
        op = std::make_unique<OperatorNode>(OperatorType::EQ, accept(TokenType::op_eq)->start());
        right = simpleExpression();
    }
    else if(expect(TokenType::op_neq)) {
        op = std::make_unique<OperatorNode>(OperatorType::NEQ, accept(TokenType::op_neq)->start());
        right = simpleExpression();
    }
    else if(expect(TokenType::op_lt)) {
        op = std::make_unique<OperatorNode>(OperatorType::LT, accept(TokenType::op_lt)->start());
        right = simpleExpression();
    }
    else if(expect(TokenType::op_leq)) {
        op = std::make_unique<OperatorNode>(OperatorType::LEQ, accept(TokenType::op_leq)->start());
        right = simpleExpression();
    }
    else if(expect(TokenType::op_gt)) {
        op = std::make_unique<OperatorNode>(OperatorType::GT, accept(TokenType::op_gt)->start());
        right = simpleExpression();
    }
    else if(expect(TokenType::op_geq)) {
        op = std::make_unique<OperatorNode>(OperatorType::GEQ, accept(TokenType::op_geq)->start());
        right = simpleExpression();
    }
    if (op != nullptr) {
        result->append_child(std::move(left));
        result->append_child(std::move(op));
        result->append_child(std::move(right));
        return result;
    }
    return left; // only return the left side if there was no right side
}

std::unique_ptr<Node> Parser::simpleExpression() {
    auto result = std::make_unique<Node>(NodeType::expression, scanner_.peek()->start());
    unique_ptr<OperatorNode> lead = nullptr;
    if(expect(TokenType::op_plus)) {
        lead = std::make_unique<OperatorNode>(OperatorType::PLUS, accept(TokenType::op_plus)->start());
    }
    else if(expect(TokenType::op_minus)) {
        lead = std::make_unique<OperatorNode>(OperatorType::MINUS, accept(TokenType::op_minus)->start());
    }
    auto left = term();
    unique_ptr<OperatorNode> op = nullptr;
    unique_ptr<Node> right;
    while(expect(TokenType::op_plus) or expect(TokenType::op_minus) or expect(TokenType::op_or)) {
        if(expect(TokenType::op_plus)) {
            op = std::make_unique<OperatorNode>(OperatorType::PLUS, accept(TokenType::op_plus)->start());
        }
        else if(expect(TokenType::op_minus)) {
            op = std::make_unique<OperatorNode>(OperatorType::MINUS, accept(TokenType::op_minus)->start());
        }
        else if(expect(TokenType::op_or)) {
            op = std::make_unique<OperatorNode>(OperatorType::OR, accept(TokenType::op_or)->start());
        }
        right = term();
    }
    if (!op && !lead) return left;
    if (lead) result->append_child(std::move(lead));
    result->append_child(std::move(left));
    result->append_child(std::move(op));
    result->append_child(std::move(right));
    return result;
}

std::unique_ptr<Node> Parser::term() {
    auto result = std::make_unique<Node>(NodeType::expression, scanner_.peek()->start());
    auto left = factor();
    unique_ptr<OperatorNode> op = nullptr;
    unique_ptr<Node> right = nullptr;
    while(expect(TokenType::op_times) or expect(TokenType::op_div) or expect(TokenType::op_mod) or expect(TokenType::op_and)) {
        if(expect(TokenType::op_times)) {
            op = std::make_unique<OperatorNode>(OperatorType::TIMES, accept(TokenType::op_times)->start());
        }
        else if(expect(TokenType::op_div)) {
            op = std::make_unique<OperatorNode>(OperatorType::DIV, accept(TokenType::op_div)->start());
        }
        else if(expect(TokenType::op_mod)) {
            op = std::make_unique<OperatorNode>(OperatorType::MOD, accept(TokenType::op_mod)->start());
        }
        else if(expect(TokenType::op_and)) {
            op = std::make_unique<OperatorNode>(OperatorType::AND, accept(TokenType::op_and)->start());
        }
        right = factor();
    }
    if (!op) return left;
    result->append_child(std::move(left));
    result->append_child(std::move(op));
    result->append_child(std::move(right));
    return result;
}

std::unique_ptr<Node> Parser::factor() {
    auto result = std::make_unique<Node>(NodeType::expression, scanner_.peek()->start());
    if(expect(TokenType::const_ident)) {
        // factor is an identifier with selectors
        auto id = ident();
        result->append_child(std::move(id)); // yucky yucky pointer wrangling! but if it's stupid and it works, it isn't stupid
        auto id_moved = std::dynamic_pointer_cast<IdentNode>(result->children().front());
        auto sel = selector();
        if (sel != nullptr) {
            result->append_child(std::move(sel));
            id_moved->set_selector(result->children().back());
        }
    }
    else if(expect(TokenType::lparen)) {
        // factor is an expressionNode with no leading operator
        accept(TokenType::lparen);
        auto res = expression();
        accept(TokenType::rparen);
        return res;
    }
    else if(expect(TokenType::op_not)) {
        // factor is an expressionNode with a leading not
        auto op = std::make_unique<OperatorNode>(OperatorType::NOT, accept(TokenType::op_not)->start());
        result->append_child(std::move(op));
        result->append_child(factor());
    }
    else {
        // factor is a number literal
        return number();
    }
    return result;
}

// returns null if there are no selectors
std::unique_ptr<Node> Parser::selector() {
    bool empty = true;
    auto result = std::make_unique<Node>(NodeType::selector_block, scanner_.peek()->start());
    while(expect(TokenType::period) or expect(TokenType::lbrack)) {
        empty = false;
        if(expect(TokenType::period)) {
            result->append_child(std::make_unique<Node>(NodeType::sel_field, accept(TokenType::period)->start()));
            result->children().back()->append_child(ident());
        }
        else {
            result->append_child(std::make_unique<Node>(NodeType::sel_index, accept(TokenType::lbrack)->start()));
            result->children().back()->append_child(expression());
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
    if (!empty) return result;
    return nullptr;
}
