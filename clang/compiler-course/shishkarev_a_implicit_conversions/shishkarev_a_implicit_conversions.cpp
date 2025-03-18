#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <utility>
#include <string>
#include <algorithm> // для std::remove_if и std::unique
#include <set>

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

    // Удаляем преобразования внутри одного типа (например, int -> int, float -> float)
    m_conversions.erase(
      std::remove_if(
        m_conversions.begin(),
        m_conversions.end(),
        [](const std::pair<std::string, std::string> &conv) {
          return conv.first == conv.second; // Удаляем, если типы совпадают
        }
      ),
      m_conversions.end()
    );

    // Удаляем дубликаты преобразований
    std::set<std::pair<std::string, std::string>> unique_conversions;
    m_conversions.erase(
      std::remove_if(
        m_conversions.begin(),
        m_conversions.end(),
        [&unique_conversions](const std::pair<std::string, std::string> &conv) {
          return !unique_conversions.insert(conv).second; // Удаляем, если преобразование уже было
        }
      ),
      m_conversions.end()
    );

    // Выводим результаты в обратном порядке
    for (auto it = m_conversions.rbegin(); it != m_conversions.rend(); ++it) {
      llvm::outs() << it->first << " -> " << it->second << ": 1\n";
    }

    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *expr) {
    // Получаем типы до и после преобразования
    auto fromType = expr->getSubExpr()->getType().getAsString();
    auto toType = expr->getType().getAsString();

    // Нормализуем типы (удаляем лишние модификаторы, такие как ссылки и указатели)
    fromType = normalizeType(fromType);
    toType = normalizeType(toType);

    // Сохраняем преобразование в порядке обхода
    m_conversions.emplace_back(fromType, toType);

    // Продолжаем обход вглубь AST
    return RecursiveASTVisitor::VisitImplicitCastExpr(expr);
  }

private:
  clang::ASTContext *m_context;
  std::vector<std::pair<std::string, std::string>> m_conversions; // Вектор для сохранения порядка

  // Функция для нормализации типов (удаление лишних модификаторов)
  std::string normalizeType(const std::string &type) {
    std::string normalized = type;
    // Удаляем ссылки и указатели
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '&'), normalized.end());
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '*'), normalized.end());
    // Удаляем пробелы
    normalized.erase(std::remove(normalized.begin(), normalized.end(), ' '), normalized.end());
    return normalized;
  }
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