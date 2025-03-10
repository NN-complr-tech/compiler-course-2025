#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class PrintDataVisitor final : public clang::RecursiveASTVisitor<PrintDataVisitor> {
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

  void PrintMember(const clang::ValueDecl *member, const std::string &member_type) {
    llvm::outs() << "| |_ " << member->getNameAsString() << ' ';
    if (member_type == "method") {
      llvm::outs() << '(';
      if (const auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(member)) {
        llvm::outs() << method->getReturnType().getAsString();
        llvm::outs() << '|' << AccessSpecifierToString(member->getAccess());
        if (method->isVirtual()) {
          llvm::outs() << "|virtual";
        }
        if (method->isOverloadedOperator()) {
          llvm::outs() << "|override";
        }
        if (method->isPureVirtual()) {
          llvm::outs() << "|pure";
        }
      }
    } else {
      llvm::outs() << '(' << member->getType().getAsString() << '|'
                   << AccessSpecifierToString(member->getAccess());
    }
    llvm::outs() << ")\n";
  }

public:
  explicit PrintDataVisitor(clang::ASTContext *context) : class_context_(context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    llvm::outs() << declaration->getNameAsString()
                 << (declaration->isStruct() ? "(struct" : "(class")
                 << (declaration->isTemplated() ? "|template)" : ")") << '\n';

    if (!declaration->bases().empty()) {
      for (const auto &base : declaration->bases()) {
        const clang::CXXRecordDecl *baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl) {
          clang::AccessSpecifier accessSpecifier = base.getAccessSpecifier();
          llvm::outs() << declaration->getName() << " -> "
                       << AccessSpecifierToString(accessSpecifier) << " "
                       << baseDecl->getName() << "\n";
        }
      }
    }

    if (!declaration->field_empty()) {
      llvm::outs() << "|_Fields\n";
      for (const auto *decl : declaration->decls()) {
        if (auto *field = llvm::dyn_cast<clang::FieldDecl>(decl)) {
          PrintMember(field, "field");
        }
      }
    }

    if (!declaration->methods().empty()) {
      llvm::outs() << "|_Methods\n";
      for (const auto *method : declaration->methods()) {
        PrintMember(method, "method");
      }
    }
    llvm::outs() << '\n';
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
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<PrintDataConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci, const std::vector<std::string> &args) override {
    return true;
  }
};

}  // namespace

static clang::FrontendPluginRegistry::Add<PrintDataAction>
    X("PrintDataPlugin", "Print information about a custom data type");