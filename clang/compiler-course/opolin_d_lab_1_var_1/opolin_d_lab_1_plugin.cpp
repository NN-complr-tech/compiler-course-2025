#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
std::string getAccessLevelStr(clang::AccessSpecifier access) {
  switch (access) {
  case clang::AS_public:
    return "public";
  case clang::AS_protected:
    return "protected";
  case clang::AS_private:
    return "private";
  default:
    return "none";
  }
}

class PrintUserTypeVisitor final
    : public clang::RecursiveASTVisitor<PrintUserTypeVisitor> {
public:
  explicit PrintUserTypeVisitor(clang::ASTContext *context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Record) {
    if (!Record->isThisDeclarationADefinition() || Record->isImplicit()
        || Record->isLocalClass()) {
      return true;
    }
    llvm::raw_ostream &os = llvm::outs();
    os << Record->getNameAsString();
    if (!Record->bases().empty()) {
      os << " -> ";
      llvm::interleaveComma(
          Record->bases(), os, [&](const clang::CXXBaseSpecifier &BaseSpec) {
            const clang::RecordDecl *BaseRecordDecl = BaseSpec.getType()->getAsRecordDecl();
            if (BaseRecordDecl) {
              os << BaseRecordDecl->getNameAsString();
            } else {
              os << BaseSpec.getType().getAsString();
            }
          });
    }
    os << "\n";
    os << "|_Fields\n";
    for (const clang::FieldDecl *Field : Record->fields()) {
      os << "| |_ ";
      os << Field->getNameAsString() << " (";
      os << Field->getType().getAsString() << "|";
      os << getAccessLevelStr(Field->getAccess());
      os << ")\n";
    }
    os << "|\n";
    os << "|_Methods\n";
    for (const clang::CXXMethodDecl *Method : Record->methods()) {
      if (Method->isImplicit() || llvm::isa<clang::CXXDestructorDecl>(Method)) {
        continue;
      }
      os << "| |_ ";
      os << Method->getNameAsString() << " (";
      os << Method->getReturnType().getAsString() << "(";
      unsigned numParams = Method->getNumParams();
      for (unsigned i = 0; i < numParams; ++i) {
        os << Method->getParamDecl(i)->getType().getAsString();
        if (i < numParams - 1) {
          os << ", ";
        }
      }
      os << ")|";
      os << getAccessLevelStr(Method->getAccess());
      if (Method->isConst()) {
        os << "|const";
      }
      if (Method->isPureVirtual()) {
        os << "|virtual|pure";
      } else if (Method->hasAttr<clang::OverrideAttr>()) {
        os << "|override";
      } else if (Method->isVirtual()) {
        os << "|virtual";
      }
      os << ")\n";
    }
    return true;
  }
};

class PrintUserTypeConsumer final : public clang::ASTConsumer {
private:
  PrintUserTypeVisitor Visitor;

public:
  explicit PrintUserTypeConsumer(clang::ASTContext *context)
      : Visitor(context) {}
  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
  }
};

class PrintUserTypeAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<PrintUserTypeConsumer>(&ci.getASTContext());
  }
  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<PrintUserTypeAction>
    X("PrintUserTypeInfo",
      "Prints info about user types (fields, methods, bases)");
