#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class PrintDataVisitor final
    : public clang::RecursiveASTVisitor<PrintDataVisitor> {
  clang::ASTContext *class_context_;

  std::string AccessSpecifierToString(clang::AccessSpecifier accessSpecifier) {
    switch (accessSpecifier) {
    case clang::AS_public:
      return "public";
    case clang::AS_protected:
      return "protected";
    case clang::AS_private:
      return "private";
    default:
      llvm::errs() << "Error: Unknown access specifier\n";
      return "unknown access specifier";
    }
  }

  void PrintMember(const clang::ValueDecl *member) {
    auto &os = llvm::outs();
    os << "| |_ " << member->getNameAsString() << ' ';
    os << '(';

    if (const auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(member)) {
      os << method->getReturnType().getAsString();
      os << '|' << AccessSpecifierToString(member->getAccess());
      if (method->isStatic()) {
        os << "|static";
      }
      if (method->isVirtual()) {
        os << "|virtual";
      }
      if (method->size_overridden_methods() > 0 && !method->isPureVirtual()) {
        os << "|override";
      }
      if (method->isPureVirtual()) {
        os << "|pure";
      }
    } else {
      os << member->getType().getAsString() << '|'
         << AccessSpecifierToString(member->getAccess());
    }

    os << ")\n";
  }

public:
  explicit PrintDataVisitor(clang::ASTContext *context)
      : class_context_(context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    auto &os = llvm::outs();
    std::string fullName = declaration->getQualifiedNameAsString();
    os << fullName
       << (declaration->isStruct()  ? "(struct"
           : declaration->isUnion() ? "(union"
                                    : "(class")
       << (declaration->isTemplated() ? "|template)" : ")") << '\n';

    if (!declaration->bases().empty()) {
      for (const auto &base : declaration->bases()) {
        if (auto *baseDecl = base.getType()->getAsCXXRecordDecl()) {
          clang::AccessSpecifier accessSpecifier = base.getAccessSpecifier();
          os << declaration->getName() << " -> "
             << AccessSpecifierToString(accessSpecifier) << " "
             << baseDecl->getName() << "\n";
        }
      }
    }

    if (!declaration->field_empty()) {
      os << "|_Fields\n";
      for (const auto *decl : declaration->decls()) {
        if (auto *field = llvm::dyn_cast<clang::FieldDecl>(decl)) {
          PrintMember(field);
        }
      }
    }

    if (!declaration->methods().empty()) {
      os << "|_Methods\n";
      for (const auto *method : declaration->methods()) {
        PrintMember(method);
      }
    }
    os << '\n';
    return true;
  }
};

class PrintDataConsumer final : public clang::ASTConsumer {
public:
  explicit PrintDataConsumer(clang::ASTContext *context) : visitor_(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  PrintDataVisitor visitor_;
};

class PrintDataAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<PrintDataConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<PrintDataAction>
    X("PrintDataPlugin", "Print information about a custom data type");
