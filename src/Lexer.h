#ifndef PJPPROJECT_LEXER_HPP
#define PJPPROJECT_LEXER_HPP

#include <iostream>


/*
 * Lexer returns tokens [0-255] if it is an unknown character, otherwise one of these for known things.
 * Here are all valid tokens:
 */
enum Token {
    tok_eof =           -1,

    // numbers and identifiers
    tok_identifier =    -2,
    tok_number =        -3,

    // keywords
    tok_begin =         -4,
    tok_end =           -5,
    tok_const =         -6,
    tok_procedure =     -7,
    tok_forward =       -8,
    tok_function =      -9,
    tok_if =            -10,
    tok_then =          -11,
    tok_else =          -12,
    tok_program =       -13,
    tok_while =         -14,
    tok_exit =          -15,
    tok_var =           -16,
    tok_integer =       -17,
    tok_for =           -18,
    tok_do =            -19,

    // 2-character operators
    tok_notequal =      -20,
    tok_lessequal =     -21,
    tok_greaterequal =  -22,
    tok_assign =        -23,
    tok_or =            -24,

    // 3-character operators (keywords)
    tok_mod =           -25,
    tok_div =           -26,
    tok_not =           -27,
    tok_and =           -28,
    tok_xor =           -29,

    // keywords in for loop
    tok_to =            -30,
    tok_downto =        -31,

    // keywords for array
    tok_array =         -32,


    // another signs
    tok_semicolon                = -33,
    tok_dot                      = -34,
    tok_comma                    = -35,
    tok_equal                    = -36,
    tok_colon                    = -37,
    tok_leftparenthesis          = -38,
    tok_rightparenthesis         = -39,
    tok_sum                      = -40,
    tok_substract                = -41,
    tok_multiply                 = -42,
    tok_less                     = -43,
    tok_greater                  = -44,
    tok_squareleftparenthesis    = -45,
    tok_squarerightparenthesis   = -46,
    tok_writeln                  = -47,
    tok_readln                   = -48,
    tok_break                    = -49,
    tok_write                    = -50,
    tok_of                       = -51,


    // undefined
    tok_undefined                = 0
};

class Lexer {
public:
    Lexer() = default;
    ~Lexer() = default;

    Token gettok();
    std::string identifierStr() const { return m_IdentifierStr; }
    int numVal() const { return this->m_NumVal; }

private:
    std::string m_IdentifierStr;
    int m_NumVal;

};


#endif //PJPPROJECT_LEXER_HPP
