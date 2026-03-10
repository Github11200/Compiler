# Verbosity

A compiler for my own programming language:

```
define sayX with x of type integer gives back void as
  say x stop
stop

let a of type integer be 10 plus 5 times 20 minus 2 stop

call sayX with a stop
```

## Features

- Variables
- Functions
- Pointers
- Expression parsing
- Loops

## How it works

### Step 1. Lexing

First, all the tokens are split based on a list of delimeters. Once they're split, each token is assigned a `TokenType` from `token.h`. All of these lexed tokens are then passed onto the next step, parsing.

### Step 2. Parsing

With each token given a `TokenType`, I start constructing the abstract syntax tree (AST). The AST for the following variable statement is shown below:

`let a of type integer be 5 plus 10 divide 2 stop`

```
Variable Statement
|
|__ Identifier - "a"
|
|__ Variable type - Integer
|
|__ Value - Binary Expression
    |
    |__ Operator - Plus
    |
    |__ Left - Integer literal
        |
        |___ Value - 5
    |
    |__ Right - Binary Expression
        |
        |__ Operator - Divide
        |
        |__ Left - Integer Literal
            |
            |__ Value - 10
        |
        |__ Right - Integer Literal
            |
            |__ Value - 2

```

As you can see, this strucutre is fairly recursive as a `BinaryExpression` can contain other binary expressions inside of it. This same process is applied to all the code and various types of nodes are created which are found in `node.h`. The implementions of the functions for creating each of these nodes is found in `ast.cpp`.

### Step 3. Code Generation

With the AST created all we have to do is just loop through each node and call the `generateCode` method attached to it. Since every node is a child class of the abstract class `ASTNode` (shown below) we can call `generateCode` on all of them without worrying about the type:

```C++
struct ASTNode {
  virtual ~ASTNode() = default;
  virtual std::string generateCode() = 0;
};
```

All of this code is then writen into a `compiled.cpp` file and then a command is run in the terminal to build and run this code.

## Questions/Feedback

Feel free to reach out to me via any of the contact methods below:

- [Email](mailto:jinayunity22@gmail.com)
- [Linkedin](https://www.linkedin.com/in/jinay-patel-6369002b4/)
