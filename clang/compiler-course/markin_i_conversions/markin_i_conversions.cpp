#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

using namespace clang;

namespace {

class ImplicitCastCounter : public RecursiveASTVisitor<ImplicitCastCounter> {
public:
  ImplicitCastCounter(ASTContext *Context, llvm::raw_ostream &OS) : Context(Context), OS(OS), CurrentFunctionName("") {}

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->isGlobal()) {
      CurrentFunctionName = FD->getNameInfo().getName().getAsString();
      OS << "Function `" << CurrentFunctionName << "`\n";
      implicitCastCounts.clear();
    }
    return true;
  }

  bool VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
    QualType sourceType = ICE->getSubExpr()->getType();
    QualType destType = ICE->getType();

    std::string sourceTypeName = sourceType.getAsString();
    std::string destTypeName = destType.getAsString();

    implicitCastCounts[std::make_pair(sourceTypeName, destTypeName)]++;

    return true;
  }

  void EndSourceFile() override {
      for (const auto& entry : implicitCastCounts) {
          OS << entry.first.first << " -> " << entry.first.second << ": " << entry.second << "\n";
      }
  }

private:
  ASTContext *Context;
  llvm::raw_ostream &OS;
  std::string CurrentFunctionName;
  std::map<std::pair<std::string, std::string>, int> implicitCastCounts;
};

class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(ASTContext *Context, llvm::raw_ostream &OS) : Visitor(Context, OS) {}
.
  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.EndSourceFile();
  }

private:
  ImplicitCastCounter Visitor;
};

class MyPluginAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    llvm::errs() << "Running my plugin on " << file << "\n";
    return std::make_unique<MyASTConsumer>(&CI.getASTContext(), llvm::outs());
  }

  bool BeginSourceFileAction(CompilerInstance &CI, StringRef Filename) override {
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return PluginASTAction::ActionType::AddToExistingAction;
  }
};

} 

static FrontendPluginRegistry::Add<MyPluginAction>
X("implicit-cast-counter", "Counts implicit casts in the input file");