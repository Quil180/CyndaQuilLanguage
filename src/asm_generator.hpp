#pragma once

#include "parserizer.hpp"
#include <algorithm>
#include <map>
#include <sstream>

class ASMGenerator {
public:
  inline explicit ASMGenerator(nodeProgram program)
      : mem_program(std::move(program)) {}

  void generateTerm(const nodeTerm *term) {
    struct TermVisitor {
      ASMGenerator &gen;

      void operator()(const nodeTermIntLit *term_int_lit) const {
        gen.mem_output << "  mov rax, " << term_int_lit->int_lit.value.value()
                       << "\n";
        gen.push("rax");
      }
      void operator()(const nodeTermIdent *term_ident) const {
        auto iterator = std::find_if(
            gen.mem_vars.cbegin(), gen.mem_vars.cend(),
            [&](const Variable &var) {
              return var.name == term_ident->identifier.value.value();
            });
        if (iterator == gen.mem_vars.cend()) {
          std::cerr << "Undeclared identifier "
                    << term_ident->identifier.value.value() << " found...\n"
                    << std::endl;
          exit(EXIT_FAILURE);
        }
        std::stringstream offset;
        offset << "QWORD [rsp + "
               << (gen.mem_stack_size - (*iterator).stack_local - 1) * 8 << "]";
        // we know we have an already declared variable identifier.
        gen.push(offset.str());
      }
      void operator()(const nodeTermParen *term_paren) const {
        gen.generateExpr(term_paren->expr);
      }
    };

    TermVisitor visitor({.gen = *this});
    std::visit(visitor, term->variant);
  }

  void generateBinExpr(const nodeBinExpr *bin_expr) {
    struct BinExprVisitor {
      ASMGenerator &gen;

      void operator()(const nodeBinExprAdd *add) const {
        gen.generateExpr(add->right);
        gen.generateExpr(add->left);
        gen.pop("rax");
        gen.pop("rbx");
        gen.mem_output << "  add rax, rbx\n";
        gen.push("rax");
      }
      void operator()(const nodeBinExprSub *sub) const {
        gen.generateExpr(sub->right);
        gen.generateExpr(sub->left);
        gen.pop("rax");
        gen.pop("rbx");
        gen.mem_output << "  sub rax, rbx\n";
        gen.push("rax");
      }
      void operator()(const nodeBinExprMul *mul) const {
        gen.generateExpr(mul->right);
        gen.generateExpr(mul->left);
        gen.pop("rax");
        gen.pop("rbx");
        gen.mem_output << "  mul rbx\n";
        gen.push("rax");
      }
      void operator()(const nodeBinExprDiv *div) const {
        gen.generateExpr(div->right);
        gen.generateExpr(div->left);
        gen.pop("rax");
        gen.pop("rbx");
        gen.mem_output << "  div rbx\n";
        gen.push("rax");
      }
    };

    BinExprVisitor visitor{.gen = *this};
    std::visit(visitor, bin_expr->variant);
  }

  void generateExpr(const nodeExpr *expr) {
    struct ExprVisitor {
      ASMGenerator &gen;

      void operator()(const nodeTerm *term) const { gen.generateTerm(term); }
      void operator()(const nodeBinExpr *bin_expr) const {
        gen.generateBinExpr(bin_expr);
      }
    };

    ExprVisitor visitor({.gen = *this});
    std::visit(visitor, expr->variant);
  }

  void generateScope(const nodeScope *scope) {
    begin_scope();
    for (const nodeStmt *stmt : scope->stmts) {
      generateSttmt(stmt);
    }
    end_scope();
  }

  void generateSttmt(const nodeStmt *stmt) {
    struct StmtVisitor {
      ASMGenerator &gen;

      void operator()(const nodeStmtRun *stmt_run) const {
        gen.generateExpr(stmt_run->expression);

        gen.mem_output << "  mov rax, 60\n";
        gen.pop("rdi");
        gen.mem_output << "  syscall\n";
      }
      void operator()(const nodeStmtCatch *stmt_catch) const {
        auto iterator = std::find_if(
            gen.mem_vars.cbegin(), gen.mem_vars.cend(),
            [&](const Variable &var) {
              return var.name == stmt_catch->identifier.value.value();
            });
        if (iterator != gen.mem_vars.cend()) {
          std::cerr << "Variable " << stmt_catch->identifier.value.value()
                    << " already declared..." << std::endl;
          exit(EXIT_FAILURE);
        }
        // the variable is unused.
        // inserting the variable into the hashmap.
        gen.mem_vars.push_back({.name = stmt_catch->identifier.value.value(),
                                .stack_local = gen.mem_stack_size});
        // putting the value we want at the top of the stack.
        gen.generateExpr(stmt_catch->expression);
      }
      void operator()(const nodeScope *scope) const {
        gen.generateScope(scope);
      }
      void operator()(const nodeStmtPerc *stmt_perc) const {
        gen.generateExpr(stmt_perc->expr);
        gen.pop("rax");
        std::string label = gen.create_label();
        gen.mem_output << "  test rax, rax\n";
        gen.mem_output << "  jz " << label << "\n";
        gen.generateScope(stmt_perc->scope);
        gen.mem_output << label << ":\n";
      }
    };

    StmtVisitor visitor({.gen = *this});
    std::visit(visitor, stmt->variant);
  }

  [[nodiscard]] std::string generateProgram() {
    mem_output << "global _start\n_start:\n";

    // now we parse the program statements...
    for (const nodeStmt *statement : mem_program.statements) {
      generateSttmt(statement);
    }

    // assuming no run is in the program proper...
    mem_output << "  mov rax, 60\n";
    mem_output << "  mov rdi, 0\n";
    mem_output << "  syscall\n";
    return mem_output.str();
  }

private:
  void push(const std::string &reg) {
    mem_output << "  push " << reg << "\n";
    mem_stack_size++;
  }
  void pop(const std::string &reg) {
    mem_output << "  pop " << reg << "\n";
    mem_stack_size--;
  }
  void begin_scope() { mem_scopes.push_back(mem_vars.size()); }
  void end_scope() {
    size_t pop_cnt = mem_vars.size() - mem_scopes.back();
    mem_output << "  add rsp, " << pop_cnt * 8 << "\n";
    mem_stack_size -= pop_cnt;
    for (int i = 0; i < pop_cnt; i++) {
      mem_vars.pop_back(); // get rid of all the vars in the scope
    }
    mem_scopes.pop_back(); // get rid of the scop we just ended
  }
  std::string create_label() {
    std::stringstream ss;
    ss << "label" << mem_label_cnt++;
    return ss.str();
  }

  struct Variable {
    // struct for the variables.
    std::string name;
    size_t stack_local;
  };

  const nodeProgram mem_program;    // program nodes.
  std::stringstream mem_output;     // output assembly.
  size_t mem_stack_size = 0;        // stack size.
  std::vector<Variable> mem_vars{}; // variable "array"
  std::vector<size_t> mem_scopes{}; // indexes of the scopes in the mem_vars.
  int mem_label_cnt = 0;            // count of if statements...
};
