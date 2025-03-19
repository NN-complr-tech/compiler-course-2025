#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class TypeInfoVisitor : public clang::RecursiveASTVisitor<TypeInfoVisitor> {
public:
  explicit TypeInfoVisitor(clang::ASTContext *context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *record) {
    // Выводим имя структуры/класса
    llvm::outs() << record->getName() << "\n";

    // Выводим базовые классы
    if (record->getNumBases() > 0) {
      llvm::outs() << "|_Base Classes\n";
      for (const auto &base : record->bases()) {
        llvm::outs() << "| |_ " << base.getType()->getAsCXXRecordDecl()->getName() << "\n";
      }
    }

    // Выводим поля
    if (record->field_begin() != record->field_end()) {
      llvm::outs() << "|_Fields\n";
      for (const auto *field : record->fields()) {
        llvm::outs() << "| |_ " << field->getName() << " (" 
                     << field->getType().getAsString() << "|"
                     << getAccessSpecifierString(field->getAccess()) << ")\n";
      }
    }

    // Выводим методы
    if (record->method_begin() != record->method_end()) {
      llvm::outs() << "|_Methods\n";
      for (const auto *method : record->methods()) {
        llvm::outs() << "| |_ " << method->getNameAsString() << " ("
                     << method->getReturnType().getAsString() << "()|"
                     << getAccessSpecifierString(method->getAccess()) << "|"
                     << (method->isVirtual() ? "virtual|" : "")
                     << (method->isPureVirtual() ? "pure|" : "") // Используем isPureVirtual()
                     << (method->size_overridden_methods() > 0 ? "override" : "")
                     << ")\n";
      }
    }

    llvm::outs() << "\n";
    return true;
  }

private:
  std::string getAccessSpecifierString(clang::AccessSpecifier access) {
    switch (access) {
      case clang::AS_public:
        return "public";
      case clang::AS_protected:
        return "protected";
      case clang::AS_private:
        return "private";
      case clang::AS_none:
        return "none";
    }
    return "unknown";
  }
};

class TypeInfoConsumer : public clang::ASTConsumer {
public:
  explicit TypeInfoConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  TypeInfoVisitor m_visitor;
};

class TypeInfoAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<TypeInfoConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<TypeInfoAction>
    X("type_info_plugin", "Prints information about user-defined types");