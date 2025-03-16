#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class MyUnVarsVisitor final
    : public clang::RecursiveASTVisitor<MyUnVarsVisitor> {
public:
  explicit MyUnVarsVisitor(clang::ASTContext *context, clang::Rewriter &R)
      : m_context(context), TheRewriter(R) {}

  bool VisitParamVarDecl(clang::ParmVarDecl *parametr) {
    if (!parametr->isUsed() && !parametr->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = parametr->getSourceRange().getBegin();
      TheRewriter.InsertText(loc, "[[maybe_unused]] ", true, true);
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *variable) {
    if (!variable->isUsed() && !variable->hasAttr<clang::UnusedAttr>()) {
      clang::SourceLocation loc = variable->getSourceRange().getBegin();
      TheRewriter.InsertText(loc, "[[maybe_unused]] ", true, true);
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &TheRewriter;
};

class MyUnVarsConsumer final : public clang::ASTConsumer {
public:
  MyUnVarsConsumer(clang::ASTContext *context, clang::Rewriter &R)
      : Visitor(context, R) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  MyUnVarsVisitor Visitor;
};

class MyUnVarsAction final : public clang::PluginASTAction {
private:
  clang::Rewriter TheRewriter;

public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MyUnVarsConsumer>(&CI.getASTContext(), TheRewriter);
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "-help") {
        PrintHelp(llvm::outs());
        // Returning false stops further processing after printing help.
        return false;
      }
    }
    return true;
  }

  void PrintHelp(llvm::raw_ostream &ros) {
    ros << "Usage: -plugin grudzin_k_UnVars_plugin [options]\n"
        << "This plugin marks unused variables by adding the attribute "
           "[[maybe_unused]].\n"
        << "Options:\n"
        << "  -help    : Display this help message.\n";
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<MyUnVarsAction>
    X("grudzin_k_UnVars_plugin",
      "This plugin marks unused variables by adding attribute and adding "
      "[[maybe_unused]] flag in code");
