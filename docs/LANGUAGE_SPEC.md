# Mini Compiler — Language Specification

## Overview

This document describes the **MiniLang** language, a small programming language designed for educational purposes. MiniLang is intentionally limited to teach compiler fundamentals without the complexity of a full-featured language.

MiniLang supports integer arithmetic, variables, conditional statements, loops, and output. It does **not** support functions, classes, arrays, strings, floating-point numbers, or a type system.

---

## Data Types

### Integer

The only data type is a **signed integer**. Integer literals are sequences of decimal digits.

```text
42
0
1000
999999
```

Negative numbers are written as the minus operator followed by an integer literal:

```text
let x = -5;      // This is: minus operator applied to 5
```

There are no floating-point numbers, strings, booleans, or other types.

---

## Variables

### Declaration

Variables are declared using the `let` keyword. Every variable must be declared before it can be used.

```text
let x = 10;
let total = 0;
let result = 2 + 3 * 4;
```

### Assignment

After a variable is declared, it can be reassigned:

```text
x = x + 1;
total = total + x;
```

### Naming Rules

Variable names (identifiers) must:
- Start with a letter (`a`–`z`, `A`–`Z`) or underscore (`_`)
- Contain only letters, digits (`0`–`9`), or underscores
- Not be a reserved keyword

Valid examples: `x`, `total`, `my_var`, `_temp`, `count2`
Invalid examples: `2x` (starts with digit), `let` (keyword), `my-var` (contains hyphen)

---

## Operators

### Arithmetic Operators

| Operator | Meaning        | Example      |
|----------|----------------|--------------|
| `+`      | Addition       | `x + 1`      |
| `-`      | Subtraction    | `x - 1`      |
| `*`      | Multiplication | `x * 2`      |
| `/`      | Division       | `x / 3`      |

Standard mathematical precedence applies:
- `*` and `/` are evaluated before `+` and `-`
- Parentheses `()` override precedence

```text
let a = 2 + 3 * 4;       // Result: 14 (not 20)
let b = (2 + 3) * 4;     // Result: 20
```

### Comparison Operators

Comparison operators compare two values. They are used in `if` and `while` conditions.

| Operator | Meaning                | Example     |
|----------|------------------------|-------------|
| `==`     | Equal to               | `x == 10`   |
| `!=`     | Not equal to           | `x != 0`    |
| `<`      | Less than              | `x < 10`    |
| `<=`     | Less than or equal     | `x <= 10`   |
| `>`      | Greater than           | `x > 0`     |
| `>=`     | Greater than or equal  | `x >= 5`    |

### Assignment Operator

| Operator | Meaning                | Example     |
|----------|------------------------|-------------|
| `=`      | Assign value           | `x = 10`    |

> **Important:** `=` (single equals) is assignment. `==` (double equals) is comparison. These are different!

---

## Statements

### Print Statement

The `print` statement outputs the value of an expression, followed by a newline.

```text
print(42);        // Outputs: 42
print(x + 1);     // Outputs the value of x + 1
```

### If/Else Statement

Conditional execution based on a comparison.

```text
if (x > 10) {
    print(x);
}
```

With an `else` branch:

```text
if (x > 10) {
    print(x);
} else {
    print(0);
}
```

The braces `{ }` are required, even for single statements.

### While Loop

Repeats a block of statements while a condition is true.

```text
let i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

This prints: `0`, `1`, `2`, `3`, `4`.

---

## Comments

Single-line comments start with `//` and extend to the end of the line.

```text
// This is a comment
let x = 10; // This is also a comment
```

There are no multi-line comments.

---

## Whitespace

Spaces, tabs, and newlines are ignored between tokens. The following are equivalent:

```text
let x=10;
let x = 10;
let  x  =  10  ;
```

---

## Semicolons

Every statement must end with a semicolon `;`.

```text
let x = 10;       // Correct
print(x);          // Correct
```

---

## Complete Example Program

```text
// Calculate and print values
let x = 2 + 3 * 4;

if (x > 10) {
    print(x);
} else {
    print(0);
}

let i = 0;
while (i < 3) {
    print(i);
    i = i + 1;
}
```

Expected output:
```text
14
0
1
2
```

---

## Error Handling

The compiler reports errors for:
- **Unsupported characters:** Characters like `@`, `#`, `$` are not part of the language
- **Unterminated operators:** A standalone `!` without `=` is an error
- **Missing files:** The CLI reports an error if the source file cannot be opened

Error messages include the **line number** and **column number** where the problem was found, making it easy to locate and fix issues.

---

## Grammar (Formal)

This is the intended grammar for the complete language (parser will be implemented on Day 2):

```text
program      → statement*
statement    → "let" IDENTIFIER "=" expression ";"
             | IDENTIFIER "=" expression ";"
             | "print" "(" expression ")" ";"
             | "if" "(" condition ")" block ("else" block)?
             | "while" "(" condition ")" block
block        → "{" statement* "}"
condition    → expression comparison_operator expression
expression   → term (("+" | "-") term)*
term         → unary (("*" | "/") unary)*
unary        → "-" unary | primary
primary      → INTEGER | IDENTIFIER | "(" expression ")"
comparison_operator → "==" | "!=" | "<" | "<=" | ">" | ">="
```

---

## Implementation Status

### Day 1 (Implemented)
- ✅ Token type definitions (all types listed above)
- ✅ Lexer that converts source code into tokens
- ✅ Keyword recognition (let, if, else, while, print)
- ✅ Identifier and integer literal recognition
- ✅ All arithmetic and comparison operators
- ✅ Single-line comment support
- ✅ Line and column tracking
- ✅ Error reporting for unsupported characters
- ✅ Command-line driver to tokenize files

### Future Days (Planned)
- Day 2: Parser and AST
- Day 3: Three-address IR and IR interpreter
- Day 4: Constant folding and constant propagation
- Day 5: Dead-code elimination and pseudo-assembly generation
- Day 6: Automated tests and validation
- Day 7: Documentation, cleanup, and interview preparation
