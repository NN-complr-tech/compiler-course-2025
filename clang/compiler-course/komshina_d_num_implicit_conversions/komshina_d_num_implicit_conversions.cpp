#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace {

class ImplicitConversionsAnalyzer final
    : public clang::RecursiveASTVisitor<ImplicitConversionsAnalyzer> {
public:
  ImplicitConversionsAnalyzer() {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    CurrentFunction = func->getNameAsString();
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *Conv) {
    if (Conv->getNumArgs() == 0)
      return true;
    HandleConv(Conv->getArg(0)->getType(), Conv->getType());
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *CastExpr) {
    switch (CastExpr->getCastKind()) {
    case clang::CK_NoOp:
    case clang::CK_LValueToRValue:
    case clang::CK_FunctionToPointerDecay:
      return true;
    default:
      HandleConv(CastExpr->getSubExpr()->getType(), CastExpr->getType());
      return true;
    }
  }

  void PrintReport(bool includeTotal = false) const {
    std::set<std::string> processedFunctions;
    int totalConversions = 0;

    for (const auto &entry : CastList) {
      const auto &[FunctionName, FromType, ToType] = entry;

      if (processedFunctions.find(FunctionName) == processedFunctions.end()) {
        llvm::outs() << "Function " << FunctionName << "\n";
        processedFunctions.insert(FunctionName);
      }

      llvm::outs() << FromType + " -> " + ToType << ": 1\n";
      totalConversions++;
    }

    if (includeTotal) {
      llvm::outs() << "Total implicit conversions: " << totalConversions
                   << "\n";
    }
  }

private:
  void HandleConv(clang::QualType From, clang::QualType To) {
    From = ResolveType(From);
    To = ResolveType(To);
    if (From == To)
      return;

    CastList.emplace_back(CurrentFunction, From.getAsString(),
                          To.getAsString());
  }

  clang::QualType ResolveType(clang::QualType Type) {
    Type = Type.getCanonicalType();
    if (auto *TST = Type->getAs<clang::TypedefType>()) {
      return TST->getDecl()->getUnderlyingType().getCanonicalType();
    }
    if (auto *ET = Type->getAs<clang::ElaboratedType>()) {
      return ET->getNamedType().getCanonicalType();
    }
    return Type;
  }

  std::string CurrentFunction;
  std::vector<std::tuple<std::string, std::string, std::string>> CastList;
};

class ImplicitConversionsConsumer final : public clang::ASTConsumer {
public:
  ImplicitConversionsConsumer() {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_visitor.PrintReport(true);
  }

private:
  ImplicitConversionsAnalyzer m_visitor;
};

class ImplicitConversionsAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ImplicitConversionsConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "--help") {
        llvm::errs() << "Usage: -Xclang -plugin -Xclang "
                        "Implicit_Conversions_Komshina_Daria_FIIT1_ClangAST\n"
                     << "Options:\n"
                     << "  --help     Show this help message\n";
        return false;
      }
    }
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitConversionsAction>
    X("Implicit_Conversions_Komshina_Daria_FIIT1_ClangAST",
      "Count implicit type conversions");
