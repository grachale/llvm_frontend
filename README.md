# LLVM frontend
Extension of the LLVM compiler group with a new simple frontend for processing the Mila programming language, inspired by Pascal. The input to compiler is a file containing the source code. The output compiler is LLVM IR (Intermediate Representation) code.

In our input language, we will limit ourselves to the declaration of constants, global variables, and functions, with the data type int (or float). Numeric constants can be entered in decimal, hexadecimal, or octal format. We support arithmetic expressions with the operations +, -, *, div, mod, and parentheses. For statements, we support assignments, if, while, for, readln, write, and writeln.

Compiler can take:

1. Main function, number output (print, println), number input (readln), global variables, expressions, assignments, decimal numeric constants.

2. Numeric constants in hexadecimal and octal base (prefix $ and &).

3. If, While (together with the break statement).

4. For (both to and downto; together with the break statement).

5. Nested blocks.

6. Static array (indexed with values in any interval).
