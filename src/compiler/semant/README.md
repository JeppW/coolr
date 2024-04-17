# Semantic analysis
The semantic analyzer is the final validity check of the program. It performs type-checking, it validates the inheritance tree of the program, and it enforces the semantic rules of the COOL language. The semantic analyzer operates on the abstract syntax tree constructed by the parser module and produces an type-annotated abstract syntax tree, provided that no semantic errors are found.

The semantic analyzer starts by building a *class table*, a data structure containing all the classes of the source program. This process includes verifying that the classes follow the rules of the COOL language, including:

- that a `Main` class with a `main` method exists
- that COOL basic classes are not overwritten or inherited by any classes
- that there are no cycles in the inheritance tree

Then, a global method environment is constructed using the methods of all the classes in the class table. Finally, each class is parsed individually by building an object environment from the class attributes and parsing the features of the class. Every time an expression is type-checked without causing a type violation, the inferred type of the expression is registered on the abstract syntax tree.

## Type checking
Type-checking is the process of determining (*inferring*) the types of the expressions in the program in accordance with the type rules of COOL and verifying that the inferred types match the types declared by the programmer. As an example, consider the attribute below.

```
attr : String <- 2+2;
```

As described in the type rules of the COOL language (see the manual), all arithmetic expressions have type `Int` and are defined for operands also of type `Int`. As such, when performing type-checking on this attribute expression, the semantic analyzer first verifies that each of the operands of the addition expression are `Int` objects. Since this is the case, the addition expression is valid, and the entire expression of the attribute has type `Int`. However, this inferred type does not match the declared type `String`, and therefore the compiler throws an error.

Since COOL supports inheritance, the semantic analyzer does not check if the inferred types exactly match the declared types, but rather if the inferred types *conform* to the declared types. For example, if a method accepts a parameter of type `A`, a parameter of type `B` may be supplied instead if class `B` inherits directly or indirectly from class `A`.

## self and SELF_TYPE
The COOL language, being an object-oriented language, supports the notion of a `self` object and a `SELF_TYPE` type, which complicates type-checking, in particular in regards to verifying whether inferred types conform to declared types. This semantic analyzer handles this problem by maintaining the current class as part of the type environment, such that objects of `SELF_TYPE` can be resolved to an absolute type that can be compared to other types when needed.

However, it should be noted that `SELF_TYPE` can not always be resolved by consulting the current class in the type environment. Specifically, when a dispatch to an instance of another class returns `SELF_TYPE`, it refers to the type of the dispatchee, not the dispatcher, and must be resolved accordingly.

## Changes in grading tests
The same adjustments were made to the semantic analysis tests as to the parser tests.