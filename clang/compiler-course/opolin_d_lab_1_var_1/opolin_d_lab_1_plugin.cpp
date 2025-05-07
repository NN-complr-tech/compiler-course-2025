#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class PrintUserTypeVisitor final
    : public clang::RecursiveASTVisitor<PrintUserTypeVisitor> {
public:
  explicit PrintUserTypeVisitor(clang::ASTContext *context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Record) {
    if (!Record->isThisDeclarationADefinition() || Record->isImplicit()) {
      return true;
    }
    if (Record->isLocalClass()) {
      return true;
    }
    llvm::raw_ostream &os = llvm::outs();
    os << Record->getNameAsString();
    bool firstBase = true;
    if (Record->getNumBases() > 0) {
      os << " -> ";
      for (const clang::CXXBaseSpecifier &BaseSpec : Record->bases()) {
        if (!firstBase) {
          os << ", ";
        }
        const clang::RecordDecl *BaseRecordDecl =
            BaseSpec.getType()->getAsRecordDecl();
        if (BaseRecordDecl) {
          os << BaseRecordDecl->getNameAsString();
        } else {
          os << BaseSpec.getType().getAsString();
        }
        firstBase = false;
      }
    }
    os << "\n";
    os << "|_Fields\n";
    for (const clang::FieldDecl *Field : Record->fields()) {
      os << "| |_ ";
      os << Field->getNameAsString() << " (";
      os << Field->getType().getAsString() << "|";
      switch (Field->getAccess()) {
      case clang::AS_public:
        os << "public";
        break;
      case clang::AS_protected:
        os << "protected";
        break;
      case clang::AS_private:
        os << "private";
        break;
      default:
        os << "none";
        break;
      }
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
      switch (Method->getAccess()) {
      case clang::AS_public:
        os << "public";
        break;
      case clang::AS_protected:
        os << "protected";
        break;
      case clang::AS_private:
        os << "private";
        break;
      default:
        os << "none";
        break;
      }
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
