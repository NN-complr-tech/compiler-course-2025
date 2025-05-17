#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {
class MaybeUnusedVisitor final
    : public clang::RecursiveASTVisitor<MaybeUnusedVisitor> {
public:
  explicit MaybeUnusedVisitor(clang::ASTContext *context,
                              clang::Rewriter &Rewrite)
      : m_context(context), TheRewriter(Rewrite) {}

  bool VisitParmVarDecl(ParmVarDecl *pvd) {
    if (!pvd->isUsed(/*CheckUsedAttr=*/true)) {
      SourceLocation sc = pvd->getBeginLoc();
      if (sc.isValid())
        TheRewriter.InsertText(sc, "[[maybe_unused]] ", true, true);
    }
    return true;
  }

  bool VisitVarDecl(VarDecl *vd) {
    if (isa<ParmVarDecl>(vd))
      return true;
    if (!vd->isImplicit() && !vd->isUsed(/*CheckUsedAttr=*/true)) {
      SourceLocation sc = vd->getBeginLoc();
      if (sc.isValid())
        TheRewriter.InsertText(sc, "[[maybe_unused]] ", true, true);
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &TheRewriter;
};

class MaybeUnusedConsumer final : public clang::ASTConsumer {
public:
  explicit MaybeUnusedConsumer(clang::ASTContext *context, Rewriter &Rewrite)
      : Visitor(context, Rewrite) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  MaybeUnusedVisitor Visitor;
};

class MaybeUnusedAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    TheRewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<MaybeUnusedConsumer>(&ci.getASTContext(),
                                                 TheRewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    auto &SM = TheRewriter.getSourceMgr();
    auto FID = SM.getMainFileID();
    TheRewriter.getEditBuffer(FID).write(llvm::outs());
  }

private:
  clang::Rewriter TheRewriter;
};
} // namespace

static clang::FrontendPluginRegistry::Add<MaybeUnusedAction>
    X("koshkin_n_MaybeUnused_plugin",
      "Marks unused parameters/variables with [[maybe_unused]] attribute");