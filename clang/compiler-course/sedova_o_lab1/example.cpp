#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <string>

using namespace clang;

namespace {

class ExampleAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    return std::make_unique<ImplicitCastConsumer>(&CI.getASTContext());
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void PrintHelp(llvm::raw_ostream &ros) { ros << "Help for Lab1 plugin\n"; }

private:
  class ImplicitCastVisitor : public RecursiveASTVisitor<ImplicitCastVisitor> {
  public:
    explicit ImplicitCastVisitor(ASTContext *Context)
        : Context(Context), CurrentFunction(nullptr) {}

    bool VisitFunctionDecl(FunctionDecl *FD) {
      if (FD->hasBody()) {
        FD->dump();
      }
      return true;
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
      if (!CurrentFunction)
        return true;

      auto castKind = ICE->getCastKind();
      std::string castStr = castKindToString(castKind);
      if (castStr.empty())
        return true;

      QualType srcType = ICE->getSubExpr()->getType();
      QualType dstType = ICE->getType();
      if (Context->hasSameType(srcType, dstType))
        return true;

      std::string srcTypeStr = srcType.getAsString();
      std::string dstTypeStr = dstType.getAsString();

      std::string key = srcTypeStr + " -> " + dstTypeStr;
      CastCounts[key]++;

      return true;
    }

  private:
    ASTContext *Context;
    FunctionDecl *CurrentFunction;
    std::map<std::string, unsigned> CastCounts;

    std::string castKindToString(CastKind kind) {
      switch (kind) {
      case CK_IntegralToFloating:
        return "IntegralToFloating";
      case CK_FloatingToIntegral:
        return "FloatingToIntegral";
      case CK_FloatingCast:
        return "FloatingCast";
      case CK_IntegralCast:
        return "IntegralCast";
      case CK_NoOp:
        return "";
      default:
        return "";
      }
    }
  };

  class ImplicitCastConsumer : public ASTConsumer {
  public:
    explicit ImplicitCastConsumer(ASTContext *Context) : Visitor(Context) {}

    virtual void HandleTranslationUnit(ASTContext &Context) override {
      Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

  private:
    ImplicitCastVisitor Visitor;
  };
};

} // namespace

static FrontendPluginRegistry::Add<ExampleAction> X("Lab1",
                                                    "Description plugin");
