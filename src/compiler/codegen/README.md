# Code generation
This code generation module uses the annotated abstract syntax tree to generate x86 assembly language code that implements the program. This is by far the most complex module and the one that allows the most creative freedom.

This code generator iterates over the program in multiple passes. First, the compiler constructs the `.data` section by building the object prototypes and dispatch tables for all classes as well as the static strings used in the program. Then, the `.text` section is constructed by generating code for each method of each class. 

A few internal methods are defined by the compiler in addition to the user code. These are all prefixed by an underscore in order to separate them from user code, as class and method names cannot start with an underscore in COOL. At the `_start` entrypoint, the code generator initializes the `Main` class, calls its `main` method and then finally jumps to an `_exit` label which cleanly exits the process.

## Object layout
In this COOL implementation, objects consist of 5 headers followed by the object attributes.

```
Offset 0:  Class tag
Offset 4:  Type name
Offset 8:  Object size
Offset 12: Dispatch table pointer
Offset 16: Parent class prototype pointer
... attributes ...
``` 

The __class tag__ header is used in dynamic type checking. It contains a unique identifier used to determine the type of the object at run-time within case expressions.

The __type name__ header is simply a pointer to a static String object that contains the name of the class. It is used in the implementation of the built-in `type_name()` method of `Object`. 

The total size of the object is encoded in the __object size__ header, including both headers and attributes. This header is required during object instantiation, as it describes how much memory should be dynamically allocated for the object.

The __dispatch table pointer__ is a pointer to the dispatch table of the class. This is the dispatch table that is used when the methods of the objects are called. 

Objects contain a pointer to the prototype of their parent class in the __parent class prototype pointer__ header. Case expressions use this header to identify the correct branch to take by repeatedly resolving the parent of the target object.

## Calling conventions
Parameters are passed on the stack in order, such that the first parameter is at the bottom position in the stack. Note that this is different from cdecl in which parameters are passed in reverse order. 

```
esp -> ...
       ... stack variables ...
       ...
ebp -> control link
       return address
       argument 3
       argument 2
       argument 1
       ... previous stack frame ...
```

The callee cleans up method arguments from the stack after the method has been executed. The return value is passed in the eax register.

## Object initialization
When a new object is initialized with the `new` keyword, its size is read from the size header of the object prototype, and a memory chunk of that size is allocated. The object prototype is copied into the newly allocated memory.

Then, the object attributes is initialized using the initialization expressions defined by the user. This is done by calling an internal `_init` method located at the top of the dispatch tables of all objects. 

## I/O
The compiled program reads from `stdin` and writes to `stdout` using 32-bit Linux system calls. For parsing input, the compiler statically allocates an input buffer with enough space for a string of the maximum allowed length. User input is temporarily placed in this buffer when the `in_out` or `in_string` methods are called. Once an appropriately-sized chunk of memory has been dynamically allocated, the contents of the buffer is copied to the heap.

## Garbage collection
COOL supports automatic memory management and garbage collection. However, a garbage collector has not yet been implemented, as doing so was not part of the course. Currently, the program simply exists with an error message once it runs out of memory. I hope to return to the project in the future and implement a garbage collector when time allows.

## Changes in grading tests
The following changes were made to the grading tests to make them compatible with this compiler.
- Since this project doesn't use SPIM, the SPIM banner and various other SPIM messages have been removed from the test cases.
- Since a garbage collector has not yet been implemented, the two tests pertaining to garbage collection did not make much sense and have therefore been removed (although both compiled and executed correctly).


