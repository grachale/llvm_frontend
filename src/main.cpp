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
#include <llvm/Support/raw_ostream.h>

#include "Parser.h"




using namespace std;
int main (int argc, char *argv[])
{
    Parser parser;

    if (!parser.Parse()) {
        return 1;
    }


    std::error_code error;
    llvm::raw_fd_ostream outputFile("/home/grachale/PJP/testingSemestral/generatedCode.ll", error);


    if (!error) {
        parser.Generate().print(outputFile, nullptr);
    } else {
        llvm::errs() << "Error opening file: " << error.message() << "\n";
    }

//    Lexer lexer;
//    while (1)
//    {
//        cout << getTokenName(lexer .gettok()) << endl;
//        cout << "Number - " << lexer .numVal() << endl;
//    }

//    Parser parser;
//
//    parser . Parse();

//    Lexer lexer;
//
//    auto token = lexer .gettok();
//
//    while ( token != Token::tok_eof )
//    {
//        cout << getTokenName(token) << endl;
//        token = lexer .gettok();
//    }
//    parser.Generate().print(llvm::outs(), nullptr);

    return 0;
}
