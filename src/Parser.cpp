#include "Parser.h"
#include <iostream>
using namespace std;


Parser::Parser()
        : genContext ( "mila" ),
          programASTNode( new ProgramASTNode() ){}


const llvm::Module& Parser::Generate()
{

    genContext.module.getOrInsertFunction("writeln", llvm::FunctionType::get(llvm::Type :: getVoidTy(genContext.ctx), true));
    genContext.module.getOrInsertFunction("write", llvm::FunctionType::get(llvm::Type :: getVoidTy(genContext.ctx), true));
    genContext.module.getOrInsertFunction("readln", llvm::FunctionType::get(llvm::Type :: getVoidTy(genContext.ctx), true));

    programASTNode ->codegen( genContext );

    return this->genContext . module;
}

void ParserError()
{
    throw std::runtime_error("Parser error occurred.");
}

int Parser::getNextToken()
{
    return CurTok = m_Lexer.gettok();
}

void Parser::Match ( Token needed )
{
    if ( CurTok != needed )
        ParserError();
    else
        getNextToken();
}

bool Parser::Parse()
{
    getNextToken();
    Start ();
    return true;
}


void Parser::Start ()
{
    Program ( programASTNode -> nameOfProgram );
    Const ( programASTNode -> m_statements );
    Var ( programASTNode -> m_statements );
    Body( programASTNode -> m_statements );
    if ( CurTok != Token::tok_dot )
        ParserError();
}

void Parser::Program(string & nameOfProgram )
{
    switch ( CurTok )
    {
        case Token::tok_program:
        {
            Match(Token::tok_program);
            nameOfProgram = m_Lexer . identifierStr();
            Match(Token::tok_identifier);
            Match(Token::tok_semicolon);
            break;
        }
        default:
            ParserError();
    }
}

void Parser::Const( vector<unique_ptr<StatementASTNode>> & consts )
{
    switch ( CurTok )
    {
        case Token::tok_const: {
            Match(Token::tok_const);
            Assign( consts );
            NextConst( consts );
            break;
        }
        default:
            break;
    }
}

void Parser::Assign ( vector<unique_ptr<StatementASTNode>> & consts )
{
    unique_ptr<ConstDeclASTNode> constant ( new ConstDeclASTNode () );
    constant -> m_const = m_Lexer .identifierStr();
    Match ( Token::tok_identifier );
    Match ( Token::tok_equal );
    unique_ptr<LiteralASTNode> valueOfConst ( new LiteralASTNode ( m_Lexer . numVal() ));
    constant ->m_expr = std::move (valueOfConst);
    consts . emplace_back ( std::move(constant) );
    Match ( Token::tok_number );
    Match ( Token::tok_semicolon );
}

void Parser::NextConst ( vector<unique_ptr<StatementASTNode>> & consts )
{
    switch ( CurTok )
    {
        case Token::tok_identifier: {
            Assign ( consts );
            NextConst ( consts );
            break;
        }
        default:
            break;
    }
}

void Parser::Var( vector<unique_ptr<StatementASTNode>> & vars  )
{
    switch ( CurTok )
    {
        case Token::tok_var: {

            Match(Token::tok_var);
            Declare( vars );
            NextVar( vars );
            break;
        }
        default:
            break;
    }
}

void Parser::Declare( vector<unique_ptr<StatementASTNode>> & vars  )
{
    string nameOfVar = m_Lexer . identifierStr();
    Match ( Token::tok_identifier );

    switch ( CurTok )
    {
        case Token::tok_comma:
        {
            unique_ptr<VarDeclASTNode> var ( new VarDeclASTNode( ) );
            var ->m_var = nameOfVar;
            unique_ptr<TypeASTNode> type ( new TypeASTNode ( Type::INT ) );
            var -> m_type = std::move ( type );
            vars . emplace_back ( std::move(var) );
            Match(Token::tok_comma);
            Declare ( vars );
            break;
        }
        case Token::tok_colon:
        {
            Match ( Token::tok_colon );

            switch ( CurTok )
            {
                case Token::tok_integer:
                {
                    unique_ptr<VarDeclASTNode> var ( new VarDeclASTNode( ) );
                    var ->m_var = nameOfVar;
                    unique_ptr<TypeASTNode> type ( new TypeASTNode ( Type::INT ) );
                    var -> m_type = std::move ( type );
                    vars . emplace_back ( std::move(var) );
                    Match ( Token::tok_integer );
                    Match (Token::tok_semicolon );
                    break;
                }
                case Token::tok_array:
                {
                    Match(Token::tok_array);
                    Match(Token::tok_squareleftparenthesis);
                    int signLowerBound = 1;
                    if ( CurTok == Token::tok_substract )
                    {
                        Match(Token::tok_substract);
                        signLowerBound *= -1;
                    }
                    Match(Token::tok_number);
                    unique_ptr<ArrayDeclASTNode> array ( new ArrayDeclASTNode () );
                    assert(!genContext.symbolTable.contains(nameOfVar));
                    genContext.symbolTable[nameOfVar] = {nameOfVar, nullptr, nullptr,0, 0 };
                    array ->m_var = nameOfVar;
                    array ->m_lowerBound = m_Lexer .numVal() * signLowerBound;
                    genContext .symbolTable[nameOfVar] .offset = array -> m_lowerBound;
                    Match(Token::tok_dot);
                    Match(Token::tok_dot);
                    int signUpperBound = 1;
                    if ( CurTok == Token::tok_substract )
                    {
                        Match(Token::tok_substract);
                        signUpperBound *= -1;
                    }
                    Match(Token::tok_number);
                    array-> m_upperBound = m_Lexer . numVal() * signUpperBound;
                    unique_ptr<TypeASTNode> type ( new TypeASTNode ( Type::INT ) );
                    array -> m_type = std::move ( type );
                    Match(Token::tok_squarerightparenthesis);
                    Match(Token::tok_of);
                    Match(Token::tok_integer);
                    Match (Token::tok_semicolon );
                    vars .emplace_back(std::move(array));
                    break;
                }
                default:
                    ParserError();
            }
            break;
        }
        default:
            ParserError();
    }
}

void Parser::NextVar( vector<unique_ptr<StatementASTNode>> & vars  )
{
    switch ( CurTok )
    {
        case Token::tok_identifier: {
            Declare( vars );
            NextVar( vars );
            break;
        }
        case Token::tok_var: {
            Match(Token::tok_var);
            Declare( vars );
            NextVar( vars );
            break;
        }
        default:
            break;
    }
}


void Parser::Body( vector<unique_ptr<StatementASTNode>> & statements )
{
    Match ( Token::tok_begin );
    Expression ( statements );
    Match (Token::tok_end );
}

void Parser::Expression( vector<unique_ptr<StatementASTNode>> & statements )
{

    switch ( CurTok )
    {
        case Token::tok_identifier:
            Assignment( statements );
            Expression( statements );
            break;
        case Token::tok_for:
            ForCycle( statements );
            Expression( statements );
            break;
        case Token::tok_while:
            WhileCycle(statements);
            Expression( statements );
            break;
        case Token::tok_if:
            If( statements );
            Expression( statements );
            break;
        case Token::tok_writeln:
            Writeln ( statements );
            Expression(statements);
            break;
        case Token::tok_write:
            Write ( statements );
            Expression(statements);
            break;
        case Token::tok_readln:
            Readln ( statements );
            Expression(statements);
            break;
        case Token::tok_break:
        {
            Match(Token::tok_break);
            Match(Token::tok_semicolon);
            unique_ptr<BreakASTNode> breakNode ( new BreakASTNode () );
            statements .emplace_back(std::move(breakNode));
            Expression( statements );
            break;
        }
        default:
            break;
    }
}

void Parser::ForCycle( vector<unique_ptr<StatementASTNode>> & statements )
{
    unique_ptr<ForASTNode> forNode ( new ForASTNode () );
    Match(Token::tok_for);
    string nameOfVar = m_Lexer . identifierStr();

    switch ( CurTok )
    {
        case Token::tok_identifier:
        {

            Match(Token::tok_identifier);
            unique_ptr<AssignASTNode> assignment ( new AssignASTNode () );

            switch ( CurTok )
            {
                case Token::tok_squareleftparenthesis:
                {
                    unique_ptr<DeclArrayRefASTNode> array ( new DeclArrayRefASTNode () );
                    array ->m_var = nameOfVar;
                    Match(Token::tok_squareleftparenthesis);
                    array -> m_index = std::move(ArithmeticExpression());
                    Match(Token::tok_squarerightparenthesis);
                    assignment -> m_var = std::move (array);
                    break;
                }
                default:
                {
                    unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode (nameOfVar) );
                    assignment ->m_var= std::move(var);
                    break;
                }
            }
            Match(Token::tok_assign);
            assignment -> m_expr = std::move( ArithmeticExpression() );
            forNode -> m_initialization = std::move (assignment);
            break;
        }
        default:
            ParserError();
    }


    switch ( CurTok )
    {
        case Token::tok_to:
        {
            Match(Token::tok_to);
            unique_ptr<AssignASTNode> increment ( new AssignASTNode () );
            unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode(nameOfVar));
            increment -> m_var = std::move(var);
            unique_ptr<BinOpASTNode> plusOne ( new BinOpASTNode (Token::tok_sum));
            unique_ptr<DeclRefASTNode> var2 ( new DeclRefASTNode(nameOfVar));
            plusOne -> m_rhs = std::move(var2);
            unique_ptr<LiteralASTNode> one ( new LiteralASTNode (1));
            plusOne -> m_lhs = std::move ( one );
            increment -> m_expr = std::move(plusOne);
            forNode -> m_increment = std::move ( increment );
            break;
        }
        case Token::tok_downto:
        {
            Match(Token::tok_downto);
            unique_ptr<AssignASTNode> decrement ( new AssignASTNode () );
            unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode(nameOfVar));
            decrement -> m_var = std::move(var);
            unique_ptr<BinOpASTNode> plusOne ( new BinOpASTNode (Token::tok_substract));
            unique_ptr<DeclRefASTNode> var2 ( new DeclRefASTNode(nameOfVar));
            plusOne -> m_lhs = std::move(var2);
            unique_ptr<LiteralASTNode> one ( new LiteralASTNode (1));
            plusOne -> m_rhs = std::move ( one );
            decrement -> m_expr = std::move(plusOne);
            forNode -> m_increment = std::move ( decrement );
            break;
        }
        default:
            ParserError();
    }

    unique_ptr<BinOpASTNode> condition ( new BinOpASTNode (Token::tok_notequal));
    unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode(nameOfVar));
    condition -> m_lhs = std::move ( var );
    condition -> m_rhs = std::move(ArithmeticExpression());
    forNode ->m_condition = std::move ( condition );
    Match(Token::tok_do);
    switch (CurTok) {
        case Token::tok_identifier:
            Assignment(forNode -> m_body);
            break;
        case Token::tok_for:
            ForCycle(forNode -> m_body);
            break;
        case Token::tok_while:
            WhileCycle(forNode -> m_body);
            break;
        case Token::tok_if:
            If(forNode -> m_body);
            break;
        case Token::tok_writeln:
            Writeln(forNode -> m_body);
            break;
        case Token::tok_readln:
            Readln(forNode -> m_body);
            break;
        case Token::tok_write:
            Write(forNode -> m_body);
            break;
        case Token::tok_break:
        {
            Match(Token::tok_break);
            Match(Token::tok_semicolon);
            unique_ptr<BreakASTNode> breakNode ( new BreakASTNode () );
            statements .emplace_back(std::move(breakNode));
            break;
        }
        case Token::tok_begin:
            Body(forNode -> m_body);
            Match(Token::tok_semicolon);
            break;
        default:
            ParserError();
    }
    statements .emplace_back(std::move(forNode));
}

void Parser::WhileCycle( vector<unique_ptr<StatementASTNode>> & statements )
{
    unique_ptr<WhileASTNode> whileNode ( new WhileASTNode () );
    Match(Token::tok_while);
    whileNode->m_cond = std::move (ArithmeticExpression());
    Match(Token::tok_do);
    switch (CurTok) {
        case Token::tok_identifier:
            Assignment(whileNode -> m_body);
            break;
        case Token::tok_for:
            ForCycle(whileNode -> m_body);
            break;
        case Token::tok_while:
            WhileCycle(whileNode -> m_body);
            break;
        case Token::tok_if:
            If(whileNode -> m_body);
            break;
        case Token::tok_writeln:
            Writeln(whileNode -> m_body);
            break;
        case Token::tok_readln:
            Readln(whileNode -> m_body);
            break;
        case Token::tok_write:
            Write(whileNode -> m_body);
            break;
        case Token::tok_break:
        {
            Match(Token::tok_break);
            Match(Token::tok_semicolon);
            unique_ptr<BreakASTNode> breakNode ( new BreakASTNode () );
            statements .emplace_back(std::move(breakNode));
            break;
        }
        case Token::tok_begin:
            Body(whileNode -> m_body);
            Match(Token::tok_semicolon);
            break;
        default:
            ParserError();
    }
    statements .emplace_back(std::move(whileNode));
}

void Parser::If ( vector<unique_ptr<StatementASTNode>> & statements ) {
    Match(Token::tok_if);
    unique_ptr<IfASTNode> ifNode ( new IfASTNode () );
    ifNode -> m_cond = std::move (ArithmeticExpression());
    Match(Token::tok_then);
    switch (CurTok) {
        case Token::tok_identifier:
            Assignment(ifNode ->m_bodyTrue);
            break;
        case Token::tok_for:
            ForCycle(ifNode ->m_bodyTrue);
            break;
        case Token::tok_while:
            WhileCycle(ifNode ->m_bodyTrue);
            break;
        case Token::tok_if:
            If(ifNode ->m_bodyTrue);
            break;
        case Token::tok_writeln:
            Writeln(ifNode ->m_bodyTrue);
            break;
        case Token::tok_write:
            Write(ifNode ->m_bodyTrue);
            break;
        case Token::tok_readln:
            Readln(ifNode ->m_bodyTrue);
            break;
        case Token::tok_break:
        {
            Match(Token::tok_break);
            Match(Token::tok_semicolon);
            unique_ptr<BreakASTNode> breakNode ( new BreakASTNode () );
            ifNode ->m_bodyTrue .emplace_back(std::move(breakNode));
            break;
        }
        case Token::tok_begin:
            Body(ifNode ->m_bodyTrue);
            Match(Token::tok_semicolon);
            break;
        default:
            ParserError();
    }
    if ( CurTok == Token::tok_else )
    {
        Match(Token::tok_else);
        switch (CurTok) {
            case Token::tok_identifier:
                Assignment(ifNode ->m_bodyFalse);
                break;
            case Token::tok_for:
                ForCycle(ifNode ->m_bodyFalse);
                break;
            case Token::tok_while:
                WhileCycle(ifNode ->m_bodyFalse);
                break;
            case Token::tok_if:
                If(ifNode ->m_bodyFalse);
                break;
            case Token::tok_writeln:
                Writeln(ifNode ->m_bodyFalse);
                break;
            case Token::tok_write:
                Write(ifNode ->m_bodyFalse);
                break;
            case Token::tok_readln:
                Readln(ifNode ->m_bodyFalse);
                break;
            case Token::tok_break:
            {
                Match(Token::tok_break);
                Match(Token::tok_semicolon);
                unique_ptr<BreakASTNode> breakNode ( new BreakASTNode () );
                ifNode ->m_bodyFalse .emplace_back(std::move(breakNode));
                break;
            }
            case Token::tok_begin:
                Body(ifNode ->m_bodyFalse);
                Match(Token::tok_semicolon);
                break;
            default:
                ParserError();
        }
    }
    statements .emplace_back(std::move(ifNode));
}



void Parser::Assignment( vector<unique_ptr<StatementASTNode>> & statements )
{
    switch ( CurTok )
    {
        case Token::tok_identifier: {
            unique_ptr<AssignASTNode> assignment ( new AssignASTNode () );
            string nameOfVar = m_Lexer . identifierStr();
            Match(Token::tok_identifier);
            switch ( CurTok )
            {
                case Token::tok_squareleftparenthesis:
                {
                    Match(Token::tok_squareleftparenthesis);
                    unique_ptr<DeclArrayRefASTNode> array ( new DeclArrayRefASTNode () );
                    array -> m_var = nameOfVar;
                    if ( CurTok == Token::tok_substract)
                    {
                        Match(Token::tok_substract);
                        assert(genContext.symbolTable.contains(nameOfVar));
                        unique_ptr<LiteralASTNode> number ( new LiteralASTNode ( m_Lexer . numVal() - genContext.symbolTable[nameOfVar] .offset ) );
                        array -> m_index = std::move( number );
                        Match(Token::tok_number);
                    } else
                    {
                        unique_ptr<BinOpASTNode> plusOffset ( new BinOpASTNode ( Token::tok_substract ));
                        assert(genContext.symbolTable.contains(nameOfVar));
                        unique_ptr<LiteralASTNode> number ( new LiteralASTNode ( genContext.symbolTable[nameOfVar] .offset ) );
                        plusOffset ->m_rhs = std::move(number);
                        plusOffset -> m_lhs = std::move ( ArithmeticExpression());
                        array -> m_index = std::move(plusOffset);
                    }
                    Match(Token::tok_squarerightparenthesis);
                    assignment ->m_var = std::move(array);
                    break;
                }
                default:
                {
                    unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode ( nameOfVar) );
                    assignment -> m_var = std::move(var);
                }
            }

            Match(Token::tok_assign);
            assignment -> m_expr = std::move( ArithmeticExpression() );
            statements . push_back (std::move(assignment));
            Match(Token::tok_semicolon);
            break;
        }
        default:
            ParserError();
    }
}

unique_ptr<AssignASTNode> Parser::Assignment()
{
    switch ( CurTok )
    {
        case Token::tok_identifier: {
            unique_ptr<AssignASTNode> assignment ( new AssignASTNode () );
            Match(Token::tok_identifier);
            unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode ( m_Lexer.identifierStr()) );
            assignment ->m_var= std::move(var);
            Match(Token::tok_assign);
            assignment -> m_expr = std::move( ArithmeticExpression() );
            break;
        }
        default:
            ParserError();
    }
    return nullptr;
}

unique_ptr<ExprASTNode> Parser::ArithmeticExpression()
{
    switch( CurTok )
    {
        case Token::tok_leftparenthesis:
        case Token::tok_number:
        case Token::tok_identifier:
        case Token::tok_not:
        {

            unique_ptr<ExprASTNode> expression = std::move(L5());
            unique_ptr<BinOpASTNode> binaryExpr = std::move(L6p());
            if ( binaryExpr == nullptr )
                return expression;
            else
            {
                binaryExpr -> m_lhs = std::move(expression);
                return binaryExpr;
            }
        }
        case Token::tok_substract:
        {
            Match(Token::tok_substract);
            unique_ptr<LiteralASTNode> number ( new LiteralASTNode (m_Lexer .numVal() * -1));
            Match(Token::tok_number);
            return number;
        }
        default:
            ParserError();
    }
    return nullptr;
}

unique_ptr<BinOpASTNode> Parser::L6p()
{
    switch( CurTok )
    {
        case Token::tok_or:
        {

            Match(Token::tok_or);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_or ) );
            unique_ptr<ExprASTNode> expression = std::move(L5());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L6p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_and:
        {
            Match(Token::tok_and);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_and ) );
            unique_ptr<ExprASTNode> expression = std::move(L5());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L6p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_xor:
        {
            Match(Token::tok_xor);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_xor ) );
            unique_ptr<ExprASTNode> expression = std::move(L5());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L6p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        default:
            return nullptr;
    }
}

unique_ptr<ExprASTNode> Parser::L5()
{
    switch( CurTok )
    {
        case Token::tok_leftparenthesis:
        case Token::tok_number:
        case Token::tok_identifier:
        case Token::tok_not:
        {
            unique_ptr<ExprASTNode> expression = std::move(L4());
            unique_ptr<BinOpASTNode> binaryExpr = std::move(L5p());
            if ( binaryExpr == nullptr )
                return expression;
            else
            {
                binaryExpr -> m_lhs = std::move(expression);
                return binaryExpr;
            }
        }
        default:
            ParserError();
    }
    return nullptr;
}

unique_ptr<BinOpASTNode> Parser::L5p()
{
    switch( CurTok )
    {
        case Token::tok_equal:
        {
            Match(Token::tok_equal);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_equal ) );
            unique_ptr<ExprASTNode> expression = std::move(L4());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L5p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_notequal:
        {
            Match(Token::tok_notequal);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_notequal ) );
            unique_ptr<ExprASTNode> expression = std::move(L4());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L5p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        default:
            return nullptr;
    }
}

unique_ptr<ExprASTNode> Parser::L4()
{
    switch( CurTok )
    {
        case Token::tok_leftparenthesis:
        case Token::tok_number:
        case Token::tok_identifier:
        case Token::tok_not:
        {
            unique_ptr<ExprASTNode> expression = std::move(L3());
            unique_ptr<BinOpASTNode> binaryExpr = std::move(L4p());
            if ( binaryExpr == nullptr )
                return expression;
            else
            {
                binaryExpr -> m_lhs = std::move(expression);
                return binaryExpr;
            }
        }
        default:
            ParserError();
    }
    return nullptr;
}

unique_ptr<BinOpASTNode> Parser::L4p()
{
    switch( CurTok ) {
        case Token::tok_greater:
        {
            Match(Token::tok_greater);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_greater ) );
            unique_ptr<ExprASTNode> expression = std::move(L3());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L4p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_less:
        {
            Match(Token::tok_less);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_less ) );
            unique_ptr<ExprASTNode> expression = std::move(L3());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L4p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_greaterequal:
        {
            Match(Token::tok_greaterequal);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_greaterequal ) );
            unique_ptr<ExprASTNode> expression = std::move(L3());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L4p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_lessequal:
        {
            Match(Token::tok_lessequal);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_lessequal ) );
            unique_ptr<ExprASTNode> expression = std::move(L3());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L4p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        default:
            return nullptr;
    }
}

unique_ptr<ExprASTNode> Parser::L3()
{
    switch( CurTok ) {
        case Token::tok_leftparenthesis:
        case Token::tok_number:
        case Token::tok_identifier:
        case Token::tok_not:
        {
            unique_ptr<ExprASTNode> expression = std::move(L2());
            unique_ptr<BinOpASTNode> binaryExpr = std::move(L3p());
            if ( binaryExpr == nullptr )
                return expression;
            else
            {
                binaryExpr -> m_lhs = std::move(expression);
                return binaryExpr;
            }
        }
        default:
            ParserError();
    }
    return nullptr;
}

unique_ptr<BinOpASTNode> Parser::L3p()
{
    switch(CurTok) {
        case Token::tok_sum:
        {
            Match(Token::tok_sum);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_sum ) );
            unique_ptr<ExprASTNode> expression = std::move(L2());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L3p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_substract:
        {
            Match(Token::tok_substract);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_substract ) );
            unique_ptr<ExprASTNode> expression = std::move(L2());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L3p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        default:
            return nullptr;
    }
}

unique_ptr<ExprASTNode> Parser::L2()
{
    switch(CurTok) {
        case Token::tok_leftparenthesis:
        case Token::tok_number:
        case Token::tok_identifier:
        case Token::tok_not:
        {
            unique_ptr<ExprASTNode> expression = std::move(L1());
            unique_ptr<BinOpASTNode> binaryExpr = std::move(L2p());
            if ( binaryExpr == nullptr )
                return expression;
            else
            {
                binaryExpr -> m_lhs = std::move(expression);
                return binaryExpr;
            }
        }
        default:
            ParserError();
    }
    return nullptr;
}

unique_ptr<BinOpASTNode> Parser::L2p()
{
    switch( CurTok ) {
        case Token::tok_multiply:
        {
            Match(Token::tok_multiply);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_multiply ) );
            unique_ptr<ExprASTNode> expression = std::move(L1());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L2p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_mod:
        {
            Match(Token::tok_mod);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_mod ) );
            unique_ptr<ExprASTNode> expression = std::move(L1());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L2p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        case Token::tok_div:
        {
            Match(Token::tok_div);
            unique_ptr<BinOpASTNode> binaryExpr ( new  BinOpASTNode ( Token::tok_div ) );
            unique_ptr<ExprASTNode> expression = std::move(L1());
            unique_ptr<BinOpASTNode> binaryExprLow = std::move(L2p());
            if ( binaryExprLow == nullptr )
            {
                binaryExpr -> m_rhs = std::move(expression);
            } else
            {
                binaryExprLow -> m_lhs = std::move(expression);
                binaryExpr -> m_rhs = std::move(binaryExprLow);
            }
            return binaryExpr;
        }
        default:
            return nullptr;
    }
}


unique_ptr<ExprASTNode> Parser::L1()
{
    switch(CurTok) {
        case Token::tok_leftparenthesis:
        {
            Match(Token::tok_leftparenthesis);
            unique_ptr<ExprASTNode> expr = move(ArithmeticExpression());
            Match(Token::tok_rightparenthesis);
            return expr;
        }
        case Token::tok_number:
        {
            unique_ptr<LiteralASTNode> number ( new LiteralASTNode (m_Lexer .numVal() ) );
            Match(Token::tok_number);
            return number;
        }
        case Token::tok_identifier:
        {
            string nameOfVar = m_Lexer . identifierStr();
            Match(Token::tok_identifier);
            switch ( CurTok )
            {
                case Token::tok_squareleftparenthesis:
                {
                    unique_ptr<DeclArrayRefASTNode> array ( new DeclArrayRefASTNode () );
                    array ->m_var = nameOfVar;
                    Match(Token::tok_squareleftparenthesis);
                    if ( CurTok == Token::tok_substract)
                    {
                        Match(Token::tok_substract);
                        assert(genContext.symbolTable.contains(nameOfVar));
                        unique_ptr<LiteralASTNode> number ( new LiteralASTNode ( m_Lexer . numVal() - genContext.symbolTable[nameOfVar] .offset ) );
                        array -> m_index = std::move( number );
                        Match(Token::tok_number);
                    } else
                    {
                        unique_ptr<BinOpASTNode> plusOffset ( new BinOpASTNode ( Token::tok_substract ));
                        assert(genContext.symbolTable.contains(nameOfVar));
                        unique_ptr<LiteralASTNode> number ( new LiteralASTNode ( genContext.symbolTable[nameOfVar] .offset ) );
                        plusOffset ->m_rhs = std::move(number);
                        plusOffset -> m_lhs = std::move ( ArithmeticExpression());
                        array -> m_index = std::move(plusOffset);
                    }
                    Match(Token::tok_squarerightparenthesis);
                    return array;
                }
                default:
                {
                    unique_ptr<DeclRefASTNode> var ( new DeclRefASTNode ( nameOfVar ) );
                    return var;
                }
            }
        }
        case Token::tok_not:
        {
            Match(Token::tok_not);
            unique_ptr<UnaryOpASTNode> expr ( new UnaryOpASTNode (Token::tok_not, L1()));
            return expr;
        }
        default:
            ParserError();
    }
    return nullptr;
}


void Parser::Writeln(vector<unique_ptr<StatementASTNode>> &statements)
{
    Match(Token::tok_writeln);
    Match(Token::tok_leftparenthesis);
    unique_ptr<FunCallASTNode> func ( new FunCallASTNode( "writeln") );
    func -> m_Exprs .emplace_back(ArithmeticExpression());
    statements .emplace_back(std::move(func));
    Match(Token::tok_rightparenthesis);
    Match(Token::tok_semicolon);
}

void Parser::Write(vector<unique_ptr<StatementASTNode>> &statements)
{
    Match(Token::tok_write);
    Match(Token::tok_leftparenthesis);
    unique_ptr<FunCallASTNode> func ( new FunCallASTNode( "write") );
    func -> m_Exprs .emplace_back(ArithmeticExpression());
    statements .emplace_back(std::move(func));
    Match(Token::tok_rightparenthesis);
    Match(Token::tok_semicolon);
}

void Parser::Readln(vector<unique_ptr<StatementASTNode>> &statements)
{
    Match(Token::tok_readln);
    Match(Token::tok_leftparenthesis);
    unique_ptr<FunCallASTNode> func ( new FunCallASTNode( "readln") );
    string nameOfVar = m_Lexer .identifierStr();
    Match(Token::tok_identifier);

    switch ( CurTok )
    {
        case Token::tok_squareleftparenthesis:
        {
            Match(Token::tok_squareleftparenthesis);
            unique_ptr<DeclArrayRefASTNode> array ( new DeclArrayRefASTNode ( ));
            array ->m_var = nameOfVar;
            array -> m_index = std::move ( ArithmeticExpression() );
            func -> m_Refs .emplace_back(std::move(array));
            Match(Token::tok_squarerightparenthesis);
            Match(Token::tok_rightparenthesis);
            break;
        }
        default:
        {
            unique_ptr<DeclRefASTNode> var ( new  DeclRefASTNode ( nameOfVar ) );
            Match(Token::tok_rightparenthesis);
            func -> m_Refs .emplace_back(std::move(var));
        }
    }
    statements .emplace_back(std::move(func));
    Match(Token::tok_semicolon);
}