#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include "../common/consts.h"
#include "../utils/pretty_print.h"

/*
 *  Abstract syntax tree.
 *  This file defines the types of nodes and their relationships.
 * 
 *  Implementing the 'typecheck' and 'code' methods is the task
 *  of the semantic analysis and code generation modules, respectively.
 */

enum NodeType {
    ProgramNodeType,
    ClassNodeType,
    AttributeNodeType,
    MethodNodeType,
    FormalsNodeType,
    FormalNodeType,
    ExpressionNodeType,
    NoExpressionNodeType,
    IntNodeType,
    StringNodeType,
    BoolNodeType,
    IdentifierNodeType,
    AssignmentNodeType,
    NewNodeType,
    IsvoidNodeType,
    PlusNodeType,
    MinusNodeType,
    MultiplicationNodeType,
    DivisionNodeType,
    LTNodeType,
    LTENodeType,
    EQNodeType,
    NegNodeType,
    ComplementNodeType,
    ConditionalNodeType,
    WhileNodeType,
    BlockNodeType,
    LetNodeType,
    LetInitializerType,
    CaseNodeType,
    CaseBranchType,
    DispatchNodeType,
    StaticDispatchNodeType
};

// forward declarations
class ClassTable;
class TypeEnvironment;

enum Associativity {
    LEFT,
    RIGHT,
    NONE
};

class Node {
    private:
        NodeType type;
        uint line_number = 0;

    public:
        Node(NodeType t) {
            type = t;
        }

        void set_line_number(uint ln) {
            line_number = ln;
        }

        uint get_line_number() {
            return line_number;
        }

        virtual void dump(uint) = 0;
};

class ExpressionNode : public Node {
    private:
        std::string checked_type = Strings::Types::NoType;

    public:
        ExpressionNode(NodeType t) : Node(t) {}

        std::string get_checked_type() {
            return checked_type;
        }

        void set_checked_type(const std::string& ct) {
            checked_type = ct;
        }

        virtual std::string typecheck(TypeEnvironment&) = 0;
        virtual void code() = 0;
};

class OperationNode : public ExpressionNode {
    private:
        uint precedence;
        OperationNode* parent = 0;
        bool parenthesized = false;
    
    public:
        OperationNode(NodeType t, uint p) : ExpressionNode(t), precedence(p) {};
    
        uint get_precedence() {
            return precedence;
        }

        OperationNode* get_parent() {
            return parent;
        }

        void set_parent(OperationNode* p) {
            parent = p;
        }

        bool is_parenthesized() {
            return parenthesized;
        }

        void set_parenthesized() {
            parenthesized = true;
        }

        virtual ExpressionNode* get_last() = 0;
        virtual void set_last(ExpressionNode*) = 0;
};

class UnaryOperationNode : public OperationNode {
    private:
        ExpressionNode* expr;

    public:
        UnaryOperationNode(NodeType t, uint p) : OperationNode(t, p) {}

        void set_expr(ExpressionNode* e) {
            expr = e;
        }; 

        ExpressionNode* get_expr() {
            return expr;
        }

        ExpressionNode* get_last() {
            return get_expr();
        }

        void set_last(ExpressionNode* e) {
            set_expr(e);
        }
};

class BinaryOperationNode : public OperationNode {
    private:
        ExpressionNode* expr1;
        ExpressionNode* expr2;
        Associativity assoc;

    public:
        BinaryOperationNode(NodeType t, uint p, Associativity a) : OperationNode(t, p), assoc(a) {}

        void set_first(ExpressionNode* e) {
            expr1 = e;
        }; 

        void set_second(ExpressionNode* e) {
            expr2 = e;
        }

        ExpressionNode* get_first() {
            return expr1;
        }

        ExpressionNode* get_second() {
            return expr2;
        }

        Associativity get_associativity() {
            return assoc;
        }

        ExpressionNode* get_last() {
            return get_second();
        }

        void set_last(ExpressionNode* e) {
            set_second(e);
        }
};

class NoExpressionNode : public ExpressionNode {
    private:
        std::string declared_type;
    
    public:
        NoExpressionNode(const std::string& t) : ExpressionNode(NodeType::NoExpressionNodeType), declared_type(t) {}

        std::string get_declared_type() {
            return declared_type;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class IntNode : public ExpressionNode {
    private:
        std::string value;
    
    public:
        IntNode(const std::string& v) : ExpressionNode(NodeType::IntNodeType), value(v) {}

        std::string get_value() {
            return value;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class StringNode : public ExpressionNode {
    private:
        std::string value;
    
    public:
        StringNode(const std::string& v) : ExpressionNode(NodeType::StringNodeType), value(v) {}

        std::string get_value() {
            return value;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class BoolNode : public ExpressionNode {
    private:
        bool value;
    
    public:
        BoolNode(bool v) : ExpressionNode(NodeType::BoolNodeType), value(v) {}

        bool get_value() {
            return value;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class IdentifierNode : public ExpressionNode {
    private:
        std::string name;
    
    public:
        IdentifierNode(const std::string& n) : ExpressionNode(NodeType::IdentifierNodeType), name(n) {}

        std::string get_name() {
            return name;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class AssignmentNode : public ExpressionNode {
    private:
        std::string name;
        ExpressionNode* expr;
    
    public:
        AssignmentNode(const std::string& n) : ExpressionNode(NodeType::AssignmentNodeType), name(n) {}

        std::string get_name() {
            return name;
        }

        void set_expr(ExpressionNode* e) {
            expr = e;
        };

        ExpressionNode* get_expr() {
            return expr;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class NewNode : public ExpressionNode {
    private:
        std::string type;
    
    public:
        NewNode(const std::string& t) : ExpressionNode(NodeType::NewNodeType), type(t) {}

        std::string get_type() {
            return type;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class IsvoidNode : public UnaryOperationNode {   
    public:
        IsvoidNode() : UnaryOperationNode(NodeType::IsvoidNodeType, 3) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class NegNode : public UnaryOperationNode {   
    public:
        NegNode() : UnaryOperationNode(NodeType::NegNodeType, 2) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class ComplementNode : public UnaryOperationNode {
    public:
        ComplementNode() : UnaryOperationNode(NodeType::ComplementNodeType, 7) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class PlusNode : public BinaryOperationNode {
    public:
        PlusNode() : BinaryOperationNode(NodeType::PlusNodeType, 5, Associativity::LEFT) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class MinusNode : public BinaryOperationNode {
    public:
        MinusNode() : BinaryOperationNode(NodeType::MinusNodeType, 5, Associativity::LEFT) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class MultiplicationNode : public BinaryOperationNode {
    public:
        MultiplicationNode() : BinaryOperationNode(NodeType::MultiplicationNodeType, 4, Associativity::LEFT) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class DivisionNode : public BinaryOperationNode {
    public:
        DivisionNode() : BinaryOperationNode(NodeType::DivisionNodeType, 4, Associativity::LEFT) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class LTNode : public BinaryOperationNode {
    public:
        LTNode() : BinaryOperationNode(NodeType::LTNodeType, 6, Associativity::NONE) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class LTENode : public BinaryOperationNode {
    public:
        LTENode() : BinaryOperationNode(NodeType::LTENodeType, 6, Associativity::NONE) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class EQNode : public BinaryOperationNode {
    public:
        EQNode() : BinaryOperationNode(NodeType::EQNodeType, 6, Associativity::NONE) {}
        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class ConditionalNode : public ExpressionNode {
    private:
        ExpressionNode* predicate_expr;
        ExpressionNode* then_expr;
        ExpressionNode* else_expr;
    
    public:
        ConditionalNode() : ExpressionNode(NodeType::ConditionalNodeType) {}

        void set_predicate(ExpressionNode* e) {
            predicate_expr = e;
        };

        void set_then(ExpressionNode* e) {
            then_expr = e;
        };

        void set_else(ExpressionNode* e) {
            else_expr = e;
        };        

        ExpressionNode* get_predicate() {
            return predicate_expr;
        }

        ExpressionNode* get_then() {
            return then_expr;
        }

        ExpressionNode* get_else() {
            return else_expr;
        }                

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class WhileNode : public ExpressionNode {
    private:
        ExpressionNode* predicate;
        ExpressionNode* body;
    
    public:
        WhileNode() : ExpressionNode(NodeType::WhileNodeType) {}

        void set_predicate(ExpressionNode* e) {
            predicate = e;
        };

        void set_body(ExpressionNode* e) {
            body = e;
        };        

        ExpressionNode* get_predicate() {
            return predicate;
        }

        ExpressionNode* get_body() {
            return body;
        }        

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class BlockNode : public ExpressionNode {
    private:
        std::vector<ExpressionNode*> expressions;
    
    public:
        BlockNode() : ExpressionNode(NodeType::BlockNodeType) {}

        void add_expression(ExpressionNode* e) {
            expressions.push_back(e);
        }

        std::vector<ExpressionNode*> get_expressions() {
            return expressions;
        }    

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class LetInitializerNode : public ExpressionNode {
    private:
        std::string name;
        std::string type;
        ExpressionNode* expr;
    
    public:
        LetInitializerNode(const std::string& n, const std::string& t) : ExpressionNode(NodeType::LetInitializerType), name(n), type(t) {}

        std::string get_name() {
            return name;
        }

        std::string get_type() {
            return type;
        }

        ExpressionNode* get_expr() {
            return expr;
        }

        void set_expr(ExpressionNode* e) {
            expr = e;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class LetNode : public ExpressionNode {
    private:
        std::vector<LetInitializerNode*> initializers;
        ExpressionNode* body;
    
    public:
        LetNode() : ExpressionNode(NodeType::LetNodeType) {}

        void set_body(ExpressionNode* e) {
            body = e;
        };        

        ExpressionNode* get_body() {
            return body;
        } 

        void add_initializer(LetInitializerNode* i) {
            initializers.push_back(i);
        }

        std::vector<LetInitializerNode*> get_initializers() {
            return initializers;
        }    

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class CaseBranchNode : public Node {
    private:
        std::string name;
        std::string type;
        ExpressionNode* expr;

    public:
        CaseBranchNode(const std::string& n, const std::string& t) : Node(NodeType::CaseBranchType), name(n), type(t) {}

        std::string get_name() {
            return name;
        }

        std::string get_type() {
            return type;
        }

        void set_expr(ExpressionNode* e) {
            expr = e;
        }

        ExpressionNode* get_expr() {
            return expr;
        }

        void dump(uint) override;
};

class CaseNode : public ExpressionNode {
    private:
        ExpressionNode* target; // called expr0 in the COOL manual
        std::vector<CaseBranchNode*> branches;
    
    public:
        CaseNode() : ExpressionNode(NodeType::CaseNodeType) {}

        void set_target(ExpressionNode* t) {
            target = t;
        }

        ExpressionNode* get_target() {
            return target;
        }

        void add_branch(CaseBranchNode* b) {
            branches.push_back(b);
        }

        std::vector<CaseBranchNode*> get_branches() {
            return branches;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class DispatchNode : public ExpressionNode {
    private:
        ExpressionNode* object;
        std::string method_name;
        std::vector<ExpressionNode*> parameters;

    public:
        DispatchNode(ExpressionNode* o, const std::string& m) : ExpressionNode(NodeType::DispatchNodeType), object(o), method_name(m) {}

        ExpressionNode* get_object() {
            return object;
        }

        std::string get_method_name() {
            return method_name;
        }

        void add_parameter(ExpressionNode* e) {
            parameters.push_back(e);
        }

        std::vector<ExpressionNode*> get_parameters() {
            return parameters;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class StaticDispatchNode : public ExpressionNode {
    private:
        ExpressionNode* object;
        std::string method_name;
        std::string static_type = "";
        std::vector<ExpressionNode*> parameters;

    public:
        StaticDispatchNode(ExpressionNode* o, const std::string& m) : ExpressionNode(NodeType::DispatchNodeType), object(o), method_name(m) {}

        ExpressionNode* get_object() {
            return object;
        }

        std::string get_method_name() {
            return method_name;
        }

        std::string get_static_type() {
            return static_type;
        }

        void set_static_type(const std::string& st) {
            static_type = st;
        }

        void add_parameter(ExpressionNode* e) {
            parameters.push_back(e);
        }

        std::vector<ExpressionNode*> get_parameters() {
            return parameters;
        }

        void dump(uint) override;
        std::string typecheck(TypeEnvironment&) override;
        void code() override;
};

class FeatureNode : public Node {
    private:
        std::string name;
        std::string type;
        ExpressionNode* expr;
    
    public:
        FeatureNode(NodeType t, const std::string& n) : Node(t), name(n) {}

        std::string get_name() {
            return name;
        }

        std::string get_type() {
            return type;
        }

        void set_type(const std::string& t) {
            type = t;
        }

        void set_expr(ExpressionNode* e) {
            expr = e;
        };

        ExpressionNode* get_expr() {
            return expr;
        }

        virtual void analyze(TypeEnvironment&) = 0;
};

class AttributeNode : public FeatureNode {   
    public:
        AttributeNode(const std::string& name) : FeatureNode(NodeType::AttributeNodeType, name) {}
        void dump(uint) override;
        void analyze(TypeEnvironment&) override;
};

class FormalNode : public Node {
    private:
        std::string name;
        std::string type;
    
    public:
        FormalNode(const std::string& n, const std::string& t) : Node(NodeType::FormalNodeType), name(n), type(t) {}

        std::string get_name() {
            return name;
        }

        std::string get_type() {
            return type;
        }

        void dump(uint) override;
};

class FormalsNode : public Node {
    private:
        std::vector<FormalNode*> formals;
    
    public:
        FormalsNode() : Node(NodeType::FormalNodeType) {}

        std::vector<FormalNode*> get_formals() {
            return formals;
        }

        void add_formal(FormalNode* formal) {
            formals.push_back(formal);
        }

        size_t length() {
            return formals.size();
        }

        void dump(uint) override;
};

class MethodNode : public FeatureNode {
    private:
        FormalsNode* formals;

    public:
        MethodNode(const std::string& name) : FeatureNode(NodeType::MethodNodeType, name) {}

        FormalsNode* get_formals() {
            return formals;
        }

        void set_formals(FormalsNode* f) {
            formals = f;
        }

        void dump(uint) override;
        void analyze(TypeEnvironment&) override;
};

class ClassNode : public Node {
    private:
        std::string name;
        std::string base_class;
        std::vector<FeatureNode*> features;

        // also track attributes and methods separately for convenience
        std::vector<AttributeNode*> attributes;
        std::vector<MethodNode*> methods;
    
    public:
        ClassNode(const std::string& n) : Node(NodeType::ClassNodeType), name(n) {}

        std::string get_name() {
            return name;
        }

        std::string get_base_class() {
            return base_class;
        }

        std::vector<FeatureNode*> get_features() {
            return features;
        }

        std::vector<AttributeNode*> get_attributes() {
            return attributes;
        }

        std::vector<MethodNode*> get_methods() {
            return methods;
        }

        void add_attribute(AttributeNode* attribute) {
            features.push_back(attribute);
            attributes.push_back(attribute);
        }

        void add_method(MethodNode* method) {
            features.push_back(method);
            methods.push_back(method);
        }

        void set_base_class(const std::string& bc) {
            base_class = bc;
        }

        void dump(uint) override;
        void analyze(TypeEnvironment&);
};

class ProgramNode : public Node {
    private:
        std::vector<ClassNode*> classes;
    
    public:
        ProgramNode() : Node(NodeType::ProgramNodeType) {}

        std::vector<ClassNode*> get_classes() {
            return classes;
        }

        void add_class(ClassNode* cls) {
            classes.push_back(cls);
        }

        void dump(uint) override;
        ClassTable* analyze();
};

#endif