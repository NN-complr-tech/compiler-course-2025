#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class ClassInfoVisitor final
    : public clang::RecursiveASTVisitor<ClassInfoVisitor> {
public:
  explicit ClassInfoVisitor(clang::ASTContext *context)
      : m_context(context), m_os(llvm::outs()) {}

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
    m_os << userType->getNameAsString() << ' '
         << (userType->isStruct() ? "(struct" : "(class")
         << (userType->isTemplated() ? "|template)" : ")") << '\n';
  }

  void printBaseClasses(const clang::CXXRecordDecl *declaration) {
    if (declaration->getNumBases() == 0)
      return;

    m_os << "|_Base Classes: ";

    llvm::interleaveComma(
        declaration->bases(), m_os, [&](const clang::CXXBaseSpecifier &base) {
          if (auto baseDecl = base.getType()->getAsCXXRecordDecl())
            m_os << baseDecl->getNameAsString();
        });

    m_os << "\n";
  }

  void printFields(const clang::CXXRecordDecl *declaration) {
    m_os << "|_Fields\n";

    bool hasFields = false;
    for (const auto *field : declaration->fields()) {
      hasFields = true;
      m_os << "| |_ " << field->getNameAsString() << " ("
           << field->getType().getAsString() << "|"
           << getAccessSpecifierAsString(field) << ")\n";
    }

    for (const auto *decl : declaration->decls()) {
      if (const auto *varDecl = llvm::dyn_cast<clang::VarDecl>(decl)) {
        if (varDecl->isStaticDataMember()) {
          hasFields = true;
          m_os << "| |_ " << varDecl->getNameAsString() << " ("
               << varDecl->getType().getAsString() << "|"
               << getAccessSpecifierAsString(varDecl) << "|static)\n";
        }
      }
    }

    if (!hasFields)
      m_os << "| |_ (no fields)\n";
  }

  void printMethods(const clang::CXXRecordDecl *declaration) {
    if (declaration->method_begin() == declaration->method_end())
      return;

    m_os << "|_Methods\n";

    for (const auto *method : declaration->methods()) {
      if (method->isImplicit())
        continue;

      m_os << "| |_ " << method->getNameAsString() << " ("
           << method->getReturnType().getAsString() << "|"
           << getAccessSpecifierAsString(method);

      if (method->isVirtual())
        m_os << "|virtual";
      if (method->isPureVirtual())
        m_os << "|pure";
      if (method->isConst())
        m_os << "|const";

      m_os << ")\n";
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
  llvm::raw_ostream &m_os;
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
    X("ClassInfoVisitorPlugin_Ionova_Ekaterina_FIIT1_ClangAST",
      "Prints detailed info about user-defined types");
