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
    MConversionOrder.clear();

    if (Func->hasBody()) {
      TraverseStmt(Func->getBody());
    }

    std::map<std::pair<std::string, std::string>, int> conversionCounts;
    for (const auto &conv : MConversions) {
      if (conv.first != conv.second && isValidConversion(conv)) {
        conversionCounts[conv]++;
      }
    }

    std::set<std::pair<std::string, std::string>> seen;
    for (const auto &conv : MConversionOrder) {
      if (conv.first != conv.second && isValidConversion(conv) && seen.insert(conv).second) {
        llvm::outs() << conv.first << " -> " << conv.second << ": " 
                     << conversionCounts[conv] << "\n";
      }
    }

    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Expr) {
    if (Expr->getCastKind() == clang::CK_FunctionToPointerDecay ||
        Expr->getCastKind() == clang::CK_ArrayToPointerDecay ||
        Expr->getCastKind() == clang::CK_LValueToRValue) {
      return RecursiveASTVisitor::VisitImplicitCastExpr(Expr);
    }

    auto FromType = Expr->getSubExpr()->getType().getAsString();
    auto ToType = Expr->getType().getAsString();

    FromType = normalizeType(FromType);
    ToType = normalizeType(ToType);

    if (FromType != ToType) {
      MConversions.emplace_back(FromType, ToType);
      MConversionOrder.emplace_back(FromType, ToType);
    }

    return RecursiveASTVisitor::VisitImplicitCastExpr(Expr);
  }

private:
  clang::ASTContext *MContext;
  std::vector<std::pair<std::string, std::string>> MConversions;
  std::vector<std::pair<std::string, std::string>> MConversionOrder;

  bool isValidConversion(const std::pair<std::string, std::string> &conv) {
    const std::set<std::string> numericTypes = {"int", "float", "double", "char", "short", "long"};
    
    return numericTypes.count(conv.first) > 0 && numericTypes.count(conv.second) > 0;
  }

  std::string normalizeType(const std::string &Type) {
    std::string Normalized = Type;

    Normalized.erase(std::remove(Normalized.begin(), Normalized.end(), ' '),
                     Normalized.end());

    if (Normalized == "int" || Normalized == "unsignedint") return "int";
    if (Normalized == "float") return "float";
    if (Normalized == "double") return "double";
    if (Normalized == "char" || Normalized == "unsignedchar") return "char";
    if (Normalized == "short" || Normalized == "unsignedshort") return "short";
    if (Normalized == "long" || Normalized == "unsignedlong") return "long";

    if (Normalized.find("const") == 0) {
      return normalizeType(Normalized.substr(5));
    }
    if (Normalized.find("unsigned") == 0) {
      return normalizeType(Normalized.substr(8));
    }

    if (Normalized.find('(') != std::string::npos ||
        Normalized.find(')') != std::string::npos ||
        Normalized.find('*') != std::string::npos ||
        Normalized.find('&') != std::string::npos) {
      return "";
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