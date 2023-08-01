#pragma once
#include <map>
#include <memory>
#include <ostream>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "Lexer.h"


class TypeASTNode;

enum class Type { INT,
    DOUBLE
};

struct Symbol {
    std::string name;
    TypeASTNode * type;
    llvm::AllocaInst* store;
    int numberOfElements;
    int offset;
};


struct GenContext {
    GenContext(const std::string moduleName)
            : builder(ctx)
            , module(moduleName, ctx)
    {
    }
    llvm::BasicBlock* BBreak;
    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    std::map<std::string, Symbol> symbolTable;
};

class ASTNode {
public:
    virtual ~ASTNode();
    void gen() const;
    virtual llvm::Value* codegen(GenContext& gen) const = 0;
};


class TypeASTNode : public ASTNode {
public:

    TypeASTNode(Type type);
    llvm::Value* codegen(GenContext& gen) const override;
    llvm::Type* genType(GenContext& gen) const;

private:
    Type m_type;
};


class ExprASTNode : public ASTNode {
public:
    ExprASTNode() {}
    virtual ~ExprASTNode();
};


class BinOpASTNode : public ExprASTNode {
public:
    Token m_op;
    std::unique_ptr<ExprASTNode> m_lhs;
    std::unique_ptr<ExprASTNode> m_rhs;

    BinOpASTNode ( Token op )
            : m_op ( op ) {}
    BinOpASTNode(Token op, std::unique_ptr<ExprASTNode> lhs, std::unique_ptr<ExprASTNode> rhs);
    llvm::Value* codegen(GenContext& gen) const override;
};

class UnaryOpASTNode : public ExprASTNode {
    Token m_op;
    std::unique_ptr<ExprASTNode> m_expr;

public:
    UnaryOpASTNode(Token op, std::unique_ptr<ExprASTNode> expr);
    llvm::Value* codegen(GenContext& gen) const override;
};

class LiteralASTNode : public ExprASTNode {
    int64_t m_value;

public:
    LiteralASTNode(int64_t value);
    llvm::Value* codegen(GenContext& gen) const override;
};

class VarASTNode : public ExprASTNode {
public:
    std::string m_var;
    virtual llvm::Value* getStore(GenContext& gen) const = 0;
};


class DeclRefASTNode : public VarASTNode {
public:
    std::string m_var;

    DeclRefASTNode() {}
    DeclRefASTNode(std::string var);
    llvm::Value* codegen(GenContext& gen) const override;
    llvm::AllocaInst* getStore(GenContext& gen) const;
};

class DeclArrayRefASTNode : public VarASTNode {
public:
    std::string m_var;
    std::unique_ptr<ExprASTNode> m_index;

    DeclArrayRefASTNode() {}
    DeclArrayRefASTNode(std::string var, std::unique_ptr<ExprASTNode> index);
    DeclArrayRefASTNode(std::string var) : m_var(var) {}
    llvm::Value* codegen(GenContext& gen) const override;
    llvm::Value* getStore(GenContext& gen) const;
};


class StatementASTNode : public ASTNode {
public:
    virtual ~StatementASTNode();
};


class IfASTNode : public StatementASTNode {
public:
    std::unique_ptr<ExprASTNode> m_cond;
    std::vector<std::unique_ptr<StatementASTNode>> m_bodyTrue;
    std::vector<std::unique_ptr<StatementASTNode>> m_bodyFalse;

    IfASTNode() {}
    IfASTNode(std::unique_ptr<ExprASTNode> cond, std::vector<std::unique_ptr<StatementASTNode>> body);
    llvm::Value* codegen(GenContext& gen) const override;
};

class WhileASTNode : public StatementASTNode {
public:
    std::unique_ptr<ExprASTNode> m_cond;
    std::vector<std::unique_ptr<StatementASTNode>> m_body;

    WhileASTNode() {}
    WhileASTNode(std::unique_ptr<ExprASTNode> cond, std::vector<std::unique_ptr<StatementASTNode>> body);
    llvm::Value* codegen(GenContext& gen) const override;
};

class BreakASTNode : public StatementASTNode {
public:

    BreakASTNode() {}
    llvm::Value* codegen(GenContext& gen) const override;
};


class FunCallASTNode : public StatementASTNode {
public:
    std::string m_func;
    std::vector<std::unique_ptr<VarASTNode>> m_Refs;
    std::vector<std::unique_ptr<ExprASTNode>> m_Exprs;

    FunCallASTNode(std::string func )
            : m_func(func) {}
    FunCallASTNode(std::string func, std::vector<std::unique_ptr<VarASTNode>> args);
    llvm::Value* codegen(GenContext& gen) const override;
};


class ConstDeclASTNode : public StatementASTNode {
public:
    std::string m_const;
    std::unique_ptr<ExprASTNode> m_expr;

    ConstDeclASTNode () {}
    ConstDeclASTNode(std::string nameOfConst, std::unique_ptr<ExprASTNode> expr);
    llvm::Value* codegen(GenContext& gen) const override;
};

class VarDeclASTNode : public StatementASTNode {
public:
    std::string m_var;
    std::unique_ptr<TypeASTNode> m_type;

    VarDeclASTNode () {}
    VarDeclASTNode(std::string var, std::unique_ptr<TypeASTNode> type);
    llvm::Value* codegen(GenContext& gen) const override;
};

class ArrayDeclASTNode : public StatementASTNode {
public:
    std::string m_var;
    std::unique_ptr<TypeASTNode> m_type;
    int m_lowerBound;
    int m_upperBound;

    ArrayDeclASTNode() {}
    ArrayDeclASTNode(std::string var, std::unique_ptr<TypeASTNode> type, int lowerBound, int upperBound);
    llvm::Value* codegen(GenContext& gen) const override;
};

class AssignASTNode : public StatementASTNode {
public:
    std::unique_ptr<VarASTNode> m_var;
    std::unique_ptr<ExprASTNode> m_expr;

    AssignASTNode() {}
    AssignASTNode(std::unique_ptr<VarASTNode> var, std::unique_ptr<ExprASTNode> expr);
    llvm::Value* codegen(GenContext& gen) const override;
};

class ForASTNode : public StatementASTNode {
public:
    std::unique_ptr<AssignASTNode> m_initialization;
    std::unique_ptr<ExprASTNode> m_condition;
    std::unique_ptr<AssignASTNode> m_increment;
    std::vector<std::unique_ptr<StatementASTNode>> m_body;

    ForASTNode() {}
    ForASTNode(std::unique_ptr<AssignASTNode> initialization, std::unique_ptr<ExprASTNode> condition,
               std::unique_ptr<AssignASTNode> increment, std::vector<std::unique_ptr<StatementASTNode>> body);

    llvm::Value* codegen(GenContext& gen) const override;
};

class ProgramASTNode : public ASTNode {
public:
    std::string nameOfProgram;
    std::vector<std::unique_ptr<StatementASTNode>> m_statements;

    ProgramASTNode() {}
    ProgramASTNode(std::vector<std::unique_ptr<StatementASTNode>> statements);
    llvm::Value* codegen(GenContext& gen) const override;
};
