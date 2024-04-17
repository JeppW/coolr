#include "ast.h"

/*
 *  Display methods for dumping the nodes of the abstract syntax tree.
 *
 *  The display methods follow the syntax used in the Stanford course
 *  so that this compiler can be tested using the grading tests from the course.
 */

void ProgramNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_program" << std::endl;
    for (ClassNode* cls : get_classes()) {
        cls->dump(spaces + 2);
    }
}

void ClassNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_class" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_base_class() << std::endl;
    
    std::cout << std::string(spaces + 2, ' ') << '(' << std::endl;

    for (FeatureNode* feature : get_features()) {
        feature->dump(spaces + 2);
    }

    std::cout << std::string(spaces + 2, ' ') << ')' << std::endl;
}

void AttributeNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_attr" << std::endl; 
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_type() << std::endl;
    get_expr()->dump(spaces + 2);
}

void MethodNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_method" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    get_formals()->dump(spaces + 2);
    std::cout << std::string(spaces + 2, ' ') << get_type() << std::endl;
    get_expr()->dump(spaces + 2);
}

void FormalsNode::dump(uint spaces) {
    for (FormalNode* formal : get_formals()) {
        formal->dump(spaces);
    }
}

void FormalNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_formal" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl; 
    std::cout << std::string(spaces + 2, ' ') << get_type() << std::endl; 
}

void NoExpressionNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_no_expr" << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void IntNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_int" << std::endl; 
    std::cout << std::string(spaces + 2, ' ') << get_value() << std::endl; 
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void StringNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_string" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_pretty_string(get_value()) << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void BoolNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_bool" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_value() << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void IdentifierNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_object" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void AssignmentNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_assign" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    get_expr()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void NewNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_new" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_type() << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void IsvoidNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_isvoid" << std::endl;
    get_expr()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void PlusNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_plus" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void MinusNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_sub" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void MultiplicationNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_mul" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void DivisionNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_divide" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void LTNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_lt" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void LTENode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_leq" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void EQNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_eq" << std::endl;
    get_first()->dump(spaces + 2);
    get_second()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void NegNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_neg" << std::endl;
    get_expr()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void ComplementNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_comp" << std::endl;
    get_expr()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void ConditionalNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_cond" << std::endl;
    get_predicate()->dump(spaces + 2);
    get_then()->dump(spaces + 2);
    get_else()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void WhileNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_loop" << std::endl;
    get_predicate()->dump(spaces + 2);
    get_body()->dump(spaces + 2);
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void BlockNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_block" << std::endl;
    for (ExpressionNode* expr : get_expressions()) {
        expr->dump(spaces + 2);
    }
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void LetNode::dump(uint spaces) {
    // the unit tests from stanford represent let cases with
    // multiple initializers in a kind of weird way -
    // all initializers after the first one are written as a nested let expression
    // - that's why this function is so complicated
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_let" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_initializers().front()->get_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_initializers().front()->get_type() << std::endl;
    get_initializers().front()->get_expr()->dump(spaces + 2);

    uint tmp_space = 2;
    std::vector<LetInitializerNode*> initializers = get_initializers();
    for (size_t i = 1; i < initializers.size(); ++i) {
        initializers[i]->dump(spaces + tmp_space);
        tmp_space += 2;
    }

    body->dump(spaces + tmp_space);

    for (size_t i = initializers.size() - 1; i > 0; --i) {
        tmp_space -= 2;
        std::cout << std::string(spaces + tmp_space, ' ') << ": " << initializers[i]->get_checked_type() << std::endl;
    }

    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void LetInitializerNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_let" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_type() << std::endl;
    get_expr()->dump(spaces + 2);
}

void CaseNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_typcase" << std::endl;
    get_target()->dump(spaces + 2);
    for (CaseBranchNode* branch : get_branches()) {
        branch->dump(spaces + 2);
    }
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void CaseBranchNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_branch" << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_type() << std::endl;
    get_expr()->dump(spaces + 2);
}

void DispatchNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_dispatch" << std::endl;
    get_object()->dump(spaces + 2);
    std::cout << std::string(spaces + 2, ' ') << get_method_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << '(' << std::endl;

    for (ExpressionNode* parameter : get_parameters()) {
        parameter->dump(spaces + 2);
    }

    std::cout << std::string(spaces + 2, ' ') << ')' << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}

void StaticDispatchNode::dump(uint spaces) {
    std::cout << std::string(spaces, ' ') << '#' << get_line_number() << std::endl;
    std::cout << std::string(spaces, ' ') << "_static_dispatch" << std::endl;
    get_object()->dump(spaces + 2);
    std::cout << std::string(spaces + 2, ' ') << get_static_type() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << get_method_name() << std::endl;
    std::cout << std::string(spaces + 2, ' ') << '(' << std::endl;

    for (ExpressionNode* parameter : get_parameters()) {
        parameter->dump(spaces + 2);
    }

    std::cout << std::string(spaces + 2, ' ') << ')' << std::endl;
    std::cout << std::string(spaces, ' ') << ": " << get_checked_type() << std::endl;
}