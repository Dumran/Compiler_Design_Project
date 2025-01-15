# Mini Parser Project

## 📚 About the Project
This project implements a simple **recursive descent parser** that analyzes and executes a basic programming language. It takes a text-based input, tokenizes it, checks it against grammar rules, and executes conditional statements, loops, and mathematical expressions. The project aims to provide a foundational understanding of language processing and interpreter design.

## 🚀 Features
- **Tokenization**: The input text is split into meaningful tokens.
- **Grammar Parsing**: Checks the input against predefined grammar rules using a recursive descent method.
- **Conditional Statements (IF)**: Supports conditional blocks.
- **Loops (WHILE)**: Executes loops based on a condition.
- **Mathematical Expressions**: Supports addition, subtraction, multiplication, division, and exponentiation.

## 🛠️ Technologies Used
- **C Programming Language**: The main development language.
- **Lexer and Parser Structure**: Custom algorithms for tokenization and grammar parsing.

## 📋 Grammar Rules
The project follows the grammar rules listed below:
- **P** → { C } '.'
- **C** → I | W | A | Ç | G
- **I** → '[' E '?' C{C} ':' C{C} ']'
- **W** → '{' E '?' C{C} '}'
- **A** → K '=' E ';'
- **Ç** → '<' E ';'
- **G** → '>' K ';'
- **E** → T {('+' | '-') T}
- **T** → U {('*' | '/' | '%') U}
- **U** → F '^' U | F
- **F** → '(' E ')' | K | R

## 📝 Example Program
Below is an example program that can be executed using this interpreter:
```plaintext
n = 0;
{ n - 2*5 ?
  < n;
  n = n + 1;
}
.
```
### Expected Output:
```plaintext
0
1
2
3
4
5
6
7
8
9
```
In this program, the variable `n` starts at 0, and the loop continues until `n - 10` becomes zero. At each step, `n` is printed to the screen, and `n` is incremented by 1.

## 📂 Project Structure
- **main.c**: The main C file. Contains the lexer, parser, and interpreter.
- **README.md**: This documentation file.

## 🖥️ How to Run
This project can be compiled and run using any C compiler. Follow the steps below to run the code:

### Compilation
```bash
gcc main.c -o parser
```

### Execution
```bash
./parser
```

## 🎯 Objectives
This project aims to provide practical experience in:
- Interpreter design
- Recursive descent parser structure
- Lexer and tokenization processes
- Handling conditional statements and loops


---
**Gülçin Bodur, Mehmet Ali Keklik, Batuhan Köse**

İstinye University - Computer Engineering
42 Turkiye - Common Core

