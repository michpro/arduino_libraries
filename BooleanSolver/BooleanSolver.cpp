/**
 * @file        BooleanSolver.cpp
 * @brief       Implementation of the BooleanSolver library functions
 *              for solving boolean expressions.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2024-2025 Michal Protasowicki
 *
 * @license SPDX-License-Identifier: MIT
 */

#include "BooleanSolver.h"

namespace BooleanSolver
{
    namespace
    {
        /**
         * @brief Structure to define replacement rules for trimming and simplifying expressions.
         */
        typedef struct
        {
            const char* from;   // Substring to be replaced (e.g., "true").
            const char* to;     // Replacement substring (e.g., "1").
        } rule_t;

#if defined(ARDUINO_ARCH_AVR)
    #define USE_PROGMEM PROGMEM
    #include <avr/pgmspace.h>
#else
    #define USE_PROGMEM
#endif

        /**
         * @brief Rules for trimming expressions to a standardized form.
         */
        const USE_PROGMEM rule_t trimRules[] =
        {
            {"true", "1"}, {"false", "0"}, {" ", ""},  {"[", "("},
            {"]", ")"},    {"~", "!"},     {"!!", ""}, {"&&", "&"},
            {"||", "|"}
        };

        /**
         * @brief Rules for simplifying expressions using boolean algebra.
         */
        const USE_PROGMEM rule_t simplifyRules[] =
        {
            {"!0", "1"},   {"!1", "0"},   {"1&1", "1"},  {"1&0", "0"},
            {"0&1", "0"},  {"0&0", "0"},  {"1|1", "1"},  {"1|0", "1"},
            {"0|1", "1"},  {"0|0", "0"},  {"0^0", "0"},  {"0^1", "1"},
            {"1^0", "1"},  {"1^1", "0"},  {"==1", ""},   {"1==0", "0"},
            {"0==0", "1"}, {"1!=1", "0"}, {"1!=0", "1"}, {"0!=1", "1"},
            {"0!=0", "0"}
        };

        const uint8_t   TRIM_RULES_CAPACITY     {sizeof(trimRules) / sizeof(trimRules[0])};
        const uint8_t   SIMPLIFY_RULES_CAPACITY {sizeof(simplifyRules) / sizeof(simplifyRules[0])};
        const int       NOT_FOUND               {-1};

        /**
         * @brief Normalizes an expression by applying trimming rules.
         * 
         * Converts the expression to lowercase and applies the trimRules to standardize operators 
         * and values (e.g., "true" to "1", "&&" to "&").
         * 
         * @param str The expression to normalize.
         */
        void normalize(String &str)
        {
            const uint8_t MAX_SIZE_FROM {6};
            const uint8_t MAX_SIZE_TO {2};

        #if defined(ARDUINO_ARCH_AVR)
            rule_t rule;
        #endif
            String _from;
            String _to;

            _from.reserve(MAX_SIZE_FROM);
            _to.reserve(MAX_SIZE_TO);

            str.toLowerCase();
            for (uint8_t idx = 0; idx < TRIM_RULES_CAPACITY; idx++)
            {
            #if defined(ARDUINO_ARCH_AVR)
                memcpy_P(&rule, &trimRules[idx], sizeof(rule_t));
                _from = rule.from;
                _to = rule.to;
            #else
                _from = trimRules[idx].from;
                _to = trimRules[idx].to;
            #endif
                str.replace(_from, _to);
            }
        }

        /**
         * @brief Finds the last subexpression enclosed in brackets.
         * 
         * Identifies the indices of the opening and closing brackets of the last parenthesized 
         * subexpression in the string.
         * 
         * @param str   The expression to search.
         * @param begin Output parameter for the index of the opening bracket.
         * @param end   Output parameter for the index of the closing bracket.
         * @return      true if a subexpression was found, false otherwise.
         */
        bool findLastExpressionInBrackets(String &str, uint8_t &begin, uint8_t &end)
        {
            bool    result  {false};
            int     _begin  {str.lastIndexOf("(")};
            int     _end    {(_begin != NOT_FOUND) ? str.indexOf(")", _begin) : NOT_FOUND};

            if ((_begin != NOT_FOUND) && (_end != NOT_FOUND))
            {
                result = true;
                begin = static_cast<uint8_t>(_begin);
                end = static_cast<uint8_t>(_end);
            }

            return result;
        }

        /**
         * @brief Simplifies an expression by repeatedly applying simplification rules.
         * 
         * Applies boolean algebra rules (e.g., "!0" to "1", "1&1" to "1") until no further 
         * simplifications are possible.
         * 
         * @param str The expression to simplify.
         */
        void simplifyExpression(String &str)
        {
            const uint8_t MAX_SIZE_FROM {5};
            const uint8_t MAX_SIZE_TO {2};

        #if defined(ARDUINO_ARCH_AVR)
            rule_t  rule;
        #endif
            bool    found {false};
            String  _from;
            String  _to;

            _from.reserve(MAX_SIZE_FROM);
            _to.reserve(MAX_SIZE_TO);
            do
            {
                found = false;
                for (uint8_t idx = 0; idx < SIMPLIFY_RULES_CAPACITY; idx++)
                {
                #if defined(ARDUINO_ARCH_AVR)
                    memcpy_P(&rule, &simplifyRules[idx], sizeof(rule_t));
                    _from = rule.from;
                    _to = rule.to;
                #else
                    _from = simplifyRules[idx].from;
                    _to = simplifyRules[idx].to;
                #endif
                    if (!found)
                    {
                        found = (str.indexOf(_from) != NOT_FOUND);
                    }
                    str.replace(_from, _to);
                }
            } while (found);
        }
    }

    /**
     * @brief Solves a boolean expression by normalizing and simplifying it.
     * 
     * Normalizes the expression, simplifies subexpressions within brackets, and then simplifies 
     * the entire expression to determine its boolean value.
     * 
     * @param expr  The boolean expression to solve.
     * @return      true if the final simplified expression is "1", false otherwise.
     */
    bool solveExpression(String &expr)
    {
        uint8_t begin   {0};
        uint8_t end     {0};

        normalize(expr);

        while (findLastExpressionInBrackets(expr, begin, end))
        {
            String _str {expr.substring(begin + 1, end)};
            simplifyExpression(_str);
            expr.remove(begin + 1, end - begin);
            expr.setCharAt(begin, (_str.equals("1") ? '1' : '0'));
        }

        simplifyExpression(expr);

        return expr.equals("1");
    }

    /**
     * @brief Replaces variable names in an expression with their values.
     * 
     * Iterates through the variables array and replaces each variable name with its value 
     * (converted to "1" or "0").
     * 
     * @param expr      The expression with variable names.
     * @param variables Array of variables with names and values.
     * @param varCount  Number of variables in the array.
     * @return          String The expression with variables replaced.
     */
    String replaceVariables(String expr, variable_t variables[], uint8_t varCount)
    {
        for (uint8_t idx = 0; idx < varCount; idx++)
        {
            expr.replace(variables[idx].name, String(variables[idx].value ? '1' : '0'));
        }

        return expr;
    }

    /**
     * @brief Sets the value of a variable by its ID.
     * 
     * Searches for a variable with the given ID and updates its value if found.
     * 
     * @param value     The new value to set.
     * @param id        The ID of the variable.
     * @param variables Array of variables.
     * @param varCount  Number of variables in the array.
     * @return          true if the variable was found and set, false otherwise.
     */
    bool setVariable(const bool value, const uint8_t id, variable_t variables[], uint8_t varCount)
    {
        bool found {false};

        while (!found && varCount)
        {
            varCount--;
            if (id == variables[varCount].id)
            {
                variables[varCount].value = value;
                found = true;
            }
        }

        return found;
    }

    /**
     * @brief Sets the value of a variable by its name.
     * 
     * Searches for a variable with the given name and updates its value if found.
     * 
     * @param value     The new value to set.
     * @param name      The name of the variable.
     * @param variables Array of variables.
     * @param varCount  Number of variables in the array.
     * @return          true if the variable was found and set, false otherwise.
     */
    bool setVariable(const bool value, const String &name, variable_t variables[], uint8_t varCount)
    {
        bool found {false};

        while (!found && varCount)
        {
            varCount--;
            if (variables[varCount].name.equals(name))
            {
                variables[varCount].value = value;
                found = true;
            }
        }

        return found;
    }
}
