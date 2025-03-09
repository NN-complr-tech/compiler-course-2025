#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class UserDataTypeVisitor final : public clang::RecursiveASTVisitor<UserDataTypeVisitor> {
public:
  explicit UserDataTypeVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *rd) {
    llvm::outs() << rd->getNameAsString();
    for (auto it = rd->bases_begin(); it != rd->bases_end(); ++it) {
      if(it!=rd->bases_begin()) llvm::outs() << ",";
      if(it == rd->bases_begin()) llvm::outs() << " -> ";
      llvm::outs() << it->getType().getAsString();
    }
    llvm::outs() << "\n";

    llvm::outs() << "|_Fields\n";
    for(auto f : rd->fields()){
      llvm::outs() << "| |_ " << f->getName() << " (" << f->getType().getAsString();
      if(f->getAccess() == clang::AS_public) llvm::outs() << "|public";
      else if(f->getAccess() == clang::AS_private) llvm::outs() << "|private";
      else if(f->getAccess() == clang::AS_protected) llvm::outs() << "|protected";
      llvm::outs() << ")\n";
    }

    llvm::outs() << "|\n";

    llvm::outs() << "|_Methods\n";
    for(auto m : rd->methods()){
      if(m->isImplicit()) continue;

      llvm::outs() << "| |_ " << m->getNameAsString() << " (" << m->getReturnType().getAsString() << "(";
      for(size_t i=0; i < m->getNumParams(); ++i){
        if(i!=0) llvm::outs() << ",";
        llvm::outs() << m->parameters()[i]->getType().getAsString();
      }
      llvm::outs() << ")";
      if(m->getAccess() == clang::AS_public) llvm::outs() << "|public";
      else if(m->getAccess() == clang::AS_private) llvm::outs() << "|private";
      else if(m->getAccess() == clang::AS_protected) llvm::outs() << "|protected";

      if(m->hasAttr<clang::OverrideAttr>()) llvm::outs() << "|override";
      else if(m->isVirtual()) llvm::outs() << "|virtual";
      if(m->isPureVirtual())llvm::outs() << "|pure";

      llvm::outs() << ")\n";
    }

    return true;
  }

private:
  clang::ASTContext *m_context;
};

class UserDataTypeConsumer final : public clang::ASTConsumer {
public:
  explicit UserDataTypeConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  UserDataTypeVisitor m_visitor;
};

class UserDataTypeAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<UserDataTypeConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<UserDataTypeAction>
    X("UserDataTypePlugin_Kurakin_Matvey_FIIT1_ClangAST", "Print information about the user's data type");
