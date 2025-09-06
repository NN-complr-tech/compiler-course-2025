#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>

namespace {
class ImplicitCastVisitor final
    : public clang::RecursiveASTVisitor<ImplicitCastVisitor> {
public:
  explicit ImplicitCastVisitor(clang::ASTContext *context)
      : m_context(context), m_currentFunction(nullptr) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    if (func->hasBody()) {
      m_currentFunction = func;
    }
    return true;
  }

  bool TraverseFunctionDecl(clang::FunctionDecl *func) {
    if (func->hasBody()) {
      m_currentFunction = func;
      RecursiveASTVisitor::TraverseStmt(func->getBody());
      printResult(func);
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) {
    if (!m_currentFunction)
      return true;

    auto src = cast->getSubExpr()->getType();
    auto dst = cast->getType();

    clang::QualType srcN = src.getCanonicalType().getUnqualifiedType();
    clang::QualType dstN = dst.getCanonicalType().getUnqualifiedType();

    if (m_context->hasSameType(srcN, dstN)) {
      return true;
    }

    if (srcN->isBuiltinType() && dstN->isBuiltinType()) {
      std::string from = srcN.getAsString();
      std::string to = dstN.getAsString();

      m_data[m_currentFunction->getNameAsString()][from + " -> " + to]++;
    }

    return true;
  }

  void printResult(clang::FunctionDecl *func) {
    std::string funcName = func->getNameAsString();
    llvm::outs() << "FUNCTION " << funcName << "\n";
    const auto &map = m_data[funcName];

    if (map.empty()) {
      llvm::outs() << "  (no implicit casts)\n";
    } else {
      for (const auto &[conversion, count] : map) {
        llvm::outs() << "  " << conversion << ": " << count << "\n";
      }
    }
  }

private:
  clang::ASTContext *m_context;
  clang::FunctionDecl *m_currentFunction;
  std::map<std::string, std::map<std::string, int>> m_data;
};

class ImplicitCastConsumer final : public clang::ASTConsumer {
public:
  explicit ImplicitCastConsumer(clang::ASTContext *context)
      : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  ImplicitCastVisitor m_visitor;
};

class ImplicitCastAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ImplicitCastConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitCastAction>
    X("implicit_cast_counter", "Counts all implicit casts grouped by function");
