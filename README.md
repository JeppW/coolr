# coolr 
`coolr` is a compiler for the COOL programming language targeting 32-bit x86 architecture on Linux. It is written in C++ using only standard libraries - no lexing or parsing generators are used.

## Why?
After completing the [*Compilers* course from StanfordOnline](https://www.edx.org/learn/computer-science/stanford-university-compilers), I had produced my very first working compiler. While I was quite happy with this achievement, I felt a lingering dissatisfaction.

- I didn't like relying on Flex, Bison, and the instructor-provided code. I wanted to understand every detail of the inner workings of my compiler. 
- I didn't like the whole `./lexer | ./parser | ./semant | ./codegen` thing. I wanted a single executable.
- I didn't like using the SPIM emulator to execute my compiled programs. I wanted my compiler to produce x86 code I could actually run directly on my own machine and debug with gdb. 

To this end, I decided to `rm -rf` my existing course project and start from scratch - no Flex, no Bison, no support code and no SPIM. This repository contains the result of this venture.

## About COOL
COOL (*Classroom Object-Oriented Language*) is a programming language designed for teaching compiler construction. For a language designed as a teaching tool, it is actually fairly complex and includes features such as:

- Object-oriented features: Inheritance, method overriding, static and dynamic dispatches.
- Type features: Static typing, type safety and support for dynamic type checking. 
- Run-time error handling: Clean exit on run-time errors - no segfaults!

A precise and formal description of the language is given in the COOL manual by Alex Aiken. I've included the manual in this repository for reference.

## Installation and usage
To try out the compiler, simply clone the repository and build the project. This requires C++17 or later.

```
$ git clone https://github.com/JeppW/coolr
$ cd coolr
$ make coolr
```

Once completed, you can compile a program using `./coolr filename.cl`. A few examples have been provided in `examples/`. This will output a NASM file which you can assemble into a working binary with the provided `assemble.sh` script.

```
$ ./coolr examples/hello_world.cl --out out.S && scripts/assemble.sh out.S myprogram
$ ./myprogram
Hello, world!
```

Naturally, this will only work on machines that support 32-bit x86 architecture.

## Testing and grading
The grading test cases from the StanfordOnline Compilers course have been used to test this compiler. Some of them have been slightly altered to reflect the changes I've introduced along the way. Where relevant, this has been described in the README files of the compiler modules in the `src/compiler` directory.

__All tests are currently passing__. You can run the tests yourself by navigating to the subdirectories of the `tests/` directory and executing the `test.sh` scripts.


## Looking for more details?
This compiler is divided into four modules: a lexical analyzer, a parser, a semantic analyzer and a code generator. The inner workings of each of the four modules are described in detail in the README files in their respective subfolders in `src/compiler`. 

## Issues
As it turns out, making compilers is pretty complicated. I have fixed a number of obscure bugs and I would expect more still persist. If you decide to give this compiler a spin and encounter a problem, I would love to know about it. 


