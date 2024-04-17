# Parsing
The purpose of the parser module is to build an abstract syntax tree (AST) representation from the token stream provided by the lexical analysis module. 

This parser employs a recursive descent parsing strategy. For each feature in each class, the parser parses the method or attribute using a set of methods corresponding to each rule in the COOL grammar, recursively diving into subexpressions as they are encountered. 

## Error feedback
Many compilers attempt to recover after encountering syntax errors in the source program, enabling them to continue parsing and potentially inform the user of multiple errors during a single compilation. In my experience, this feature often is more confusing than it is useful, as subsequent errors are commonly a result of the first one, and no one likes it when gcc unleashes a flood of intimidating error messages onto their terminal screen. For this reason, I decided to exclude this feature. 

This somewhat controversial design choice has the added benefit of significantly simplifying the parser, as it allows it to skip panic recovery and simply exit upon the first syntax error.

## Operator precedence
The parser is responsible for handling operator precedence, ensuring that the AST correctly reflects the order in which operations should be executed. The precedence of COOL operators is given below in descending order of precedence.
```
0: .
1: @
2: ~
3: isvoid
4: * /
5: + -
6: <= < =
7: not
```

Note that as in most languages, precedence can be controlled using parentheses.

The way that this parser implements operator precedence is perhaps best explained with an example. Consider the expression `not 1 * 2 + 3 < 4`. Here, the `not` keyword is encountered first, and the subsequent expression is parsed, yielding the parse tree:

```
_not
  Int: 1
```

Then, the `*` operator is encountered and the next immediate expression is parsed. Since multiplication binds tighter that boolean complement according to the table above, the multiplication node is inserted at the bottom of the tree, using the child of the existing `_not` node as the left-most child and the next expression (in this case `2`) as the right-most child.

```
_not
  _multiplication
    Int: 1
    Int: 2
```
Then, the `+` operator is encountered. Addition has a lower precedence than multiplication, so we move one step up the tree. However, it has a higher precedence than boolean complement, so we insert it inbetween the `_not` and `_multiplication` nodes.

```
_not
  _plus
    _multiplication
      Int: 1
      Int: 2
    Int: 3
```

Similarly, the `<` operator has a lower precedence than multiplication and addition, but a higher precedence than boolean complement.

```
_not
  _lt
    _plus
      _multiplication
        Int: 1
        Int: 2
      Int: 3
    Int: 4
```

This procedure produces an abstract syntax tree that correctly accounts for operator precedence. Once this step is completed, subsequent compiler phases do not need to consider parenthesized expressions or operator precedence, because the parse tree already encodes the order of operations.

## Changes in grading tests
To adapt the StanfordOnline grading tests to this parser, the following adjustments were made to parser tests.

- The 'filename' field from the classes were removed. This was filtered in the original grading script anyway and just seems unnecessary to me. 
- As described above, this parser halts at the first syntax error. The grading tests have been updated to reflect this design choice; all error messages after the first one have been removed.
