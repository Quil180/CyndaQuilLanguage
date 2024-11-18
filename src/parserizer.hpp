#pragma once

#include "fightingArena.hpp"
#include "tokener.hpp"
#include <cassert>
#include <cstdlib>
#include <variant>

struct nodeTermIntLit {
  Token int_lit;
};

struct nodeTermIdent {
  Token identifier;
};

struct nodeExpr;

struct nodeTermParen {
  nodeExpr *expr;
};


struct nodeBinExprMul {
  nodeExpr *left;
  nodeExpr *right;
};

struct nodeBinExprAdd {
  nodeExpr *left;
  nodeExpr *right;
};

struct nodeBinExprSub {
  nodeExpr *left;
  nodeExpr *right;
};

struct nodeBinExprDiv {
  nodeExpr *left;
  nodeExpr *right;
};

struct nodeBinExpr {
  std::variant<nodeBinExprAdd *, nodeBinExprMul *, nodeBinExprSub *,
               nodeBinExprDiv *>
      variant;
};

struct nodeTerm {
  std::variant<nodeTermIntLit *, nodeTermIdent *, nodeTermParen *> variant;
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
    } else if (auto open_paren= try_consume(tokenType::open_paren)) {
      auto expr = parse_expression();
      if (!expr.has_value()) {
        std::cerr << "Expected expression..." << std::endl;
        exit(EXIT_FAILURE);
      }
      // we have a valid expression...
      try_consume(tokenType::close_paren, "Expected ')'...");
      auto term_paren = mem_allocator.alloc<nodeTermParen>();
      term_paren->expr = expr.value();
      auto term = mem_allocator.alloc<nodeTerm>();
      term->variant = term_paren;
      return term;
    } else {
      return {};
    }
  }

  std::optional<nodeExpr *> parse_expression(int min_precedence = 0) {
    std::optional<nodeTerm *> term_left = parse_term();
    if (!term_left.has_value()) {
      return {};
    }

    auto expr_left = mem_allocator.alloc<nodeExpr>();
    expr_left->variant = term_left.value();

    // we have a valid term...
    while (true) {
      // checking if we have a binary operator.
      std::optional<Token> current_token = peek();
      std::optional<int> precedence;
      if (current_token.has_value()) {
        precedence = binary_precedence(current_token->type);
        if (!precedence.has_value() || precedence < min_precedence) {
          break;
        }
      } else {
        break;
      }

      Token oper = consume();
      int next_min_prec = precedence.value() + 1;
      // now we compute the right side.
      auto expr_right = parse_expression(next_min_prec);

      if (!expr_right.has_value()) {
        std::cerr << "Unable to parse expression..." << std::endl;
        exit(EXIT_FAILURE);
      }

      // now we need the left.
      auto expr = mem_allocator.alloc<nodeBinExpr>();
      auto new_expr_left = mem_allocator.alloc<nodeExpr>();
      if (oper.type == tokenType::plus) {
        auto add = mem_allocator.alloc<nodeBinExprAdd>();
        new_expr_left->variant = expr_left->variant;
        add->left = new_expr_left;
        add->right = expr_right.value();
        expr->variant = add;
      } else if (oper.type == tokenType::star) {
        auto mul = mem_allocator.alloc<nodeBinExprMul>();
        new_expr_left->variant = expr_left->variant;
        mul->left = new_expr_left;
        mul->right = expr_right.value();
        expr->variant = mul;
      } else if (oper.type == tokenType::sub) {
        auto sub = mem_allocator.alloc<nodeBinExprSub>();
        new_expr_left->variant = expr_left->variant;
        sub->left = new_expr_left;
        sub->right = expr_right.value();
        expr->variant = sub;
      } else if (oper.type == tokenType::div) {
        auto div = mem_allocator.alloc<nodeBinExprDiv>();
        new_expr_left->variant = expr_left->variant;
        div->left = new_expr_left;
        div->right = expr_right.value();
        expr->variant = div;
      } else {
        assert(false);
      }
      expr_left->variant = expr;
    }
    return expr_left;
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

      try_consume(tokenType::end_line,
                  "Expected '~' at end of run statement...");

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
