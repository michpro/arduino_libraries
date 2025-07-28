# Library: BooleanSolver

- [Library: BooleanSolver](#library-booleansolver)
  - [Summary](#summary)
  - [Example of Usage](#example-of-usage)
    - [Explanation of the Example](#explanation-of-the-example)

---

## Summary

The `BooleanSolver` library is a C++ utility designed to evaluate boolean expressions provided as strings. It supports:
- **Expression Solving**: The `solveExpression` function evaluates expressions like `"A && B || !C"` after normalization and simplification.
- **Variable Management**: Variables are stored in a `variable_t` structure with a name, value, and ID.<br>The `replaceVariables` function substitutes variable names with their values, and `setVariable` functions update variable values by ID or name.
- **Arduino Compatibility**: 
  - The library is compatible with all architectures.
  - The code uses `PROGMEM` for AVR architectures to optimize memory usage.

The library processes expressions by:
1. Normalizing them (e.g., converting "true" to "1", "&&" to "&").
2. Simplifying subexpressions within brackets.
3. Applying boolean algebra rules until a final "1" (true) or "0" (false) is obtained.

---

## Example of Usage

Below is an example demonstrating how to use the `BooleanSolver` library on an Arduino-compatible platform.

```cpp
#include "BooleanSolver.h"

void setup()
{
    Serial.begin(115200);

    // Define an array of variables
    BooleanSolver::variable_t   variables[] =
    {
        {"A", false, 0},                                // Variable A, initially false
        {"B", true, 1}                                  // Variable B, initially true
    };
    const uint8_t               VAR_COUNT {sizeof(variables) / sizeof(variables[0])};

    String expression = "A && B";                       // Define a boolean expression

    // Replace variables with their values
    String exprWithValues = BooleanSolver::replaceVariables(expression, variables, VAR_COUNT);
    Serial.print("Expression with values: ");
    Serial.println(exprWithValues);                     // Outputs "0 && 1"

    // Solve the expression
    bool result = BooleanSolver::solveExpression(exprWithValues);
    Serial.print("Result of 'A && B': ");
    Serial.println(result ? "true" : "false");          // Outputs "false"

    // Update variable A to true using its name
    BooleanSolver::setVariable(true, "A", variables, VAR_COUNT);

    // Replace variables again
    exprWithValues = BooleanSolver::replaceVariables(expression, variables, VAR_COUNT);
    Serial.print("Updated expression: ");
    Serial.println(exprWithValues);                     // Outputs "1 && 1"

    // Solve the updated expression
    result = BooleanSolver::solveExpression(exprWithValues);
    Serial.print("Result after setting A to true: ");
    Serial.println(result ? "true" : "false");          // Outputs "true"
}

void loop()
{
    // Nothing to do in the loop
}
```

### Explanation of the Example
1. **Initialization**: Two variables, `A` (false) and `B` (true), are defined.
2. **Expression**: The expression `"A && B"` is processed.
3. **First Evaluation**: After replacing variables, `"0 && 1"` evaluates to `false`.
4. **Update**: Variable `A` is set to `true`, making the expression `"1 && 1"`.
5. **Second Evaluation**: The updated expression evaluates to true.

This example demonstrates the core functionality of the library: solving expressions, replacing variables, and updating variable values dynamically.

---
