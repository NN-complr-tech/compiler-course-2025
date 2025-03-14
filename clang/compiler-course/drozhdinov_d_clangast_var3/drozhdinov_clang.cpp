#include <map>
#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using llvm::outs;
using std::map;
using std::string;
using std::vector;

namespace {
class CastCounter final : public clang::RecursiveASTVisitor<CastCounter> {
public:
  CastCounter() = default;
  bool VisitFunctionDecl(clang::FunctionDecl *Expr) {
    CurrentFunction = Expr->getNameAsString();
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *Expr) {
    if (Expr->getNumArgs() < 1) {
      return true;
    }

    clang::QualType SourceType = Expr->getArg(0)->getType();
    clang::QualType DestType = Expr->getType();

    if (SourceType == DestType) {
      return true;
    }
    CastMap[CurrentFunction]
           [std::make_pair(SourceType.getAsString(), DestType.getAsString())]++;
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Expr) {
    clang::CastKind Kind = Expr->getCastKind();

    if (Kind == clang::CK_LValueToRValue ||
        Kind == clang::CK_FunctionToPointerDecay) {
      return true;
    }

    clang::QualType SourceType =
        Expr->getSubExpr()->getType().getCanonicalType();
    clang::QualType DestType = Expr->getType().getCanonicalType();

    if (SourceType == DestType) {
      return true;
    }
    CastMap[CurrentFunction]
           [std::make_pair(SourceType.getAsString(), DestType.getAsString())]++;
    return true;
  }

  void getResult() {
    for (auto iter = CastMap.rbegin(); iter != CastMap.rend(); iter++) {
      outs() << "Function " << iter->first << "\n";
      for (auto [cast, val] : iter->second) {
        outs() << cast.first << " -> " << cast.second << ": " << val << "\n";
      }
    }
  }

private:
  map<string, map<std::pair<string, string>, int>> CastMap;
  string CurrentFunction;
};

class CastCounterConsumer final : public clang::ASTConsumer {
private:
  CastCounter cc;

public:
  CastCounterConsumer() = default;

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    cc.TraverseDecl(Context.getTranslationUnitDecl());
    cc.getResult();
  }
};

class CastCounterAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<CastCounterConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const vector<string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<CastCounterAction>
    X("CastCounter_DrozhdinovD_FIIT1_ClangAST",
      "Detects and counts implicit casts in function bodies and constructor "
      "conversions");
