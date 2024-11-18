#pragma once

#include "fightingArena.hpp"
#include "tokener.hpp"
#include <cstdlib>
#include <variant>

struct nodeTermIntLit {
  Token int_lit;
};

struct nodeTermIdent {
  Token identifier;
};

struct nodeExpr;

/*struct nodeBinExprMul {*/
/*  nodeExpr *left;*/
/*  nodeExpr *right;*/
/*};*/

struct nodeBinExprAdd {
  nodeExpr *left;
  nodeExpr *right;
};

struct nodeBinExpr {
  nodeBinExprAdd *add;
};

struct nodeTerm {
  std::variant<nodeTermIntLit *, nodeTermIdent *> variant;
};

struct nodeExpr {
  // a variant is just going to be one or the other.
  std::variant<nodeTerm *, nodeBinExpr *> variant;
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
  std::vector<nodeStmt *> statements;
};

class Parser {
public:
  inline explicit Parser(std::vector<Token> tokens)
      : mem_tokens(std::move(tokens)), mem_allocator(1024 * 1024 * 4) {}

  std::optional<nodeTerm *> parse_term() {
    if (auto int_lit = try_consume(tokenType::int_lit)) {
      // integer literal found.
      auto *term_IntLit = mem_allocator.alloc<nodeTermIntLit>();
      term_IntLit->int_lit = int_lit.value();
      auto term = mem_allocator.alloc<nodeTerm>();
      term->variant = term_IntLit;
      return term;
    } else if (auto ident = try_consume(tokenType::ident)) {
      // identifier found.
      auto term_Ident = mem_allocator.alloc<nodeTermIdent>();
      term_Ident->identifier = ident.value();
      auto term = mem_allocator.alloc<nodeTerm>();
      term->variant = term_Ident;
      return term;
    } else {
      return {};
    }
  }

  std::optional<nodeExpr *> parse_expression() {
    if (auto term = parse_term()) {
      if (try_consume(tokenType::plus).has_value()) {
        // we have a binary expression.
        auto bin_expr = mem_allocator.alloc<nodeBinExpr>();
        // we have a plus.
        auto bin_expr_add = mem_allocator.alloc<nodeBinExprAdd>();
        auto left_expr = mem_allocator.alloc<nodeExpr>();
        left_expr->variant = term.value();
        bin_expr_add->left = left_expr;
        if (auto right = parse_expression()) {
          bin_expr_add->right = right.value();
          bin_expr->add = bin_expr_add;
          auto expr = mem_allocator.alloc<nodeExpr>();
          expr->variant = bin_expr;
          return expr;
        } else {
          std::cerr << "Expected valid right to binary expression..."
                    << std::endl;
          exit(EXIT_FAILURE);
        }
      } else {
        auto expr = mem_allocator.alloc<nodeExpr>();
        expr->variant = term.value();
        return expr;
      }
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

      try_consume(tokenType::end_line, "Expected '~' at end of run statement...");

      auto stmt = mem_allocator.alloc<nodeStmt>();
      stmt->variant = stmt_run;
      return stmt;
    } else if (peek().has_value() && peek().value().type == tokenType::_catch) {
      auto statement_catch = mem_allocator.alloc<nodeStmtCatch>();
      consume(); // get rid of the "catch"
      if (auto expression = parse_expression()) {
        statement_catch->expression = expression.value();
      } else {
        std::cerr << "Invalid catch... Correct format is..." << std::endl;
        std::cerr << "catch [expression] as [identifier]~" << std::endl;
        exit(EXIT_FAILURE);
      }
      consume(); // get rid of the "as"
      statement_catch->identifier = consume();
      try_consume(tokenType::end_line, "Expected '~' at end of statement...");
      auto stmt = mem_allocator.alloc<nodeStmt>();
      stmt->variant = statement_catch;
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

  inline Token try_consume(tokenType type, const std::string &error) {
    if (peek().has_value() && peek().value().type == type) {
      return consume();
    } else {
      std::cerr << error << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  inline std::optional<Token> try_consume(tokenType type) {
    if (peek().has_value() && peek().value().type == type) {
      return consume();
    } else {
      return {};
    }
  }

  inline Token consume() { return mem_tokens.at(mem_index++); }
};
