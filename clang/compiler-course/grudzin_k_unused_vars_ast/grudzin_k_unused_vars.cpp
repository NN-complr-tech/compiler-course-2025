#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class MyUnVarsVisitor final
      : public clang::RecursiveASTVisitor<MyUnVarsVisitor> {
public:
  explicit MyUnVarsVisitor(clang::ASTContext *context) : m_context(context) {}

  // (check https://compiler-explorer.com/z/M3vxY6P15 for correct type)
  bool VisitParamVarDecl(clang::ParmVarDecl *parametr) {
    if(!parametr->isUsed() && !parametr->hasAttr<clang::UnusedAttr>()){
      parametr->addAttr(clang::UnusedAttr::Create(*m_context));
    }
    return true;
  }
  bool VisitVarDecl(clang::VarDecl *variable) {
    if(!variable->isUsed() && !variable->hasAttr<clang::UnusedAttr>()){
      variable->addAttr(clang::UnusedAttr::Create(*m_context));
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
};

// You need to dump your data to change code info
class MyDumpVisitor final : public clang::RecursiveASTVisitor<MyDumpVisitor> {
public:
  explicit MyDumpVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *function) {
    function->dump();
    return true;
  }

private:
  clang::ASTContext *m_context;
};

class MyUnVarsConsumer final : public clang::ASTConsumer {
public:
  explicit MyUnVarsConsumer(clang::ASTContext *context)
      : m_visitor(context),d_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    d_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  MyUnVarsVisitor m_visitor;
  MyDumpVisitor d_visitor;
};

class MyUnVarsAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<MyUnVarsConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<MyUnVarsAction>
    X("grudzin_k_UnVars_plugin", "Description plugin");