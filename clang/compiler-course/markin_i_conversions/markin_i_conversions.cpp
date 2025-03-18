#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <vector>

using namespace clang;
using namespace std;

namespace {

class ImplicitConversionAnalyzer
    : public clang::RecursiveASTVisitor<ImplicitConversionAnalyzer>,
      public clang::ASTConsumer {
public:
  ImplicitConversionAnalyzer(llvm::raw_ostream &outputStream) : OS(outputStream) {}

  bool VisitFunctionDecl(clang::FunctionDecl *funcDecl) {
    currentFunctionName = funcDecl->getNameAsString();
    functionList.push_back(currentFunctionName);
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *cxxExpr) {
    if (cxxExpr->getNumArgs() < 1) {
      return true;
    }

    QualType sourceType = cxxExpr->getArg(0)->getType();
    QualType destType = cxxExpr->getType();

    if (sourceType == destType) {
      return true;
    }
    conversionMap[currentFunctionName]
                 [make_pair(sourceType.getAsString(), destType.getAsString())]++;
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *implCastExpr) {
    CastKind castKind = implCastExpr->getCastKind();

    if (castKind == CK_LValueToRValue ||
        castKind == CK_FunctionToPointerDecay) {
      return true;
    }

    QualType sourceType =
        implCastExpr->getSubExpr()->getType().getCanonicalType();
    QualType destType = implCastExpr->getType().getCanonicalType();

    if (sourceType == destType) {
      return true;
    }
    conversionMap[currentFunctionName]
                 [make_pair(sourceType.getAsString(), destType.getAsString())]++;
    return true;
  }

  void PrintResults() {
    for (const auto &funcName : functionList) {
      OS << "Function `" << funcName << "`\n";
      for (const auto &[conversion, count] : conversionMap[funcName]) {
        OS << conversion.first << " -> " << conversion.second << ": " << count
           << "\n";
      }
    }
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    TraverseDecl(Context.getTranslationUnitDecl());
    PrintResults();
  }

private:
  map<string, map<pair<string, string>, int>> conversionMap;
  string currentFunctionName;
  llvm::raw_ostream &OS;
  vector<string> functionList;
};

class ImplicitConversionAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<ImplicitConversionAnalyzer>(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const vector<string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitConversionAction>
    X("ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST",
      "Counts implicit casts in function bodies");