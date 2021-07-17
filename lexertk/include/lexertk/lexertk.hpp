/*
 *****************************************************************
 *                 Simple C++ Lexer Toolkit Library              *
 *                                                               *
 * Author: Arash Partow (2001)                                   *
 * URL: http://www.partow.net/programming/lexertk/index.html     *
 *                                                               *
 * Copyright notice:                                             *
 * Free use of the Simple C++ Lexer Toolkit Library is permitted *
 * under the guidelines and in accordance with the MIT License.  *
 * http://www.opensource.org/licenses/MIT                        *
 *                                                               *
 *                                                               *
 * The lexer will tokenize input against the following BNF:      *
 *                                                               *
 * expression ::= term { +|- term }                              *
 * term       ::= (symbol | factor) {operator symbol | factor}   *
 * factor     ::= symbol | ( '(' {-} expression ')' )            *
 * symbol     ::= number | gensymb | string                      *
 * gensymb    ::= alphabet {alphabet | digit}                    *
 * string     ::= '"' {alphabet | digit | operator } '"'         *
 * operator   ::= * | / | % | ^ | < | > | <= | >= | << | >> !=   *
 * alphabet   ::= a | b | .. | z | A | B | .. | Z                *
 * digit      ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 |  7 | 8 | 9         *
 * sign       ::= + | -                                          *
 * edef       ::= e | E                                          *
 * decimal    ::= {digit} (digit [.] | [.] digit) {digit}        *
 * exponent   ::= edef [sign] digit {digit}                      *
 * real       ::= [sign] decimal [exponent]                      *
 * integer    ::= [sign] {digit}                                 *
 * number     ::= real | integer                                 *
 *                                                               *
 *                                                               *
 * Note: This lexer has been taken from the ExprTk Library.      *
 *                                                               *
 *****************************************************************
*/

#ifndef LEXERTK_LEXERTK_HPP
#define LEXERTK_LEXERTK_HPP

#include "token.hpp"
#include "generator.hpp"

#endif  //LEXERTK_LEXERTK_HPP
