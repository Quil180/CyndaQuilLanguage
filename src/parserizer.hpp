#pragma once

#include "fightingArena.hpp"
#include "tokener.hpp"
#include <cstdlib>
#include <optional>
#include <variant>

struct nodeExprIntLit {
  Token int_lit;
};

struct nodeExprIdent {
  Token identifier;
};

struct BinExprMul {
  nodeExpr *left;
  nodeExpr *right;
};

struct BinExprAdd {
  nodeExpr *left;
  nodeExpr *right;
};

struct BinExpr {
  std::variant<BinExprAdd *, BinExprMul *> variant;
};

struct nodeExpr {
  // a variant is just going to be one or the other.
  std::variant<nodeExprIntLit *, nodeExprIdent *, BinExpr *> variant;
};

struct nodeStmtRun {
  nodeExpr *expression;
};

struct nodeStmtCatch {
  Token identifier;
  nodeExpr *expression;
};

struct nodeStmt {
  std::variant<nodeStmtRun *, nodeStmtCatch *> variant;
};

struct nodeProgram {
  std::vector<nodeStmt> statements;
};

class Parser {
public:
  inline explicit Parser(std::vector<Token> tokens)
      : mem_tokens(std::move(tokens)), mem_allocator(1024 * 1024 * 4) {}

  std::optional<nodeExpr *> parse_expression() {
    if (peek().has_value() && peek().value().type == tokenType::int_lit) {
      auto *node_exprIntLit = mem_allocator.alloc<nodeExprIntLit>();
      node_exprIntLit->int_lit = consume();
      auto expr = mem_allocator.alloc<nodeExpr>();
      expr->variant = node_exprIntLit;
      return expr;
    } else if (peek().has_value() && peek().value().type == tokenType::ident) {
      auto expr_ident = mem_allocator.alloc<nodeExprIdent>();
      expr_ident->identifier = consume();
      auto expr = mem_allocator.alloc<nodeExpr>();
      expr->variant = expr_ident;
      return expr;
    } else {
      return {};
    }
  }

  std::optional<nodeStmt *> parse_statement() {
    if (peek().value().type == tokenType::run) {
      consume();
      auto stmt_run = mem_allocator.alloc<nodeStmtRun>();
      if (auto node_expr = parse_expression()) {
        stmt_run->expression = node_expr.value();
      } else {
        std::cerr << "invaild run expression..." << std::endl;
        exit(EXIT_FAILURE);
      }
      if (peek().has_value() && peek().value().type == tokenType::end_line) {
        consume();
      } else {
        std::cerr << "Expected '~'..." << std::endl;
        exit(EXIT_FAILURE);
      }
      auto stmt = mem_allocator.alloc<nodeStmt>();
      stmt->variant = stmt_run;
      return stmt;
    } else if (peek().has_value() && peek().value().type == tokenType::_catch &&
               peek(2).has_value() && peek(2).value().type == tokenType::as &&
               peek(3).has_value() &&
               peek(3).value().type == tokenType::ident) {
      auto statement_catch = nodeStmtCatch{};
      consume(); // get rid of the "catch"
      if (auto expression = parse_expression()) {
        statement_catch.expression = expression.value();
      } else {
        std::cerr << "Invalid catch... Correct format is..." << std::endl;
        std::cerr << "catch [expression] as [identifier]~" << std::endl;
        exit(EXIT_FAILURE);
      }
      consume(); // get rid of the "as"
      statement_catch.identifier = consume();
      if (peek().has_value() && peek().value().type == tokenType::end_line) {
        consume();
      } else {
        std::cerr << "Expected '~' not found..." << std::endl;
        exit(EXIT_FAILURE);
      }
      auto stmt = mem_allocator.alloc<nodeStmt>();
      stmt->variant = ;
      return stmt;
    } else {
      return {};
    }
  }

  std::optional<nodeProgram> parse_program() {
    nodeProgram program;
    while (peek().has_value()) {
      if (auto statement = parse_statement()) {
        program.statements.push_back(statement.value());
      } else {
        std::cerr << "Invalid statement found..." << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    return program;
  }

private:
  const std::vector<Token> mem_tokens;
  size_t mem_index = 0;
  ArenaAllocator mem_allocator;

  [[nodiscard]] inline std::optional<Token>
  peek(int offset = 0) const { // const as it shouldnt edit anything.
    if (mem_index + offset >= mem_tokens.size()) {
      return {};
    } else {
      return mem_tokens.at(mem_index +
                           offset); // return the character at that index.
    }
  }

  inline Token consume() { return mem_tokens.at(mem_index++); }
};
