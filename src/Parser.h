#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "Lexer.h"
#include "ast.h"

#include <memory>
using namespace std;


class Parser {
public:
    Parser();
    ~Parser() = default;

    bool Parse();                    // parse
    const llvm::Module& Generate();  // generate

private:
    int getNextToken();
    void Match ( Token needed );

    GenContext genContext;

    Lexer m_Lexer;                   // lexer is used to read tokens
    Token CurTok;                      // to keep the current token

    unique_ptr<ProgramASTNode> programASTNode;



    // Creating AST, checking syntax
    void Start ();
    // Program
    void Program ( string & nameOfProgram );
    // Consts
    void Const ( vector<unique_ptr<StatementASTNode>> & consts );
    void Assign ( vector<unique_ptr<StatementASTNode>> & consts );
    void NextConst ( vector<unique_ptr<StatementASTNode>> & consts );
    // Vars
    void Var ( vector<unique_ptr<StatementASTNode>> & vars );
    void NextVar ( vector<unique_ptr<StatementASTNode>> & vars );
    void Declare ( vector<unique_ptr<StatementASTNode>> & vars );


    void Body ( vector<unique_ptr<StatementASTNode>> & statements );
    void Expression ( vector<unique_ptr<StatementASTNode>> & statements );

    // Assignment
    void Assignment ( vector<unique_ptr<StatementASTNode>> & statements );

    // Arithmetic Expression
    unique_ptr<ExprASTNode> ArithmeticExpression();
    unique_ptr<ExprASTNode> L1 ();
    unique_ptr<ExprASTNode> L2 ();
    unique_ptr<BinOpASTNode> L2p();
    unique_ptr<ExprASTNode> L3 ();
    unique_ptr<BinOpASTNode> L3p();
    unique_ptr<ExprASTNode> L4 ();
    unique_ptr<BinOpASTNode> L4p();
    unique_ptr<ExprASTNode> L5 ();
    unique_ptr<BinOpASTNode> L5p();
    unique_ptr<BinOpASTNode> L6p();

    // Readln, Write and Writeln
    void Writeln ( vector<unique_ptr<StatementASTNode>> & statements );
    void Write ( vector<unique_ptr<StatementASTNode>> & statements );
    void Readln  ( vector<unique_ptr<StatementASTNode>> & statements );


    // Cycles
    void ForCycle( vector<unique_ptr<StatementASTNode>> & statements );
    void WhileCycle( vector<unique_ptr<StatementASTNode>> & statements );
    unique_ptr<AssignASTNode> Assignment();

    // If
    void If(  vector<unique_ptr<StatementASTNode>> & statements  );

};

#endif //PJPPROJECT_PARSER_HPP
