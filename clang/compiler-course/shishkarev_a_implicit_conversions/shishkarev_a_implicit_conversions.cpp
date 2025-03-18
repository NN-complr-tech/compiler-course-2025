#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <utility>
#include <string>

namespace {
class ImplicitConversionVisitor : public clang::RecursiveASTVisitor<ImplicitConversionVisitor> {
public:
  explicit ImplicitConversionVisitor(clang::ASTContext *context) : m_context(context) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    llvm::outs() << "Function `" << func->getName() << "`\n";
    m_conversions.clear();

    // Обходим тело функции
    if (func->hasBody()) {
      TraverseStmt(func->getBody());
    }

    // Выводим результаты в порядке, в котором они были найдены
    for (const auto &conv : m_conversions) {
      llvm::outs() << conv.first << " -> " << conv.second << ": 1\n";
    }

    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *expr) {
    // Получаем типы до и после преобразования
    auto fromType = expr->getSubExpr()->getType().getAsString();
    auto toType = expr->getType().getAsString();

    // Сохраняем преобразование в порядке обхода
    m_conversions.emplace_back(fromType, toType);

    // Продолжаем обход вглубь AST
    return RecursiveASTVisitor::VisitImplicitCastExpr(expr);
  }

private:
  clang::ASTContext *m_context;
  std::vector<std::pair<std::string, std::string>> m_conversions; // Вектор для сохранения порядка
};

class ImplicitConversionConsumer : public clang::ASTConsumer {
public:
  explicit ImplicitConversionConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  ImplicitConversionVisitor m_visitor;
};

class ImplicitConversionAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ImplicitConversionConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitConversionAction>
    X("implicit_conversion_plugin", "Counts implicit type conversions");