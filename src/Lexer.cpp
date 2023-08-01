#include "Lexer.h"
using namespace std;


Token Lexer::gettok()
{

    static int lastChar = ' ';
    if ( lastChar == '=' || lastChar == '>')
        lastChar = ' ';

    // Skipping whitespace characters
    while (isspace(lastChar))
        lastChar = cin.get();

    // Identifier or keyword
    if (isalpha(lastChar)) {
        m_IdentifierStr = lastChar;
        while (isalnum((lastChar = getchar())))
            m_IdentifierStr += lastChar;

        if (m_IdentifierStr == "begin")
            return tok_begin;
        if (m_IdentifierStr == "end")
            return tok_end;
        if (m_IdentifierStr == "const")
            return tok_const;
        if (m_IdentifierStr == "procedure")
            return tok_procedure;
        if (m_IdentifierStr == "forward")
            return tok_forward;
        if (m_IdentifierStr == "function")
            return tok_function;
        if (m_IdentifierStr == "if")
            return tok_if;
        if (m_IdentifierStr == "then")
            return tok_then;
        if (m_IdentifierStr == "else")
            return tok_else;
        if (m_IdentifierStr == "program")
            return tok_program;
        if (m_IdentifierStr == "while")
            return tok_while;
        if (m_IdentifierStr == "exit")
            return tok_exit;
        if (m_IdentifierStr == "var")
            return tok_var;
        if (m_IdentifierStr == "integer")
            return tok_integer;
        if (m_IdentifierStr == "for")
            return tok_for;
        if (m_IdentifierStr == "do")
            return tok_do;
        if (m_IdentifierStr == "to")
            return tok_to;
        if (m_IdentifierStr == "downto")
            return tok_downto;
        if (m_IdentifierStr == "array")
            return tok_array;
        if (m_IdentifierStr == "writeln")
            return tok_writeln;
        if (m_IdentifierStr == "readln")
            return tok_readln;
        if (m_IdentifierStr == "break")
            return tok_break;
        if (m_IdentifierStr == "write")
            return tok_write;
        if (m_IdentifierStr == "of")
            return tok_of;

        // Some operators
        if (m_IdentifierStr == "or")
            return tok_or;
        if (m_IdentifierStr == "mod")
            return tok_mod;
        if (m_IdentifierStr == "div")
            return tok_div;
        if (m_IdentifierStr == "not")
            return tok_not;
        if (m_IdentifierStr == "and")
            return tok_and;
        if (m_IdentifierStr == "xor")
            return tok_xor;

        return tok_identifier;
    }


    // Number
    if (isdigit(lastChar) )
    {
        string numStr;
        do {
            numStr += lastChar;
            lastChar = cin.get();
        } while (isdigit(lastChar));

        m_NumVal = stoi(numStr);
        return tok_number;
    }

    // Octal
    if ( lastChar == '&')
    {
        string numStr;
        lastChar = cin.get();
        do {
            numStr += lastChar;
            lastChar = cin.get();
        } while (isdigit(lastChar));
        m_NumVal = stoi(numStr, 0, 8);
        return tok_number;
    }

    // Hex
    if (lastChar == '$')
    {
        string numStr;
        lastChar = cin.get();
        do {
            numStr += lastChar;
            lastChar = cin.get();
        } while (isdigit(lastChar));
        m_NumVal = stoi(numStr, 0, 16);
        return tok_number;
    }


    int thisChar = lastChar;
    lastChar = cin.get();

    // Punctuation signs and operators
    switch ( thisChar )
    {
        case '(':
            return tok_leftparenthesis;
        case ')':
            return tok_rightparenthesis;
        case '=':
            return tok_equal;
        case ',':
            return tok_comma;
        case ';':
            return tok_semicolon;
        case '.':
            return tok_dot;
        case '+':
            return tok_sum;
        case '-':
            return tok_substract;
        case '*':
            return tok_multiply;
        case '[':
            return tok_squareleftparenthesis;
        case ']':
            return tok_squarerightparenthesis;
    }

    switch ( thisChar )
    {
        case EOF:
            return tok_eof;
        case '<':
            if (lastChar == '=')
                return tok_lessequal;
            else if (lastChar == '>')
                return tok_notequal;
            else
                return tok_less;
        case '>':
            if (lastChar == '=')
                return tok_greaterequal;
            else
                return tok_greater;
        case ':':
            if (lastChar == '=')
                return tok_assign;
            else
                return tok_colon;
    }

    // Undefined
    return tok_undefined;
}

