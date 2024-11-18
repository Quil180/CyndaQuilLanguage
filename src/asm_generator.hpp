#pragma once

#include "parserizer.hpp"
#include <cassert>
#include <sstream>
#include <unordered_map>
#include <variant>

class ASMGenerator {
public:
  inline explicit ASMGenerator(nodeProgram program)
      : mem_program(std::move(program)) {}

  void generateExpr(const nodeExpr *expr) {
    struct ExprVisitor {
      ASMGenerator *gen;

      void operator()(const nodeExprIntLit *expr_int_lit) const {
        gen->mem_output << "  mov rax, " << expr_int_lit->int_lit.value.value()
                        << "\n";
        gen->push("rax");
      }
      void operator()(const nodeExprIdent *expr_ident) {
        if (!gen->mem_vars.contains(expr_ident->identifier.value.value())) {
          std::cerr << "Undeclared identifier "
                    << expr_ident->identifier.value.value() << " found...\n"
                    << std::endl;
          exit(EXIT_FAILURE);
        }
        const auto var = gen->mem_vars.at(expr_ident->identifier.value.value());
        std::stringstream offset;
        offset << "QWORD [rsp + "
               << (gen->mem_stack_size - var.stack_local - 1) * 8 << "]\n";
        // we know we have an already declared variable identifier.
        gen->push(offset.str());
      }
      void operator()(const nodeBinExpr *bin_expr) {
        assert(false);
      }
    };

    ExprVisitor visitor({.gen = this});
    std::visit(visitor, expr->variant);
  }

  void generateSttmt(const nodeStmt *stmt) {
    struct StmtVisitor {
      ASMGenerator *gen;

      void operator()(const nodeStmtRun *stmt_run) const {
        gen->generateExpr(stmt_run->expression);

        gen->mem_output << "  mov rax, 60\n";
        gen->pop("rdi");
        gen->mem_output << "  syscall\n";
      }
      void operator()(const nodeStmtCatch *stmt_catch) const {
        if (gen->mem_vars.contains(stmt_catch->identifier.value.value())) {
          std::cerr << "Variable " << stmt_catch->identifier.value.value()
                    << " already declared..." << std::endl;
          exit(EXIT_FAILURE);
        }
        // the variable is unused.
        // inserting the variable into the hashmap.
        gen->mem_vars.insert({stmt_catch->identifier.value.value(),
                              Var{.stack_local = gen->mem_stack_size}});
        // putting the value we want at the top of the stack.
        gen->generateExpr(stmt_catch->expression);
      }
    };

    StmtVisitor visitor({.gen = this});
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

  struct Var {
    // struct for the variables.
    size_t stack_local;
  };

  const nodeProgram mem_program; // program nodes.
  std::stringstream mem_output;  // output assembly.
  size_t mem_stack_size = 0;     // stack size.
  std::unordered_map<std::string, Var> mem_vars{};
};
