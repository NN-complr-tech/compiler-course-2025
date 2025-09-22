#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <map>
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

    std::map<std::pair<std::string, std::string>, int> conversionCounts;
    
    for (const auto &conv : MConversions) {
      if (conv.first != conv.second) {
        conversionCounts[conv]++;
      }
    }

    for (const auto &[conv, count] : conversionCounts) {
      llvm::outs() << conv.first << " -> " << conv.second << ": " << count << "\n";
    }

    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Expr) {
    auto FromType = Expr->getSubExpr()->getType().getAsString();
    auto ToType = Expr->getType().getAsString();

    FromType = normalizeType(FromType);
    ToType = normalizeType(ToType);

    if (FromType != ToType) {
      MConversions.emplace_back(FromType, ToType);
    }

    return RecursiveASTVisitor::VisitImplicitCastExpr(Expr);
  }

private:
  clang::ASTContext *MContext;
  std::vector<std::pair<std::string, std::string>> MConversions;

  std::string normalizeType(const std::string &Type) {
    std::string Normalized = Type;

    Normalized.erase(std::remove(Normalized.begin(), Normalized.end(), ' '),
                     Normalized.end());

    if (Normalized == "int") return "int";
    if (Normalized == "float") return "float";
    if (Normalized == "double") return "double";
    if (Normalized == "char") return "char";
    if (Normalized == "short") return "short";
    if (Normalized == "long") return "long";

    size_t ptrPos = Normalized.find('*');
    size_t refPos = Normalized.find('&');
    if (ptrPos != std::string::npos || refPos != std::string::npos) {
      size_t pos = std::min(ptrPos, refPos);
      if (pos == std::string::npos) pos = std::max(ptrPos, refPos);
      return normalizeType(Normalized.substr(0, pos));
    }

    if (Normalized.find("const") != std::string::npos) {
      std::string withoutConst = Normalized;
      size_t constPos = withoutConst.find("const");
      if (constPos != std::string::npos) {
        withoutConst.erase(constPos, 5);
        while (!withoutConst.empty() && withoutConst[0] == ' ') {
          withoutConst.erase(0, 1);
        }
        return normalizeType(withoutConst);
      }
    }
    
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