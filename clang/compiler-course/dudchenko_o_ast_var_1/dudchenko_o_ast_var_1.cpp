#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class TypeInfoVisitor : public clang::RecursiveASTVisitor<TypeInfoVisitor> {
private:
  clang::ASTContext *m_context;

public:
  explicit TypeInfoVisitor(clang::ASTContext *context) : m_context(context) {
    llvm::errs() << "DEBUG: TypeInfoVisitor created\n";
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *record) {
    llvm::errs() << "DEBUG: Visiting record: "
                 << (record->getIdentifier() ? record->getNameAsString() : "(anonymous)")
                 << "\n";

    auto &outs = llvm::outs();

    if (record->isImplicit()) {
      llvm::errs() << "DEBUG: Skipping implicit record\n";
      return true;
    }

    if (!record->getIdentifier()) {
      llvm::errs() << "DEBUG: Skipping anonymous record\n";
      return true;
    }

    outs << record->getNameAsString();

    if (record->getNumBases() > 0) {
      outs << " -> ";
      bool firstBase = true;
      for (const auto &base : record->bases()) {
        if (!firstBase) {
          outs << ", ";
        }
        auto *baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl && baseDecl->getIdentifier()) {
          outs << baseDecl->getNameAsString();
        } else {
          outs << base.getType().getAsString();
        }
        firstBase = false;
      }
    }
    outs << "\n";

    if (record->field_begin() != record->field_end()) {
      outs << "|_Fields\n";
      for (const auto *field : record->fields()) {
        std::string fieldName = field->getName().str();
        if (fieldName.empty()) {
          fieldName = "(anonymous)";
        }
        outs << "| |_ " << fieldName << " (" << field->getType().getAsString()
             << "|" << getAccessSpecifierString(field->getAccess()) << ")\n";
      }
    }

    if (record->method_begin() != record->method_end()) {
      bool hasExplicitMethods = false;
      for (const auto *method : record->methods()) {
        if (!method->isImplicit() && !method->isDefaulted()) {
          hasExplicitMethods = true;
          break;
        }
      }

      if (hasExplicitMethods) {
        outs << "|_Methods\n";
        for (const auto *method : record->methods()) {
          if (method->isImplicit() || method->isDefaulted()) {
            continue;
          }

          std::string methodName = method->getNameAsString();
          if (methodName.empty()) {
            methodName = "(anonymous)";
          }

          llvm::StringRef retType =
              llvm::StringRef(method->getReturnType().getAsString()).trim();

          outs << "| |_ " << methodName << " ("
               << retType << "()|"
               << getAccessSpecifierString(method->getAccess());

          bool isVirtualMethod = method->isVirtual();
          bool isPureVirtual = method->isPureVirtual();
          bool hasOverride = method->size_overridden_methods() > 0;

          if (isVirtualMethod) {
            if (hasOverride) {
              outs << "|override";
            } else {
              outs << "|virtual";
              if (isPureVirtual) {
                outs << "|pure";
              }
            }
          }

          outs << ")\n";
        }
      }
    }

    outs << "|\n";
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
  explicit TypeInfoConsumer(clang::ASTContext *context) : m_visitor(context) {
    llvm::errs() << "DEBUG: TypeInfoConsumer created\n";
  }

  void HandleTranslationUnit(clang::ASTContext &context) override {
    llvm::errs() << "DEBUG: Handling translation unit\n";
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    llvm::errs() << "DEBUG: Finished traversal\n";
  }

private:
  TypeInfoVisitor m_visitor;
};

class TypeInfoAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    llvm::errs() << "DEBUG: Creating AST consumer\n";
    return std::make_unique<TypeInfoConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    llvm::errs() << "DEBUG: ParseArgs called\n";
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return CmdlineBeforeMainAction;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<TypeInfoAction>
    X("type_info_plugin", "Prints information about user-defined types");
