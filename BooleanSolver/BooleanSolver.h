/**
 * @file        BooleanSolver.h
 * @brief       Header file for the BooleanSolver library, providing functions
 *              to solve boolean expressions and manage variables.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2024-2025 Michal Protasowicki
 *
 * @license SPDX-License-Identifier: MIT
 */

#pragma once

#include "Arduino.h"

#ifdef ARDUINO_ARCH_AVR
#define USE_PROGMEM PROGMEM
#else
#define USE_PROGMEM
#endif

namespace BooleanSolver
{
    /**
     * @brief Structure to hold information about a boolean variable.
     */
    typedef struct
    {
        String  name;   // Name of the variable (e.g., "A", "B").
        bool    value;  // Current boolean value of the variable (true or false).
        uint8_t id;     // Unique identifier for the variable.
    } variable_t;

    /**
     * @brief Solves a boolean expression provided as a string.
     * 
     * This function evaluates a boolean expression by normalizing it, simplifying subexpressions 
     * within brackets, and applying boolean algebra rules to compute the final result.
     * 
     * @param expr  The boolean expression to solve (e.g., "A && B || !C").
     * @return      true if the expression evaluates to true, false otherwise.
     */
    bool solveExpression(String &expr);

    /**
     * @brief Replaces variable names in an expression with their corresponding values.
     * 
     * This function substitutes each variable name in the expression with its boolean value 
     * (converted to "1" or "0") from the provided array of variables.
     * 
     * @param expr      The expression containing variable names (e.g., "A && B").
     * @param variables Array of variable_t structures containing variable names and values.
     * @param varCount  Number of variables in the array.
     * @return          String The expression with variables replaced by their values (e.g., "0 && 1").
     */
    String replaceVariables(String expr, variable_t variables[], uint8_t varCount);

    /**
     * @brief Sets the value of a variable identified by its ID.
     * 
     * Searches the array of variables for a variable with the specified ID and updates its value.
     * 
     * @param value     The new boolean value to set.
     * @param id        The ID of the variable to update.
     * @param variables Array of variable_t structures.
     * @param varCount  Number of variables in the array.
     * @return          true if the variable was found and updated, false otherwise.
     */
    bool setVariable(const bool value, const uint8_t id, variable_t variables[], uint8_t varCount);

    /**
     * @brief Sets the value of a variable identified by its name.
     * 
     * Searches the array of variables for a variable with the specified name and updates its value.
     * 
     * @param value     The new boolean value to set.
     * @param name      The name of the variable to update.
     * @param variables Array of variable_t structures.
     * @param varCount  Number of variables in the array.
     * @return          true if the variable was found and updated, false otherwise.
     */
    bool setVariable(const bool value, const String &name, variable_t variables[], uint8_t varCount);
}
