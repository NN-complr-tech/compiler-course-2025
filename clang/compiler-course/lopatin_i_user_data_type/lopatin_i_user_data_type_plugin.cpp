#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace lopatin_i_user_data_type {

std::string getAccessSpelling(clang::AccessSpecifier access) {
  switch (access) {
    case clang::AS_public: return "public";
    case clang::AS_protected: return "protected";
    case clang::AS_private: return "private";
    default: return "";
  }
}

class UserDataTypeVisitor final : public clang::RecursiveASTVisitor<UserDataTypeVisitor> {
  llvm::raw_ostream &m_os;
public:
  explicit UserDataTypeVisitor(clang::ASTContext *context, llvm::raw_ostream &os) : m_os(os) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *D) {
    if (!D->isThisDeclarationADefinition() || D->isImplicit())
      return true;

    // classes
    m_os << D->getName();
    if (D->getNumBases()) {
      m_os << " -> ";
      bool first = true;
      for (const clang::CXXBaseSpecifier &B : D->bases()) {
        if (!first) m_os << ", ";
        first = false;
        m_os << B.getType()->getAsCXXRecordDecl()->getName();
      }
    }
    m_os << "\n";

    // fields
    m_os << "|_Fields\n";
    for (clang::FieldDecl *F : D->fields()) {
      m_os << "| |_ " << F->getName() << " ("
           << F->getType().getAsString() << "|"
           << lopatin_i_user_data_type::getAccessSpelling(F->getAccess()) << ")\n";
    }

    // methods
    m_os << "|_Methods\n";
    for (clang::CXXMethodDecl *M : D->methods()) {
      if (M->isImplicit()) continue;

      // method type
      std::string TypeStr;
      clang::QualType ReturnType = M->getReturnType();
      TypeStr += ReturnType.getAsString() + "(";

      for (unsigned i = 0; i < M->getNumParams(); ++i) {
        if (i > 0) TypeStr += ", ";
        TypeStr += M->getParamDecl(i)->getType().getAsString();
      }
      TypeStr += ")";

      // specifiers
      std::vector<std::string> Specs;
      if (M->isVirtual() && !M->hasAttr<clang::OverrideAttr>()) {
        Specs.push_back("virtual");
      }
      if (M->isPureVirtual()) Specs.push_back("pure");
      if (M->hasAttr<clang::OverrideAttr>()) Specs.push_back("override");
      if (M->isConst()) Specs.push_back("const");
      if (const auto *FPT = M->getType()->getAs<clang::FunctionProtoType>()) {
      if (FPT->getExceptionSpecType() == clang::EST_NoexceptTrue) {
          Specs.push_back("noexcept");
        }
      }

      m_os << "| |_ " << M->getNameAsString() << " (" << TypeStr << "|"
           << lopatin_i_user_data_type::getAccessSpelling(M->getAccess());

      for (const auto &S : Specs) {
        m_os << "|" << S;
      }
      m_os << ")\n";
    }
    return true;
  }
};

class UserDataTypeConsumer final : public clang::ASTConsumer {
public:
  explicit UserDataTypeConsumer(clang::ASTContext *context) : m_visitor(context, llvm::outs()) {}

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

} // namespace lopatin_i_user_data_type

static clang::FrontendPluginRegistry::Add<lopatin_i_user_data_type::UserDataTypeAction>
    X("UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST", "Prints info about user defined data types");
