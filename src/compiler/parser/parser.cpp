#include "parser.h"

/*
 *  Parser.
 *
 *  The recursive descent parser operates on a Tokenstream object  
 *  and constructs an abstract syntax tree (AST).
 */

bool Parser::peek_equals(Tokenstream& ts, TokenType type) {
    // check if the next token in the stream matches the supplied type
    Token* t = ts.peek();

    if (!t) {
        // if there is no next token, we have unexpectedly reached
        // the end of the token stream, so we exit with an error
        parser_error(ts, ts.get());
    }

    return t->get_type() == type;
}

ProgramNode Parser::parse(Tokenstream& ts) {
    parse_program(ts);
    return ast;
}

void Parser::parse_program(Tokenstream& ts) {
    ast.set_line_number(ts.get_line_number());

    // parse each class in the program in a top-down manner
    // do-while, because at least one class is required
    do {
        Token* t = ts.get();
        if (!t || t->get_type() != TokenType::CLASS) {
            return parser_error(ts, t);
        } else {
            parse_class(ts);
        }

    } while (!ts.eof());
}

void Parser::parse_class(Tokenstream& ts) {
    Token* next = ts.get();

    if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
        return parser_error(ts, next);
    }

    TypeIdToken* class_name = dynamic_cast<TypeIdToken*>(next);
    ClassNode* node = new ClassNode(class_name->get_value());
    node->set_line_number(ts.get_line_number());

    // unless otherwise is specified, all classes
    // inherit from Object
    node->set_base_class(Strings::Types::Object);

    Token* t = ts.peek();
    if (!t) { parser_error(ts, ts.get()); }

    switch (t->get_type()) {
        // parse class that inherits from another class
        case TokenType::INHERITS: {
            ts.consume();
            next = ts.get();

            if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
                return parser_error(ts, next);
            }
            
            TypeIdToken* base_class = dynamic_cast<TypeIdToken*>(next);
            node->set_base_class(base_class->get_value());

            // intentionally no break - we want to continue
            // to the next case, which is the class body
        }

        case TokenType::CURLY_BRACKET_OPEN: {
            // parse class body
            next = ts.get();
    
            if (!next || next->get_type() != TokenType::CURLY_BRACKET_OPEN) {
                return parser_error(ts, next);
            }

            parse_features(ts, node);

            // make sure class is terminated properly with '};'
            next = ts.get();
            if (!next || next->get_type() != TokenType::CURLY_BRACKET_CLOSE) {
                return parser_error(ts, next);
            }

            next = ts.get();
            if (!next || next->get_type() != TokenType::SEMICOLON) {
                return parser_error(ts, next);
            }

            break;
        }

        default:
            // if another keyword is encountered, it is invalid syntax
            parser_error(ts, ts.get());
            break;
    }

    ast.add_class(node);
}

void Parser::parse_features(Tokenstream& ts, ClassNode* cls) {
    Token* next;

    for (;;) {
        if (!peek_equals(ts, TokenType::OBJ_IDENTIFIER)) {
            // done parsing class features
            return;
        }

        Token* identifier = ts.get();
        ObjIdToken* feature_name = dynamic_cast<ObjIdToken*>(identifier);

        Token* t = ts.peek();
        if (!t) { parser_error(ts, ts.get()); }
        switch(t->get_type()) {
            case TokenType::COLON: {
                // attribute feature
                ts.consume();
                
                next = ts.get();
                if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
                    return parser_error(ts, next);
                } 

                // built attribute node from identifier and type
                TypeIdToken* attribute_type = dynamic_cast<TypeIdToken*>(next);
                AttributeNode* attr_node = new AttributeNode(feature_name->get_value());
                attr_node->set_type(attribute_type->get_value());
                attr_node->set_line_number(ts.get_line_number());

                // check if the optional initialization expression is present
                if (peek_equals(ts, TokenType::ASSIGN)) {
                    // initialization expression
                    ts.consume();
                    ExpressionNode* expr = parse_expression(ts);
                    attr_node->set_expr(expr);
                } else {
                    // no initialization, assign _no_expr instead
                    ExpressionNode* no_expr = new NoExpressionNode(attr_node->get_type());
                    no_expr->set_line_number(ts.get_line_number());
                    attr_node->set_expr(no_expr);
                }

                // make sure attribute feature is terminated by semicolon
                next = ts.get();
                if (!next || next->get_type() != TokenType::SEMICOLON) {
                    return parser_error(ts, next);
                }

                cls->add_attribute(attr_node);
                break;
            }

            case TokenType::PARENTHESIS_OPEN: {
                // method feature
                ts.consume();
                MethodNode* method_node = new MethodNode(feature_name->get_value());
                method_node->set_line_number(ts.get_line_number());

                parse_formals(ts, method_node);

                if (!peek_equals(ts, TokenType::COLON)) {
                    return parser_error(ts, ts.get());
                }

                ts.consume();

                if (!peek_equals(ts, TokenType::TYPE_IDENTIFIER)){
                    return parser_error(ts, ts.get());
                }

                Token* type = ts.get();
                TypeIdToken* type_token = dynamic_cast<TypeIdToken*>(type);
                method_node->set_type(type_token->get_value());

                next = ts.get();
                if (!next || next->get_type() != TokenType::CURLY_BRACKET_OPEN) {
                    return parser_error(ts, next);
                }

                // parse method body
                ExpressionNode* expr = parse_expression(ts);
                method_node->set_expr(expr);

                // make sure method is terminated properly with '};'
                next = ts.get();
                if (!next || next->get_type() != TokenType::CURLY_BRACKET_CLOSE) {
                    return parser_error(ts, next);
                }

                next = ts.get();
                if (!next || next->get_type() != TokenType::SEMICOLON) {
                    return parser_error(ts, next);
                }

                cls->add_method(method_node);
                break;
            }
            
            default: {
                // we got another identifier, but not 
                // an attribute or method, so raise an error
                return parser_error(ts, ts.get());
            }
        }
    }
}

void Parser::parse_formals(Tokenstream& ts, MethodNode* node) {
    // sets the method parameters of a method node
    FormalsNode* formals = new FormalsNode();
    formals->set_line_number(ts.get_line_number());

    for (;;) {
        Token* t = ts.peek();
        if (!t) { parser_error(ts, ts.get()); }

        switch (t->get_type()) {
            case TokenType::PARENTHESIS_CLOSE:
                // got closing parenthesis, stop parsing formals
                ts.consume();
                return node->set_formals(formals);
            
            case TokenType::OBJ_IDENTIFIER: {
                // parse another formal
                Token* name = ts.get();
                ObjIdToken* name_token = dynamic_cast<ObjIdToken*>(name);
                
                if (!peek_equals(ts, TokenType::COLON)) {
                    return parser_error(ts, ts.get());
                }

                ts.consume();

                if (!peek_equals(ts, TokenType::TYPE_IDENTIFIER)) {
                    return parser_error(ts, ts.get());
                }

                Token* type = ts.get();
                TypeIdToken* type_token = dynamic_cast<TypeIdToken*>(type);

                FormalNode* formal = new FormalNode(name_token->get_value(), type_token->get_value());
                formal->set_line_number(ts.get_line_number());
                formals->add_formal(formal);

                if (peek_equals(ts, TokenType::COMMA)) {
                    ts.consume();
                } else if (!peek_equals(ts, TokenType::PARENTHESIS_CLOSE)) {
                    // if the next character is not a comma, it has
                    // to be a closing parenthesis. if it is not,
                    // we throw an error
                    return parser_error(ts, ts.get());
                }

                break;
            }

            default: 
                return parser_error(ts, ts.get());
        }
    }
}

/*
 *  Methods for parsing the actual expressions
 *  in the class methods and attributes
 */ 

ExpressionNode* Parser::parse_expression(Tokenstream& ts) {
    // we parse expressions by looking a few tokens ahead
    // expressions that start with an expression (e.g. <expr> + <expr>)
    // are handled in post_expression
    ExpressionNode* expr = parse_single_expression(ts);
    return post_expression(ts, expr);
}

ExpressionNode* Parser::parse_single_expression(Tokenstream& ts) {
    ExpressionNode* expr;

    // find a production that matches the current 
    // position in the token stream
    if (expr = parse_assignment(ts)) {}
    else if (expr = parse_dispatch(ts)) {} 
    else if (expr = parse_conditional(ts)) {} 
    else if (expr = parse_while(ts)) {}
    else if (expr = parse_block(ts)) {} 
    else if (expr = parse_let(ts)) {}
    else if (expr = parse_case(ts)) {} 
    else if (expr = parse_new(ts)) {}
    else if (expr = parse_isvoid(ts)) {}
    else if (expr = parse_neg(ts)) {}
    else if (expr = parse_complement(ts)) {}
    else if (expr = parse_int_const(ts)) {} 
    else if (expr = parse_string_const(ts)) {} 
    else if (expr = parse_bool_const(ts)) {} 
    else if (expr = parse_identifier(ts)) {}
    else if (expr = parse_parentheses(ts)) {}

    else {
        // no expression that matches the next token was found
        parser_error(ts, ts.get());
    }

    return expr;
}


//
// parse the simple constructs: Int, String and Bool literals and object identifiers
//

ExpressionNode* Parser::parse_int_const(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::INTEGER)) {
        Token* token = ts.get();
        IntToken* int_token = dynamic_cast<IntToken*>(token);
        IntNode* int_node = new IntNode(int_token->get_value());
        int_node->set_line_number(ts.get_line_number());
        return int_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_string_const(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::STRING)) {
        Token* token = ts.get();
        StringToken* string_token = dynamic_cast<StringToken*>(token);
        StringNode* string_node = new StringNode(string_token->get_value());
        string_node->set_line_number(ts.get_line_number());
        return string_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_bool_const(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::BOOL)) {
        Token* token = ts.get();
        BoolToken* bool_token = dynamic_cast<BoolToken*>(token);
        BoolNode* bool_node = new BoolNode(bool_token->get_value());
        bool_node->set_line_number(ts.get_line_number());
        return bool_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_identifier(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::OBJ_IDENTIFIER)) {
        Token* token = ts.get();
        ObjIdToken* obj_token = dynamic_cast<ObjIdToken*>(token);
        IdentifierNode* obj_node = new IdentifierNode(obj_token->get_value());
        obj_node->set_line_number(ts.get_line_number());
        return obj_node;
    }
    
    return nullptr;
}

//
// parse assignments
//

ExpressionNode* Parser::parse_assignment(Tokenstream& ts) {
    Token* t1 = ts.get();
    Token* t2 = ts.get();

    if (!t1) { parser_error(ts, t1); }
    if (!t2) { parser_error(ts, t2); }

    if (t1->get_type() == TokenType::OBJ_IDENTIFIER && t2->get_type() == TokenType::ASSIGN) {
        ObjIdToken* name = dynamic_cast<ObjIdToken*>(t1);
        AssignmentNode* assign_node = new AssignmentNode(name->get_value());
        assign_node->set_line_number(ts.get_line_number());
        ExpressionNode* expr = parse_expression(ts);
        assign_node->set_expr(expr);
        return assign_node;
    }

    ts.unget(2);
    return nullptr;
}

//
// parse object initialization
//

ExpressionNode* Parser::parse_new(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::NEW)) {
        ts.consume();
        Token* next = ts.get();

        if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
            // it is only allowed to call 'new' on type identifiers
            parser_error(ts, next);
        }

        TypeIdToken* type_token = dynamic_cast<TypeIdToken*>(next);
        NewNode* new_node = new NewNode(type_token->get_value());
        new_node->set_line_number(ts.get_line_number());
        return new_node;
    }

    return nullptr;
}

//
// parse unary operators
// 

ExpressionNode* Parser::parse_neg(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::SQUIGGLE)) {
        ts.consume();
        NegNode* neg_node = new NegNode();
        neg_node->set_line_number(ts.get_line_number());
        ExpressionNode* expr = parse_single_expression(ts);
        neg_node->set_expr(expr);
        return neg_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_complement(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::NOT)) {
        ts.consume();
        ComplementNode* comp_node = new ComplementNode();
        comp_node->set_line_number(ts.get_line_number());
        ExpressionNode* expr = parse_single_expression(ts);
        comp_node->set_expr(expr);
        return comp_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_isvoid(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::ISVOID)) {
        ts.consume();
        IsvoidNode* isvoid_node = new IsvoidNode();
        isvoid_node->set_line_number(ts.get_line_number());
        ExpressionNode* expr = parse_single_expression(ts);
        isvoid_node->set_expr(expr);
        return isvoid_node;
    }
    
    return nullptr;
}

//
// parse control structures (conditionals, loops, switch cases)
//

ExpressionNode* Parser::parse_conditional(Tokenstream& ts) {
    Token* next;

    if (peek_equals(ts, TokenType::IF)) {
        ts.consume();

        ConditionalNode* cond_node = new ConditionalNode();
        cond_node->set_line_number(ts.get_line_number());

        ExpressionNode* predicate = parse_expression(ts);

        next = ts.get();
        if (!next || next->get_type() != TokenType::THEN) {
            parser_error(ts, next);
        }

        ExpressionNode* then_expr = parse_expression(ts);

        next = ts.get();
        if (!next || next->get_type() != TokenType::ELSE) {
            parser_error(ts, next);
        }

        ExpressionNode* else_expr = parse_expression(ts);

        next = ts.get();
        if (!next || next->get_type() != TokenType::FI) {
            parser_error(ts, next);
        }

        cond_node->set_predicate(predicate);
        cond_node->set_then(then_expr);
        cond_node->set_else(else_expr);

        return cond_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_while(Tokenstream& ts) {
    Token* next;

    if (peek_equals(ts, TokenType::WHILE)) {
        ts.consume();

        WhileNode* while_node = new WhileNode();
        while_node->set_line_number(ts.get_line_number());

        ExpressionNode* predicate = parse_expression(ts);

        next = ts.get();
        if (!next || next->get_type() != TokenType::LOOP) {
            parser_error(ts, next);
        }

        ExpressionNode* body = parse_expression(ts);

        next = ts.get();
        if (!next || next->get_type() != TokenType::POOL) {
            parser_error(ts, next);
        }

        while_node->set_predicate(predicate);
        while_node->set_body(body);

        return while_node;
    }
    
    return nullptr;
}

ExpressionNode* Parser::parse_case(Tokenstream& ts) {
    Token* next;

    if (peek_equals(ts, TokenType::CASE)) {
        ts.consume();

        CaseNode* case_node = new CaseNode();
        case_node->set_line_number(ts.get_line_number());

        ExpressionNode* expr = parse_expression(ts);
        case_node->set_target(expr);

        next = ts.get();
        if (!next || next->get_type() != TokenType::OF) {
            parser_error(ts, next);
        }

        // parse branches
        // do-while, because at least one branch is required
        do {           
            next = ts.get();
            if (!next || next->get_type() != TokenType::OBJ_IDENTIFIER) {
                parser_error(ts, next);
            }
            ObjIdToken* identifier = dynamic_cast<ObjIdToken*>(next);

            next = ts.get();
            if (!next || next->get_type() != TokenType::COLON) {
                parser_error(ts, next);
            }

            next = ts.get();
            if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
                parser_error(ts, next);
            }

            TypeIdToken* type = dynamic_cast<TypeIdToken*>(next);

            next = ts.get();
            if (!next || next->get_type() != TokenType::ARROW) {
                parser_error(ts, next);
            }

            CaseBranchNode* branch_node = new CaseBranchNode(identifier->get_value(), type->get_value());
            branch_node->set_line_number(ts.get_line_number());
            ExpressionNode* expr = parse_expression(ts);
            branch_node->set_expr(expr);
            case_node->add_branch(branch_node);

            next = ts.get();
            if (!next || next->get_type() != TokenType::SEMICOLON) {
                parser_error(ts, next);
            }
        } while (!peek_equals(ts, TokenType::ESAC));
        
        ts.consume();
        return case_node;
    }
    
    return nullptr;
}

//
// parse blocks, which are sequences of expressions
//

ExpressionNode* Parser::parse_block(Tokenstream& ts) {
    Token* next;

    if (peek_equals(ts, TokenType::CURLY_BRACKET_OPEN)) {
        ts.consume();

        BlockNode* block_node = new BlockNode();
        block_node->set_line_number(ts.get_line_number());

        // do-while, because at least one expression is required
        do {
            ExpressionNode* expr = parse_expression(ts);
            block_node->add_expression(expr);

            // expressions in blocks are terminated by a semicolon
            next = ts.get();
            if (!next || next->get_type() != TokenType::SEMICOLON) {
                parser_error(ts, next);
            }
        } while (!peek_equals(ts, TokenType::CURLY_BRACKET_CLOSE));

        ts.consume();
        return block_node;
    }
    
    return nullptr;
}

//
// parse let expressions
//

ExpressionNode* Parser::parse_let(Tokenstream& ts) {
    Token* next;

    if (peek_equals(ts, TokenType::LET)) {
        ts.consume();

        LetNode* let_node = new LetNode();
        let_node->set_line_number(ts.get_line_number());

        // parse let initializers
        // do-while, because at least one initializer is required
        do {           
            next = ts.get();
            if (!next || next->get_type() != TokenType::OBJ_IDENTIFIER) {
                parser_error(ts, next);
            }
            ObjIdToken* identifier = dynamic_cast<ObjIdToken*>(next);

            next = ts.get();
            if (!next || next->get_type() != TokenType::COLON) {
                parser_error(ts, next);
            }

            next = ts.get();
            if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
                parser_error(ts, next);
            }
            TypeIdToken* type = dynamic_cast<TypeIdToken*>(next);

            LetInitializerNode* init = new LetInitializerNode(identifier->get_value(), type->get_value());
            init->set_line_number(ts.get_line_number());

            // check for the optional initialization expressions
            if (peek_equals(ts, TokenType::ASSIGN)) {
                ts.consume();
                ExpressionNode* expr = parse_expression(ts);
                init->set_expr(expr);
            } else {
                ExpressionNode* no_expr = new NoExpressionNode(init->get_type());
                no_expr->set_line_number(ts.get_line_number());
                init->set_expr(no_expr);
            }

            let_node->add_initializer(init);

            next = ts.peek();
            if (!next || next->get_type() != TokenType::IN && next->get_type() != TokenType::COMMA) {
                parser_error(ts, next);
            }
        } while (ts.get()->get_type() != TokenType::IN);

        // parse let body
        ExpressionNode* body = parse_expression(ts);
        let_node->set_body(body);
        
        return let_node;
    }
    
    return nullptr;
}

//
// parse parenthesized expressions
//

ExpressionNode* Parser::parse_parentheses(Tokenstream& ts) {
    if (peek_equals(ts, TokenType::PARENTHESIS_OPEN)) {
        ts.consume();

        ExpressionNode* expr = parse_expression(ts);

        // if the parenthesized expression is an operator (like 2+2), register it
        // so operator precedence does not affect it
        if (OperationNode* operator_expr = dynamic_cast<OperationNode*>(expr)) {
            operator_expr->set_parenthesized();
        }

        Token* next = ts.get();
        if (!next || next->get_type() != TokenType::PARENTHESIS_CLOSE) {
            parser_error(ts, next);
        }

        return expr;
    }
    
    return nullptr;
}

//
// parse dispatches
//

// helper method for parsing parameters
std::vector<ExpressionNode*> Parser::parse_dispatch_parameters(Tokenstream& ts) {
    std::vector<ExpressionNode*> parameters;

    while (!peek_equals(ts, TokenType::PARENTHESIS_CLOSE)) {
        ExpressionNode* expr = parse_expression(ts);
        parameters.push_back(expr);

        if (peek_equals(ts, TokenType::PARENTHESIS_CLOSE)) {
            break;
        } 
        
        // parameters are comma-separated
        if (!peek_equals(ts, TokenType::COMMA)) {
            parser_error(ts, ts.get());
        } else {
            // if the next token is not a comma, it must be
            // a closing parenthesis, otherwise it's an error
            ts.consume();
            if (peek_equals(ts, TokenType::PARENTHESIS_CLOSE)) {
                parser_error(ts, ts.get());
            } 
        }        
    }

    // consume the closing parenthesis
    ts.consume();

    return parameters;
}

ExpressionNode* Parser::parse_dispatch(Tokenstream& ts) {
    // this dispatch is shorthand for self.method(params), 
    // so add 'self' as object
    IdentifierNode* self_obj = new IdentifierNode(Strings::Self);
    self_obj->set_line_number(ts.get_line_number());

    Token* t1 = ts.get();
    Token* t2 = ts.get();

    if (!t1) { parser_error(ts, t1); }
    if (!t2) { parser_error(ts, t2); }

    if (t1->get_type() == TokenType::OBJ_IDENTIFIER && t2->get_type() == TokenType::PARENTHESIS_OPEN) {
        ObjIdToken* method_name = dynamic_cast<ObjIdToken*>(t1);
        DispatchNode* dispatch_node = new DispatchNode(self_obj, method_name->get_value());
        dispatch_node->set_line_number(ts.get_line_number());

        for (ExpressionNode* parameter : parse_dispatch_parameters(ts)) {
            dispatch_node->add_parameter(parameter);
        }

        return dispatch_node;
    }

    ts.unget(2);
    return nullptr;
};

/*
 *  Post-parsing: check if the expression we just parsed is only
 *  the first part of a larger expression (e.g. <expr> + <expr>)
*/

ExpressionNode* Parser::post_expression(Tokenstream& ts, ExpressionNode* expr) {
    bool continue_flag = true;
    bool binary_op_flag = false;

    while (continue_flag) {
        Token* t = ts.peek();
        if (!t) { parser_error(ts, ts.get()); }
    
        OperationNode* prev_operator = dynamic_cast<OperationNode*>(expr);
        OperationNode* right_most_operation = prev_operator;
        BinaryOperationNode* node;

        // check if the previous expression is an operator
        if (prev_operator) {
            // if so, find the right-most operation in the tree
            for (;;) {
                ExpressionNode* expr = right_most_operation->get_last();
                if (OperationNode* nested_binary_operator = dynamic_cast<OperationNode*>(expr)) {
                    OperationNode* new_right_most_operation = nested_binary_operator;
                    new_right_most_operation->set_parent(right_most_operation);
                    right_most_operation = new_right_most_operation;
                } else {
                    break;
                }
            }
        }

        switch (t->get_type()) {
            // binary operators
            case TokenType::PLUS:
                node = new PlusNode();
                binary_op_flag = true;
                break;

            case TokenType::MINUS:
                node = new MinusNode();
                binary_op_flag = true;
                break;

            case TokenType::MULTIPLICATION:
                node = new MultiplicationNode();
                binary_op_flag = true;
                break;
            
            case TokenType::DIVISION:
                node = new DivisionNode();
                binary_op_flag = true;
                break;

            case TokenType::LT:
                node = new LTNode();
                binary_op_flag = true;
                break;

            case TokenType::LTE:
                node = new LTENode();
                binary_op_flag = true;
                break;

            case TokenType::EQ:
                node = new EQNode();
                binary_op_flag = true;
                break;                

            // dispatch
            case TokenType::DOT: {
                ts.consume();
                Token* next = ts.get();

                if (!next || next->get_type() != TokenType::OBJ_IDENTIFIER) {
                    parser_error(ts, next);
                }

                ObjIdToken* identifier = dynamic_cast<ObjIdToken*>(next);

                next = ts.get();
                if (!next || next->get_type() != TokenType::PARENTHESIS_OPEN) {
                    parser_error(ts, next);
                }

                if (prev_operator && !prev_operator->is_parenthesized()) {
                    // we have an expression like `a + b.dispatch`
                    // dispatches always bind tighter than operators, replace the existing right-most expression
                    // with a dispatch to that expression
                    DispatchNode* dispatch_node = new DispatchNode(right_most_operation->get_last(), identifier->get_value());
                    dispatch_node->set_line_number(ts.get_line_number());
                    
                    for (ExpressionNode* e : parse_dispatch_parameters(ts)) {
                        dispatch_node->add_parameter(e);
                    }

                    right_most_operation->set_last(dispatch_node);
                } 
                
                else {
                    // just a normal dispatch, replace the expression with a dispatch to that expression 
                    DispatchNode* dispatch_node = new DispatchNode(expr, identifier->get_value());
                    dispatch_node->set_line_number(ts.get_line_number());
    
                    for (ExpressionNode* e : parse_dispatch_parameters(ts)) {
                        dispatch_node->add_parameter(e);
                    }

                    expr = dispatch_node;
                }
                
                break;
            }
        
            // static dispatch
            case TokenType::AT: {
                ts.consume();
                Token* next = ts.get();
                if (!next || next->get_type() != TokenType::TYPE_IDENTIFIER) {
                    parser_error(ts, next);
                }

                TypeIdToken* static_type = dynamic_cast<TypeIdToken*>(next);

                next = ts.get();
                if (!next || next->get_type() != TokenType::DOT) {
                    parser_error(ts, next);
                }

                next = ts.get();
                if (!next || next->get_type() != TokenType::OBJ_IDENTIFIER) {
                    parser_error(ts, next);
                }

                ObjIdToken* identifier = dynamic_cast<ObjIdToken*>(next);

                next = ts.get();
                if (!next || next->get_type() != TokenType::PARENTHESIS_OPEN) {
                    parser_error(ts, next);
                }

                if (prev_operator && !prev_operator->is_parenthesized()) {
                    // we have an expression like `a + b.dispatch`
                    // dispatches always bind tighter than operators, replace the existing right-most expression
                    // with a dispatch to that expression
                    StaticDispatchNode* dispatch_node = new StaticDispatchNode(right_most_operation->get_last(), identifier->get_value());
                    dispatch_node->set_static_type(static_type->get_value());
                    dispatch_node->set_line_number(ts.get_line_number());
                    
                    for (ExpressionNode* e : parse_dispatch_parameters(ts)) {
                        dispatch_node->add_parameter(e);
                    }

                    right_most_operation->set_last(dispatch_node);
                } 
                
                else {
                    // just a normal dispatch, replace the expression with a dispatch to that expression 
                    StaticDispatchNode* dispatch_node = new StaticDispatchNode(expr, identifier->get_value());
                    dispatch_node->set_static_type(static_type->get_value());
                    dispatch_node->set_line_number(ts.get_line_number());

                    for (ExpressionNode* e : parse_dispatch_parameters(ts)) {
                        dispatch_node->add_parameter(e);
                    }

                    expr = dispatch_node;
                }

                break;
            }

            default:
                continue_flag = false;
                break;
        }

        if (binary_op_flag) {
            // the expression is a binary operation
            Token* t = ts.get();
            node->set_line_number(ts.get_line_number());
            ExpressionNode* next_expr = parse_single_expression(ts);
            
            // first, check if we're associating non-associative operations (e.g. a == b == c)
            if (BinaryOperationNode* prev_binary_expr = dynamic_cast<BinaryOperationNode*>(expr)) {
                if (prev_binary_expr->get_associativity() == Associativity::NONE
                    && node->get_associativity() == Associativity::NONE) {
                        parser_error(ts, t);
                    }
            }

            if (prev_operator && !prev_operator->is_parenthesized()) {
                // find the correct position in the expression tree to insert next expression
                OperationNode* operation_in_tree = right_most_operation;

                for (;;) {
                    if (node->get_precedence() < operation_in_tree->get_precedence()) {
                        // if this operator has higher precedence this one in the tree
                        node->set_first(operation_in_tree->get_last());
                        node->set_second(next_expr);
                        operation_in_tree->set_last(node);
                        break;
                    } 
                    
                    else {
                        // if this operator has lower or equal precedence than the previous
                        // note: we make the previous expression the first child of the new node
                        // in order to enforce left-associativity
                        
                        if (!operation_in_tree->get_parent()) {
                            node->set_first(expr);
                            node->set_second(next_expr);
                            expr = node;
                            break;
                        } else {
                            operation_in_tree = operation_in_tree->get_parent();
                        }    
                    }
                }
            }

            else if (dynamic_cast<DispatchNode*>(expr)) {
                // if previous expression is a dispatch
                // example: a.dispatch() + b
                node->set_first(expr);
                node->set_second(next_expr);
                expr = node;
            } 
            
            else {
                // if it's just an expression (like 123 or abc)
                node->set_first(expr);
                node->set_second(next_expr);
                expr = node;
            }
        }

        binary_op_flag = false;
    }

    return expr;
}