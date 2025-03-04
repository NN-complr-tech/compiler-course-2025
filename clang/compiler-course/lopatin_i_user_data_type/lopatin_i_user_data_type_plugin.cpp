#include <string>
#include <sstream>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {

std::string getAccessSpelling(clang::AccessSpecifier access) {
  switch (access) {
    case clang::AS_public: return "public";
    case clang::AS_protected: return "protected";
    case clang::AS_private: return "private";
    default: return "";
  }
}

class UserDataTypeVisitor final : public clang::RecursiveASTVisitor<UserDataTypeVisitor> {
public:
  explicit UserDataTypeVisitor(clang::ASTContext *context) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *D) {
    if (!D->isThisDeclarationADefinition() || D->isImplicit())
      return true;

    auto& os = llvm::outs();
    os << D->getName();
	
	// classes
    if (D->getNumBases()) {
      os << " -> ";
      bool first = true;
      for (const clang::CXXBaseSpecifier &B : D->bases()) {
        if (!first) os << ", ";
        first = false;
        os << B.getType()->getAsCXXRecordDecl()->getName();
      }
    }
    os << "\n";

    // fields
    os << "|_Fields\n";
    bool hasFields = false;
    for (clang::FieldDecl *F : D->fields()) {
      hasFields = true;
      os << "| |_ " << F->getName() << " ("
         << F->getType().getAsString() << "|"
         << ::getAccessSpelling(F->getAccess()) << ")\n";
    }
    if (!hasFields) os << "| |_ (no fields)\n";

    // methods
    if (D->method_begin() != D->method_end()) {
      os << "|_Methods\n";
      for (clang::CXXMethodDecl *M : D->methods()) {
        if (M->isImplicit()) continue;

        std::stringstream TypeStr;
        TypeStr << M->getReturnType().getAsString() << "(";
        for (unsigned i = 0; i < M->getNumParams(); ++i) {
          if (i > 0) TypeStr << ", ";
          TypeStr << M->getParamDecl(i)->getType().getAsString();
        }
        TypeStr << ")";

		// specs
        llvm::SmallVector<std::string, 4> Specs;
        if (M->isVirtual() && !M->hasAttr<clang::OverrideAttr>())
          Specs.push_back("virtual");
        if (M->isPureVirtual()) Specs.push_back("pure");
        if (M->hasAttr<clang::OverrideAttr>()) Specs.push_back("override");
        if (M->isConst()) Specs.push_back("const");

        os << "| |_ " << M->getNameAsString() << " (" << TypeStr.str() << "|"
           << ::getAccessSpelling(M->getAccess());
        for (const auto &S : Specs)
          os << "|" << S;
        os << ")\n";
      }
    }
    return true;
  }
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
    X("UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST", "Prints info about user defined data types");
