#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {
class ImplicitConversionVisitor
    : public clang::RecursiveASTVisitor<ImplicitConversionVisitor> {
public:
  explicit ImplicitConversionVisitor(clang::ASTContext *Context)
      : MContext(Context) {}

  bool VisitFunctionDecl(clang::FunctionDecl *Func) {
    llvm::outs() << "Function `" << Func->getName() << "`\n";
    MConversions.clear();

    if (Func->hasBody()) {
      TraverseStmt(Func->getBody());
    }

    MConversions.erase(
        std::remove_if(MConversions.begin(), MConversions.end(),
                       [](const std::pair<std::string, std::string> &Conv) {
                         return Conv.first == Conv.second;
                       }),
        MConversions.end());

    std::set<std::pair<std::string, std::string>> UniqueConversions;
    MConversions.erase(
        std::remove_if(MConversions.begin(), MConversions.end(),
                       [&UniqueConversions](
                           const std::pair<std::string, std::string> &Conv) {
                         return !UniqueConversions.insert(Conv).second;
                       }),
        MConversions.end());

    for (auto It = MConversions.rbegin(); It != MConversions.rend(); ++It) {
      llvm::outs() << It->first << " -> " << It->second << ": 1\n";
    }

    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Expr) {
    auto FromType = Expr->getSubExpr()->getType().getAsString();
    auto ToType = Expr->getType().getAsString();

    FromType = normalizeType(FromType, ToType);
    ToType = normalizeType(ToType, FromType);

    MConversions.emplace_back(FromType, ToType);

    return RecursiveASTVisitor::VisitImplicitCastExpr(Expr);
  }

private:
  clang::ASTContext *MContext;
  std::vector<std::pair<std::string, std::string>> MConversions;

  std::string normalizeType(const std::string &Type,
                            const std::string &OtherType) {
    std::string Normalized = Type;

    size_t PtrPos = Normalized.find("(*)");
    if (PtrPos != std::string::npos) {
      Normalized.erase(PtrPos, 3); // Удаляем "(*)"
    }

    std::string WithoutModifiers = Normalized;
    WithoutModifiers.erase(
        std::remove(WithoutModifiers.begin(), WithoutModifiers.end(), '&'),
        WithoutModifiers.end());
    WithoutModifiers.erase(
        std::remove(WithoutModifiers.begin(), WithoutModifiers.end(), '*'),
        WithoutModifiers.end());

    if (WithoutModifiers == OtherType) {
      Normalized = WithoutModifiers;
    }

    Normalized.erase(std::remove(Normalized.begin(), Normalized.end(), ' '),
                     Normalized.end());

    return Normalized;
  }
};

class ImplicitConversionConsumer : public clang::ASTConsumer {
public:
  explicit ImplicitConversionConsumer(clang::ASTContext *Context)
      : MVisitor(Context) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    MVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  ImplicitConversionVisitor MVisitor;
};

class ImplicitConversionAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<ImplicitConversionConsumer>(&CI.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitConversionAction>
    X("implicit_conversion_plugin", "Counts implicit type conversions");