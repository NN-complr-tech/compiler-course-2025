#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/STLExtras.h"

namespace {

std::string getAccessModifier(clang::AccessSpecifier am){
  if(am == clang::AS_public) return "|public";
  else if(am == clang::AS_private) return "|private";
  else if(am == clang::AS_protected) return "|protected";
  return "";
}

class UserDataTypeVisitor final : public clang::RecursiveASTVisitor<UserDataTypeVisitor> {
public:
  explicit UserDataTypeVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *rd) {
    auto &os = llvm::outs();

    os << rd->getNameAsString();
    if(rd->getNumBases()) {
      os << " -> ";
      llvm::interleave(
        rd->bases(),
        os,
        [&](const clang::CXXBaseSpecifier &x) { 
          os << x.getType().getAsString(); 
        },
        ","
      );
    }
    os << "\n";

    os << "|_Fields\n";
    for(const auto &f : rd->fields()) {
      os << "| |_ " << f->getName() << " (" << f->getType().getAsString();
      os << getAccessModifier(f->getAccess());
      os << ")\n";
    }
    if(rd->fields().empty())
      os << "| |_ (has no fields)\n";

    os << "|\n";

    os << "|_Methods\n";
    for(const auto &m : rd->methods()) {
      if(m->isImplicit()) continue;

      os << "| |_ " << m->getNameAsString() << " (" << m->getReturnType().getAsString() << "(";
      if(m->getNumParams()) {
        llvm::interleave(
          m->parameters(),
          os,
          [&](const clang::ParmVarDecl* x) {
            os << x->getType().getAsString();
          },
          ","
        );
      }
      os << ")";
      os << getAccessModifier(m->getAccess());

      if(m->hasAttr<clang::OverrideAttr>()) os << "|override";
      else if(m->isVirtual()) os << "|virtual";
      if(m->isPureVirtual()) os << "|pure";
      if(m->isStatic()) os << "|static";

      os << ")\n";
    }
    if(rd->methods().empty())
      os << "| |_ (has no methods)\n";

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
