#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class UnusedVisitor final : public clang::RecursiveASTVisitor<UnusedVisitor> {
public:
  explicit UnusedVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitVarDecl(clang::VarDecl *var) {
    if (!var->isUsed() && !var->hasAttr<clang::UnusedAttr>()) {
        var->addAttr(clang::UnusedAttr::Create(*m_context));
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *param) {
    if (!param->isUsed() && !param->hasAttr<clang::UnusedAttr>()) {
        param->addAttr(clang::UnusedAttr::Create(*m_context));
    }
    return true;
  }
private:
  clang::ASTContext *m_context;
};

class DumpVisitor final : public clang::RecursiveASTVisitor<DumpVisitor> {
    public:
      explicit DumpVisitor(clang::ASTContext *context) : m_context(context) {}
      bool VisitFunctionDecl(clang::FunctionDecl *func) {
        func->dump();
        return true;
      }
    private:
      clang::ASTContext *m_context;
    };

class ExampleConsumer final : public clang::ASTConsumer {
public:
  explicit ExampleConsumer(clang::ASTContext *context) : um_visitor(context),dm_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    um_visitor.TraverseDecl(context.getTranslationUnitDecl());
    dm_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  UnusedVisitor um_visitor;
  DumpVisitor dm_visitor;
};

class ExampleAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ExampleConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<ExampleAction>
    X("unused_variable_plugin", "Description plugin");
