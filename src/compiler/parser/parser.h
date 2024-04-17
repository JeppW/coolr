#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "../../common/token.h"
#include "../../common/ast.h"
#include "../../common/consts.h"
#include "../../utils/errors.h"

class Parser {
    private:
        ProgramNode ast;

        void parse_program(Tokenstream&);
        void parse_class(Tokenstream&);
        void parse_features(Tokenstream&, ClassNode*);
        void parse_formals(Tokenstream&, MethodNode*);

        ExpressionNode* parse_expression(Tokenstream&);
        ExpressionNode* parse_int_const(Tokenstream&);
        ExpressionNode* parse_string_const(Tokenstream&);
        ExpressionNode* parse_bool_const(Tokenstream&);
        ExpressionNode* parse_identifier(Tokenstream&);
        ExpressionNode* parse_assignment(Tokenstream&);
        ExpressionNode* parse_new(Tokenstream&);
        ExpressionNode* parse_isvoid(Tokenstream&);
        ExpressionNode* parse_neg(Tokenstream&);
        ExpressionNode* parse_complement(Tokenstream&);
        ExpressionNode* parse_conditional(Tokenstream&);
        ExpressionNode* parse_while(Tokenstream&);
        ExpressionNode* parse_block(Tokenstream&);
        ExpressionNode* parse_let(Tokenstream&);
        ExpressionNode* parse_case(Tokenstream&);
        ExpressionNode* parse_parentheses(Tokenstream&);
        ExpressionNode* parse_dispatch(Tokenstream&);
        
        ExpressionNode* parse_single_expression(Tokenstream&);
        ExpressionNode* post_expression(Tokenstream&, ExpressionNode*);    
        std::vector<ExpressionNode*> parse_dispatch_parameters(Tokenstream&);

        bool peek_equals(Tokenstream&, TokenType);

    public:
        ProgramNode parse(Tokenstream&);
};

#endif