#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

class AddMaybeUnusedVisitor
    : public RecursiveASTVisitor<AddMaybeUnusedVisitor> {
private:
  Rewriter &TheRewriter;

public:
  explicit AddMaybeUnusedVisitor(Rewriter &R, ASTContext *Ctx)
      : TheRewriter(R) {}

  bool VisitVarDecl(VarDecl *VD) {
    if (!VD->hasInit() || VD->isImplicit() || VD->isFunctionOrMethodVarDecl()) {
      return true;
    }

    StringRef Name = VD->getName();
    if (Name.contains("unused")) {
      SourceLocation Loc = VD->getBeginLoc();
      if (Loc.isValid()) {
        TheRewriter.InsertText(Loc, "[[maybe_unused]] ");
      }
    }

    return true;
  }
};

class AddMaybeUnusedConsumer : public ASTConsumer {
private:
  Rewriter TheRewriter;
  AddMaybeUnusedVisitor Visitor;

public:
  explicit AddMaybeUnusedConsumer(CompilerInstance &CI)
      : TheRewriter(CI.getSourceManager(), CI.getLangOpts()),
        Visitor(TheRewriter, &CI.getASTContext()) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    std::error_code EC;
    StringRef Filename = TheRewriter.getSourceMgr().getFilename(
        TheRewriter.getSourceMgr().getMainFileID());

    llvm::raw_fd_ostream OS(Filename, EC, llvm::sys::fs::OF_Text);
    if (!EC) {
      TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
          .write(OS);
    }
  }
};

class AddMaybeUnusedAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<AddMaybeUnusedConsumer>(CI);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
  }
};

static FrontendPluginRegistry::Add<AddMaybeUnusedAction>
    X("lab1_IvashchukVA_FIIT2_ClangAST",
      "Adds [[maybe_unused]] to variables containing 'unused'");