#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <ostream>
#include "ast.h"

void FloatingPointError()
{
    throw std::runtime_error("Error with usage of floating point");
}

void ASTNode::gen() const
{
    GenContext gen("demo-pjp");
    codegen(gen);
    gen.module.print(llvm::outs(), nullptr);
}

llvm::Value* TypeASTNode::codegen(GenContext& gen) const { return nullptr; }

llvm::Type* TypeASTNode::genType(GenContext& gen) const
{
    switch (m_type) {
        case Type::DOUBLE:
            return llvm::Type::getDoubleTy(gen.ctx);
        case Type::INT:
            return llvm::Type::getInt32Ty(gen.ctx);
        default:
            assert(false);
    }
}

llvm::Value* BinOpASTNode::codegen(GenContext& gen) const
{
    auto lhs = m_lhs->codegen(gen);
    auto rhs = m_rhs->codegen(gen);

    assert(lhs);
    assert(rhs);

    bool dblArith = lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy();
    auto maybeSIToFP = [&gen](llvm::Value* val) {
        if (!val->getType()->isDoubleTy())
            return gen.builder.CreateSIToFP(val, llvm::Type::getDoubleTy(gen.ctx));
        return val;
    };

    switch (m_op) {
        case Token::tok_sum:
            if (dblArith)
                return gen.builder.CreateFAdd(maybeSIToFP(lhs), maybeSIToFP(rhs), "add");
            else
                return gen.builder.CreateAdd(lhs, rhs, "add");
        case Token::tok_substract:
            if (dblArith)
                return gen.builder.CreateFSub(maybeSIToFP(lhs), maybeSIToFP(rhs), "sub");
            else
                return gen.builder.CreateSub(lhs, rhs, "sub");
        case Token::tok_multiply:
            if (dblArith)
                return gen.builder.CreateFMul(maybeSIToFP(lhs), maybeSIToFP(rhs), "mul");
            else
                return gen.builder.CreateMul(lhs, rhs, "mul");

        case Token::tok_mod:
            if (dblArith)
            {
                // Error: Modulus operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateSRem(lhs, rhs, "mod");
        case Token::tok_div:
            if (dblArith)
                return gen.builder.CreateFDiv(maybeSIToFP(lhs), maybeSIToFP(rhs), "div");
            else
                return gen.builder.CreateSDiv(lhs, rhs, "div");
        case Token::tok_greater:
            if (dblArith)
            {
                // Error: Greater-than operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateICmpSGT(lhs, rhs, "greater");

        case Token::tok_less:
            if (dblArith)
            {
                // Error: Less-than operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateICmpSLT(lhs, rhs, "less");

        case Token::tok_greaterequal:
            if (dblArith)
            {
                // Error: Greater-than-or-equal operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateICmpSGE(lhs, rhs, "greaterequal");

        case Token::tok_lessequal:
            if (dblArith)
            {
                // Error: Less-than-or-equal operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateICmpSLE(lhs, rhs, "lessequal");

        case Token::tok_notequal:
            if (dblArith)
            {
                // Error: Not-equal operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateICmpNE(lhs, rhs, "notequal");

        case Token::tok_xor:
            if (dblArith)
            {
                // Error: XOR operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateXor(lhs, rhs, "xor");

        case Token::tok_and:
            if (dblArith)
            {
                // Error: AND operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateAnd(lhs, rhs, "and");

        case Token::tok_or:
            if (dblArith)
            {
                // Error: OR operation not supported for floating-point arithmetic
                FloatingPointError();
            }
            else
                return gen.builder.CreateOr(lhs, rhs, "or");
        case Token::tok_equal:
            if (dblArith)
                return gen.builder.CreateFCmpOEQ(maybeSIToFP(lhs), maybeSIToFP(rhs), "eq");
            else
                return gen.builder.CreateICmpEQ(lhs, rhs, "eq");
        default:
            throw "unimplemented";
    }
}

llvm::Value* UnaryOpASTNode::codegen(GenContext& gen) const
{
    auto expr = m_expr->codegen(gen);

    assert(expr);

    switch (m_op) {
        case Token::tok_not:
            if (expr->getType()->isDoubleTy())
            {
                // Error: Logical NOT operation not supported for double type
                FloatingPointError();
            }
            else
                return gen.builder.CreateNot(expr, "unnot");
        default:
            throw "unimplemented";
    }
}

llvm::Value* AssignASTNode::codegen(GenContext& gen) const
{
    auto* store = m_var->getStore(gen);
    auto expr = m_expr->codegen(gen);

    gen.builder.CreateStore(expr, store);
    return nullptr;
}

llvm::Value* LiteralASTNode::codegen(GenContext& gen) const
{
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), m_value);
}


llvm::Value* DeclRefASTNode::codegen(GenContext& gen) const
{
    assert(gen.symbolTable.contains(m_var));
    const auto& symbol = gen.symbolTable[m_var];

    return gen.builder.CreateLoad(symbol.type->genType(gen), symbol.store, m_var);
}

llvm::AllocaInst* DeclRefASTNode::getStore(GenContext& gen) const
{
    assert(gen.symbolTable.contains(m_var));
    return gen.symbolTable[m_var].store;
}

llvm::Value* DeclArrayRefASTNode::codegen(GenContext& gen) const {
    assert(gen.symbolTable.contains(m_var));
    const auto& symbol = gen.symbolTable[m_var];

    llvm::Value* indexValue = m_index->codegen(gen);

    llvm::ArrayType* arrayType = llvm::ArrayType::get(llvm::Type::getInt32Ty(gen.ctx), symbol.numberOfElements);

    // Get the element pointer using the index value
    llvm::Value * ind [] { gen.builder.getInt64(0), indexValue};
    auto elementPtr = gen.builder.CreateGEP(arrayType, symbol.store, ind, m_var + "_index");
    return gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), elementPtr);
}

llvm::Value* DeclArrayRefASTNode::getStore(GenContext& gen) const {
    assert(gen.symbolTable.contains(m_var));
    const auto& symbol = gen.symbolTable[m_var];

    llvm::Value* indexValue = m_index->codegen(gen);

    llvm::ArrayType* arrayType = llvm::ArrayType::get(llvm::Type::getInt32Ty(gen.ctx), symbol.numberOfElements);

    // Get the element pointer using the index value
    llvm::Value * ind [] { gen.builder.getInt64(0), indexValue};
    auto elementPtr = gen.builder.CreateGEP(arrayType, symbol.store, ind, m_var + "_index");

    return  elementPtr;

}

llvm::Value* FunCallASTNode::codegen(GenContext& gen) const
{
    auto* func = gen.module.getFunction(m_func);
    assert(func);

    std::vector<llvm::Value*> args;
    if ( this -> m_func == "readln")
    {
        for (const auto & ref : m_Refs )
            args . emplace_back(ref ->getStore(gen));
    } else {
        for (const auto& arg : m_Exprs)
            args.emplace_back(arg->codegen(gen));
    }

    return gen.builder.CreateCall(func, args);
}


llvm::Value* WhileASTNode::codegen(GenContext& gen) const {
    llvm::Function* currentFunction = gen.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* BBcond = llvm::BasicBlock::Create(gen.ctx, "cond", currentFunction);
    llvm::BasicBlock* BBbody = llvm::BasicBlock::Create(gen.ctx, "body", currentFunction);
    llvm::BasicBlock* BBafter = llvm::BasicBlock::Create(gen.ctx, "after", currentFunction);
    gen .BBreak = BBafter;

    // Branch to the condition block
    gen.builder.CreateBr(BBcond);

    // Emit code for the condition block
    gen.builder.SetInsertPoint(BBcond);
    llvm::Value* cond = m_cond->codegen(gen);
    gen.builder.CreateCondBr(cond, BBbody, BBafter);

    // Emit code for the body block
    gen.builder.SetInsertPoint(BBbody);
    for (const auto& statement : m_body) {
        llvm::Value* result = statement->codegen(gen);
        // Check if the result of the statement is a break instruction
        if ( result != nullptr )
        {
            llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(result);
            if (instruction != nullptr && instruction->getName() == "break") {
                // If it's a break, branch to the after block
                gen.builder.CreateBr(BBafter);
                gen.builder.SetInsertPoint(BBafter);
                return nullptr;
            }
        }
        gen .BBreak = BBafter;
    }

    // Branch back to the condition block
    gen.builder.CreateBr(BBcond);

    // Emit code for the after block
    gen.builder.SetInsertPoint(BBafter);

    return nullptr;
}

llvm::Value* ForASTNode::codegen(GenContext& gen) const {
    llvm::Function* currentFunction = gen.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* BBinit = llvm::BasicBlock::Create(gen.ctx, "init", currentFunction);
    llvm::BasicBlock* BBcond = llvm::BasicBlock::Create(gen.ctx, "cond", currentFunction);
    llvm::BasicBlock* BBbody = llvm::BasicBlock::Create(gen.ctx, "body", currentFunction);
    llvm::BasicBlock* BBafter = llvm::BasicBlock::Create(gen.ctx, "after", currentFunction);
    gen .BBreak = BBafter;

    // Branch to the initialization block
    gen.builder.CreateBr(BBinit);

    // Emit code for the initialization block
    gen.builder.SetInsertPoint(BBinit);
    m_initialization->codegen(gen);
    gen.builder.CreateBr(BBcond);

    // Emit code for the condition block
    gen.builder.SetInsertPoint(BBcond);
    llvm::Value* cond = m_condition->codegen(gen);
    gen.builder.CreateCondBr(cond, BBbody, BBafter);

    // Emit code for the body block
    gen.builder.SetInsertPoint(BBbody);
    for (const auto& statement : m_body) {
        llvm::Value* result = statement->codegen(gen);
        // Check if the result of the statement is a break instruction
        if ( result != nullptr )
        {
            llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(result);
            if (instruction != nullptr && instruction->getName() == "break") {
                // If it's a break, branch to the after block
                gen.builder.CreateBr(BBafter);
                gen.builder.SetInsertPoint(BBafter);  // Set insert point to BBafter before returning
                return nullptr;
            }
        }
        gen .BBreak = BBafter;
    }

    // Emit code for the increment block
    gen.builder.SetInsertPoint(gen.builder.GetInsertBlock());  // Set insert point to the current block
    m_increment->codegen(gen);
    gen.builder.CreateBr(BBcond);

    // Emit code for the after block
    gen.builder.SetInsertPoint(BBafter);

    return nullptr;
}

llvm::Value* IfASTNode::codegen(GenContext& gen) const {
    auto parent = gen.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* BBbody = llvm::BasicBlock::Create(gen.ctx, "body", parent);
    llvm::BasicBlock* BBelse = llvm::BasicBlock::Create(gen.ctx, "else", parent);
    llvm::BasicBlock* BBafter = llvm::BasicBlock::Create(gen.ctx, "after", parent);

    auto cond = m_cond->codegen(gen);
    gen.builder.CreateCondBr(cond, BBbody, BBelse);

    llvm::Instruction* value = nullptr;

    // Generate code for the true body
    gen.builder.SetInsertPoint(BBbody);
    for (const auto& statement : m_bodyTrue) {
        llvm::Value* result = statement->codegen(gen);
        // Check if the result of the statement is a break instruction
        if ( result != nullptr )
        {
            llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(result);
            if (instruction != nullptr && instruction->getName() == "break")
            {
                // Need to jump
                gen.builder.CreateBr(gen.BBreak);
            }
        }
    }
    gen.builder.CreateBr(BBafter);

    // Generate code for the else body
    parent->getBasicBlockList().push_back(BBelse);
    gen.builder.SetInsertPoint(BBelse);
    for (const auto& statement : m_bodyFalse) {
        llvm::Value* result = statement->codegen(gen);
        // Check if the result of the statement is a break instruction
        if ( result != nullptr )
        {
            llvm::Instruction* instruction = llvm::dyn_cast<llvm::Instruction>(result);
            if (instruction != nullptr && instruction->getName() == "break")
            {
                // Need to jump
                gen.builder.CreateBr(gen.BBreak);
            }
        }
    }
    gen.builder.CreateBr(BBafter);

    parent->getBasicBlockList().push_back(BBafter);

    gen.builder.SetInsertPoint(BBafter);


    return nullptr;
}

llvm::Value* BreakASTNode::codegen(GenContext& gen) const {
    llvm::Instruction* instruction = llvm::BinaryOperator::Create(llvm::Instruction::Add,
                                                                  llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0),
                                                                  llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));
    instruction->setName("break");
    return instruction;
}

llvm::Value* ConstDeclASTNode::codegen(GenContext& gen) const
{
    assert(!gen.symbolTable.contains(m_const));


    // Generate code for the expression
    llvm::Value * exprValue = m_expr->codegen(gen);

    // Create a constant global variable in the module
    llvm::Constant* constValue = llvm::cast<llvm::Constant>(exprValue);
    llvm::GlobalVariable* constGlobal = new llvm::GlobalVariable(
    gen.module, constValue->getType(), true, llvm::GlobalValue::InternalLinkage,
            constValue, m_const);

    // Create an alloca instruction to store the constant in the symbol table
    llvm::AllocaInst* constStore = gen.builder.CreateAlloca(constValue->getType(), nullptr, m_const);
    gen.builder.CreateStore(constValue, constStore);

    // Add the constant symbol to the symbol table
    Symbol constSymbol;
    constSymbol.name = m_const;
    constSymbol.type = new TypeASTNode (Type::INT);
    constSymbol.store = constStore;
    gen.symbolTable[m_const] = constSymbol;

    return nullptr;
}

llvm::Value* VarDeclASTNode::codegen(GenContext& gen) const
{
    assert(!gen.symbolTable.contains(m_var));

    llvm::AllocaInst * store = gen.builder.CreateAlloca(m_type->genType(gen), 0, m_var);
    gen.symbolTable[m_var] = {m_var, m_type.get(), store};

    return nullptr;
}

llvm::Value* ArrayDeclASTNode::codegen(GenContext& gen) const {

    llvm::Type* elementType = m_type->genType(gen);
    llvm::ArrayType* arrayType = llvm::ArrayType::get(elementType, m_upperBound - m_lowerBound + 1);
    llvm::AllocaInst* arrayAlloca = gen.builder.CreateAlloca(arrayType, nullptr, m_var);

    gen.symbolTable[m_var] = {m_var, m_type.get(), arrayAlloca, m_upperBound - m_lowerBound + 1, m_lowerBound };

    return nullptr;
}


llvm::Value* ProgramASTNode::codegen(GenContext& gen) const
{
    llvm::FunctionType* ftMain = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), false);
    llvm::Function* fMain = llvm::Function::Create(ftMain, llvm::Function::ExternalLinkage, "main", gen.module);
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(gen.ctx, "entry", fMain);
    gen.builder.SetInsertPoint(BB);

    for (const auto& s : m_statements)
        s->codegen(gen);

    gen.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));

    return nullptr;
}
