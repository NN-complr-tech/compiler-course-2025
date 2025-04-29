#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <vector>

namespace {

class ImplicitConversionAnalyzer final
    : public clang::RecursiveASTVisitor<ImplicitConversionAnalyzer>,
      public clang::ASTConsumer {
public:
  ImplicitConversionAnalyzer(llvm::raw_ostream &outputStream)
      : OS(outputStream) {
      }

  bool VisitFunctionDecl(clang::FunctionDecl *funcDecl) {
    currentFunctionName = funcDecl->getNameAsString();
    functionList.push_back(currentFunctionName);
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *cxxExpr) {
    if (cxxExpr->getNumArgs() < 1) {
      return true;
    }

    clang::QualType sourceType = cxxExpr->getArg(0)->getType();
    clang::QualType destType = cxxExpr->getType();

    if (sourceType == destType) {
      return true;
    }
    conversionMap[currentFunctionName][std::make_pair(
        sourceType.getAsString(), destType.getAsString())]++;
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *implCastExpr) {
    clang::CastKind castKind = implCastExpr->getCastKind();

    if (castKind == clang::CK_LValueToRValue ||
        castKind == clang::CK_FunctionToPointerDecay) {
      return true;
    }

    clang::QualType sourceType =
        implCastExpr->getSubExpr()->getType().getCanonicalType();
    clang::QualType destType = implCastExpr->getType().getCanonicalType();

    if (sourceType == destType) {
      return true;
    }
    conversionMap[currentFunctionName][std::make_pair(
        sourceType.getAsString(), destType.getAsString())]++;
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

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    TraverseDecl(Context.getTranslationUnitDecl());
    PrintResults();
  }

private:
  std::map<std::string, std::map<std::pair<std::string, std::string>, int>>
      conversionMap;
  std::string currentFunctionName;
  llvm::raw_ostream &OS;
  std::vector<std::string> functionList;
};

class ImplicitConversionAction final : public clang::PluginASTAction {
private:
  bool showHelp;
public:
  ImplicitConversionAction() : showHelp(false) {}
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    if (showHelp) {
      return nullptr; // Не создаем ASTConsumer, если запрошена справка
    }
    return std::make_unique<ImplicitConversionAnalyzer>(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {

    if (Args.empty()) {
      return true; // No arguments provided, continue execution
    }

    if (Args[0] == "--help") {
      llvm::outs() << "ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST: Counts implicit conversions and constructor calls.\n";
      llvm::outs() << "Usage: clang -cc1 -load <plugin_path> -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST <source_file>\n";
      llvm::outs() << "\n";
      llvm::outs() << "This plugin counts the number of implicit conversions and constructor calls for each function in the code.\n";
      llvm::outs() << "\n";
      llvm::outs() << "Example:\n";
      llvm::outs() << "  clang++ -cc1 -load ./ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST.so -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST test.cpp\n";
      showHelp = true;
      return true;
    } else {
      llvm::errs()
          << "ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST: Unknown argument: "
          << Args[0] << "\n";
      return false;
    }
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitConversionAction>
    X("ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST",
      "Counts implicit casts in function bodies");
