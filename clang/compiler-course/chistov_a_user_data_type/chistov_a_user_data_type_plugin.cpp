#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class Printer {
  std::string GetAccessSpecifier(const clang::ValueDecl *member) {
    switch (member->getAccess()) {
      case clang::AS_public:
        return "public";
      case clang::AS_protected:
        return "protected";
      case clang::AS_private:
        return "private";
      default:
        llvm::errs() << "Error: Unknown access specifier for member: "
                     << member->getNameAsString() << "\n";
        return "unknown access specifier";
    }
  }

public:
  void PrintType(clang::CXXRecordDecl *userType) {
    llvm::outs() << userType->getNameAsString() << (userType->isStruct() ? "(struct" : "(class")
                 << (userType->isTemplated() ? "|template)" : ")") << '\n';
  }

  void PrintMember(const clang::ValueDecl *member, const std::string &member_type) {
    if (member_type == "method") {
      llvm::outs() << "| |_ " << member->getNameAsString() << ' ';
      llvm::outs() << '(';
      if (const auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(member)) {
        llvm::outs() << method->getReturnType().getAsString();
        llvm::outs() << '|';
        llvm::outs() << GetAccessSpecifier(member);
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
      llvm::outs() << "| |_ " << member->getNameAsString() << ' ';
      llvm::outs() << '(' << member->getType().getAsString() << '|';
      llvm::outs() << GetAccessSpecifier(member);
    }
    llvm::outs() << ")\n";
  }
};

class PrintDataVisitor final : public clang::RecursiveASTVisitor<PrintDataVisitor> {
public:
  explicit PrintDataVisitor(clang::ASTContext *context) : class_context_(context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *declaration) {
    printer_.PrintType(declaration);
    if (!declaration->bases().empty()) {
      for (const auto &base : declaration->bases()) {
        const clang::CXXRecordDecl *baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl) {
          clang::AccessSpecifier accessSpecifier = base.getAccessSpecifier();
          std::string access;
          switch (accessSpecifier) {
            case clang::AS_public:
              access = "public";
              break;
            case clang::AS_protected:
              access = "protected";
              break;
            case clang::AS_private:
              access = "private";
              break;
            default:
              access = "unknown";
              break;
          }
          llvm::outs() << declaration->getName() << " -> "
                       << access << " " << baseDecl->getName() << "\n";
        }
      }
    }

    if (!declaration->field_empty()) {
      llvm::outs() << "|_Fields\n";
      for (const auto &decl : declaration->decls()) {
        if (auto *field = llvm::dyn_cast<clang::FieldDecl>(decl)) {
          printer_.PrintMember(field, "field");
        }
      }
    }

    if (!declaration->methods().empty()) {
      llvm::outs() << "|_Methods\n";
      for (const auto *method : declaration->methods()) {
        printer_.PrintMember(method, "method");
      }
    }
    llvm::outs() << '\n';
    return true;
  }

private:
  clang::ASTContext *class_context_;
  Printer printer_;
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