#include "ast.h"

ASTNode::~ASTNode() = default;

TypeASTNode::TypeASTNode(Type type)
        : m_type(type)
{
}

ExprASTNode::~ExprASTNode() = default;

BinOpASTNode::BinOpASTNode(Token op, std::unique_ptr<ExprASTNode> lhs, std::unique_ptr<ExprASTNode> rhs)
        : m_op(op)
        , m_lhs(std::move(lhs))
        , m_rhs(std::move(rhs))
{
}

UnaryOpASTNode::UnaryOpASTNode(Token op, std::unique_ptr<ExprASTNode> expr)
        : m_op(op)
        , m_expr(std::move(expr))
{
}

LiteralASTNode::LiteralASTNode(int64_t value)
        : m_value(value)
{
}

DeclRefASTNode::DeclRefASTNode(std::string var)
        : m_var(std::move(var))
{
}

DeclArrayRefASTNode::DeclArrayRefASTNode(std::string var, std::unique_ptr<ExprASTNode> index)
        : m_var(std::move(var)), m_index(std::move(index)) {}


FunCallASTNode::FunCallASTNode(std::string func, std::vector<std::unique_ptr<VarASTNode>> args)
        : m_func(std::move(func))
        , m_Refs(std::move(args))
{
}

StatementASTNode::~StatementASTNode() = default;


IfASTNode::IfASTNode(std::unique_ptr<ExprASTNode> cond, std::vector<std::unique_ptr<StatementASTNode>> bodyTrue)
        : m_cond(std::move(cond))
        , m_bodyTrue(std::move(bodyTrue))
{
}

ArrayDeclASTNode::ArrayDeclASTNode(std::string var, std::unique_ptr<TypeASTNode> type, int lowerBound, int upperBound)
        : m_var(std::move(var)), m_type(std::move(type)), m_lowerBound(lowerBound), m_upperBound(upperBound) {}


WhileASTNode::WhileASTNode(std::unique_ptr<ExprASTNode> cond, std::vector<std::unique_ptr<StatementASTNode>> body)
        : m_cond(std::move(cond))
        , m_body(std::move(body))
{
}

ForASTNode::ForASTNode(std::unique_ptr<AssignASTNode> initialization, std::unique_ptr<ExprASTNode> condition,
                       std::unique_ptr<AssignASTNode> increment, std::vector<std::unique_ptr<StatementASTNode>> body)
        : m_initialization(std::move(initialization)),
          m_condition(std::move(condition)),
          m_increment(std::move(increment)),
          m_body(std::move(body)) {}

ConstDeclASTNode::ConstDeclASTNode(std::string nameOfConst, std::unique_ptr<ExprASTNode> expr)
        : m_const( std::move(nameOfConst) )
        , m_expr( std::move(expr) )
{
}


VarDeclASTNode::VarDeclASTNode(std::string var, std::unique_ptr<TypeASTNode> type )
        : m_var(std::move(var))
        , m_type(std::move(type))
{
}

AssignASTNode::AssignASTNode(std::unique_ptr<VarASTNode> var, std::unique_ptr<ExprASTNode> expr)
        : m_var(std::move(var))
        , m_expr(std::move(expr))
{
}

ProgramASTNode::ProgramASTNode(std::vector<std::unique_ptr<StatementASTNode>> statements)
        : m_statements(std::move(statements))
{
}
