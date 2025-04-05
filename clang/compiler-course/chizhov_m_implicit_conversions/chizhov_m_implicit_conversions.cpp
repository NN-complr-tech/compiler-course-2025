#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <vector>

namespace {

class ImplicitCastVisitor final
    : public clang::RecursiveASTVisitor<ImplicitCastVisitor> {
public:
  void Conversions(const clang::QualType &FromType,
                   const clang::QualType &ToType) {
    if (FromType == ToType) {
      return;
    }

    clang::QualType canonicalFromType = FromType.getCanonicalType();
    clang::QualType canonicalToType = ToType.getCanonicalType();

    std::string fromTypeStr = canonicalFromType.getAsString();
    std::string toTypeStr = canonicalToType.getAsString();

    std::string conversion = fromTypeStr + " -> " + toTypeStr;

    bool foundFunction = false;
    for (auto &entry : converList) {
      if (entry.funcName == currentFunction) {
        bool foundConversion = false;
        for (auto &[conversionStr, count] : entry.conv) {
          if (conversionStr == conversion) {
            count++;
            foundConversion = true;
            break;
          }
        }

        if (!foundConversion) {
          entry.conv.push_back({conversion, 1});
        }

        foundFunction = true;
        break;
      }
    }

    if (!foundFunction) {
      Conv newConv;
      newConv.funcName = currentFunction;
      newConv.conv.push_back({conversion, 1});
      converList.push_back(newConv);
    }
  }

  bool VisitFunctionDecl(clang::FunctionDecl *Func) {
    currentFunction = Func->getNameInfo().getName().getAsString();
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *construct) {
    if (construct->getNumArgs() == 1) {
      clang::QualType FromType = construct->getArg(0)->getType();
      clang::QualType ToType = construct->getType();
      Conversions(FromType, ToType);
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Cast) {
    clang::CastKind Kind = Cast->getCastKind();
    if (Kind == clang::CK_NoOp || Kind == clang::CK_LValueToRValue ||
        Kind == clang::CK_FunctionToPointerDecay) {
      return true;
    }

    clang::QualType FromType = Cast->getSubExpr()->getType();
    clang::QualType ToType = Cast->getType();

    Conversions(FromType, ToType);

    return true;
  }

  void PrintResults() {
    for (const auto &Conv : converList) {
      llvm::outs() << "Function " << Conv.funcName << "\n";

      for (const auto &[conversion, count] : Conv.conv) {
        llvm::outs() << "  " << conversion << ": " << count << "\n";
      }
    }
  }

private:
  struct Conv {
    std::string funcName;
    std::vector<std::pair<std::string, int>> conv;
  };
  std::string currentFunction;
  std::vector<Conv> converList;
};

class ImplicitCastConsumer final : public clang::ASTConsumer {
public:
  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.PrintResults();
  }

private:
  ImplicitCastVisitor Visitor;
};

class ImplicitCastAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &C, llvm::StringRef) override {
    return std::make_unique<ImplicitCastConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &C,
                 const std::vector<std::string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitCastAction>
    X("Implicit_Conv_Chizhov_Maxim_FIIT3_ClangAST",
      "Counts implicit type conversions");
