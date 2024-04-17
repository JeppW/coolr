# Lexical analyzer
The lexical analysis module converts the source COOL program into a stream of tokens to be used by the parser module.

The lexer simply reads the input from start to finish, identifying valid tokens using regular expressions, and appends them to the output token stream. In the event that the current position in the file stream matches multiple regexes, the longest one is used.

The lexical analyzer utilizes a state machine approach, where specific characters and tokens trigger state transitions. For example, when a `"` character is encountered, the lexer switches to the `STRING` state and starts constructing a string token, and when an escape character is encountered while the lexer is in the `STRING`, it transitions to the `ESCAPED_STRING` state. Once a new `"` character is encountered while in the `STRING` state, the lexer transitions back to the `DEFAULT` state and continues parsing tokens normally.

## Nested comments
COOL supports a deeply strange feature that is worth mentioning here: *nested multi-line comments*. A multi-line comments embedded within another multi-line comment must be closed separately from the outer comment. Confusingly, that means that the following code snippet is actually invalid.

```
(*
    In COOL, the asterix symbol (*) is used for multiplication.
*)
```

The reason that the snippet is invalid is that the `(*` within the comment signifies the start of a new nested comment which must also be closed.

Lexical analyzers usually do not have to handle nested structures like this, and they are somewhat problematic, as they cannot be represented with a pure state machine. In this compiler, a simple work-around is applied: while in the `MULTILINE_COMMENT` state, the lexer maintains an internal counter variable that keeps track of the comment depth and only transitions back to the `DEFAULT` state once the comment depth reaches zero.


## Changes in grading tests
The comment in the first line of all tests has been removed.