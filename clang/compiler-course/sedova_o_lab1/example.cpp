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

  void PrintHelp(llvm::raw_ostream &ros) {
    ros << "Lab1 plugin: This pass analyzes the source code to identify and "
           "count implicit type casts "
           "within functions. It helps developers understand where and how "
           "often implicit conversions "
           "occur, which can be useful for debugging, optimization, and code "
           "quality improvements.\n";
  }

private:
  class ImplicitCastVisitor : public RecursiveASTVisitor<ImplicitCastVisitor> {
  public:
    explicit ImplicitCastVisitor(ASTContext *Context)
        : Context(Context), CurrentFunction(nullptr), ImplicitCastCount(0),
          TotalImplicitCastCount(0) {}

    bool VisitFunctionDecl(FunctionDecl *FD) {
      if (FD->hasBody()) {
        CurrentFunction = FD;
        ImplicitCastCount = 0;
        TraverseStmt(FD->getBody());
        llvm::outs() << "Function '"
                     << FD->getNameInfo().getName().getAsString()
                     << "' contains " << ImplicitCastCount
                     << " implicit casts.\n";
        TotalImplicitCastCount += ImplicitCastCount;
        CurrentFunction = nullptr;
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

      ++ImplicitCastCount;
      return true;
    }

    void PrintTotalStatistics() const {
      llvm::outs() << "Total implicit casts in translation unit: "
                   << TotalImplicitCastCount << "\n";

      llvm::outs() << "Implicit cast breakdown by type:\n";
      for (const auto &pair : CastCounts) {
        llvm::outs() << "  " << pair.first << ": " << pair.second << "\n";
      }
    }

  private:
    ASTContext *Context;
    FunctionDecl *CurrentFunction;
    unsigned ImplicitCastCount;
    unsigned TotalImplicitCastCount;
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

    void HandleTranslationUnit(ASTContext &Context) override {
      Visitor.TraverseDecl(Context.getTranslationUnitDecl());
      Visitor.PrintTotalStatistics();
    }

  private:
    ImplicitCastVisitor Visitor;
  };
};

} // namespace

static FrontendPluginRegistry::Add<ExampleAction> X("Lab1",
                                                    "Description plugin");
