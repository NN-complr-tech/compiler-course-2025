#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class ClassInfoVisitor final
    : public clang::RecursiveASTVisitor<ClassInfoVisitor> {
public:
  explicit ClassInfoVisitor(clang::ASTContext *context) : m_context(context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    if (!declaration->isThisDeclarationADefinition() ||
        declaration->isImplicit())
      return true;

    printClassInfo(declaration);
    printBaseClasses(declaration);
    printFields(declaration);
    printMethods(declaration);

    return true;
  }

private:
  void printClassInfo(const clang::CXXRecordDecl *userType) {
    llvm::outs() << userType->getNameAsString() << ' ';
    llvm::outs() << (userType->isStruct() ? "(struct" : "(class");
    llvm::outs() << (userType->isTemplated() ? "|template)" : ")") << '\n';
  }

  void printBaseClasses(const clang::CXXRecordDecl *declaration) {
    if (declaration->getNumBases() == 0)
      return;
    llvm::outs() << "|_Base Classes: ";
    bool first = true;
    for (const auto &base : declaration->bases()) {
      if (!first)
        llvm::outs() << ", ";
      first = false;
      if (auto baseDecl = base.getType()->getAsCXXRecordDecl())
        llvm::outs() << baseDecl->getNameAsString();
    }
    llvm::outs() << "\n";
  }

  void printFields(const clang::CXXRecordDecl *declaration) {
    llvm::outs() << "|_Fields\n";
    bool hasFields = false;
    for (const auto *field : declaration->fields()) {
      hasFields = true;
      llvm::outs() << "| |_ " << field->getNameAsString() << " ("
                   << field->getType().getAsString() << "|"
                   << getAccessSpecifierAsString(field) << ")\n";
    }

    for (const auto *decl : declaration->decls()) {
      if (const auto *varDecl = llvm::dyn_cast<clang::VarDecl>(decl)) {
        if (varDecl->isStaticDataMember()) {
          hasFields = true;
          llvm::outs() << "| |_ " << varDecl->getNameAsString() << " ("
                       << varDecl->getType().getAsString() << "|"
                       << getAccessSpecifierAsString(varDecl) << "|static)\n";
        }
      }
    }

    if (!hasFields)
      llvm::outs() << "| |_ (no fields)\n";
  }

  void printMethods(const clang::CXXRecordDecl *declaration) {
    if (declaration->method_begin() == declaration->method_end())
      return;
    llvm::outs() << "|_Methods\n";
    for (const auto *method : declaration->methods()) {
      if (method->isImplicit())
        continue;

      llvm::outs() << "| |_ " << method->getNameAsString() << " ("
                   << method->getReturnType().getAsString() << "|"
                   << getAccessSpecifierAsString(method);
      if (method->isVirtual())
        llvm::outs() << "|virtual";
      if (method->isPureVirtual())
        llvm::outs() << "|pure";
      if (method->isConst())
        llvm::outs() << "|const";
      llvm::outs() << ")\n";
    }
  }

  std::string getAccessSpecifierAsString(const clang::Decl *decl) {
    if (auto *namedDecl = llvm::dyn_cast<clang::NamedDecl>(decl)) {
      switch (namedDecl->getAccess()) {
      case clang::AS_public:
        return "public";
      case clang::AS_protected:
        return "protected";
      case clang::AS_private:
        return "private";
      default:
        return "unknown";
      }
    }
    return "unknown";
  }

  clang::ASTContext *m_context;
};

class ClassInfoConsumer final : public clang::ASTConsumer {
public:
  explicit ClassInfoConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  ClassInfoVisitor m_visitor;
};

class ClassInfoAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ClassInfoConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<ClassInfoAction>
    X("classInfoPlugin", "Prints detailed info about user-defined types");
