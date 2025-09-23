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
    auto &outs = llvm::outs();

    if (record->isImplicit() || record->getName().empty()) {
      return true;
    }

    outs << record->getName();

    if (record->getNumBases() > 0) {
      outs << " -> ";
      bool firstBase = true;
      for (const auto &base : record->bases()) {
        if (!firstBase) {
          outs << ", ";
        }
        auto *baseDecl = base.getType()->getAsCXXRecordDecl();
        if (baseDecl && !baseDecl->getName().empty()) {
          outs << baseDecl->getName();
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

          outs << "| |_ " << methodName << " ("
               << method->getReturnType().getAsString() << "()|"
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

  PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<TypeInfoAction>
    X("type_info_plugin", "Prints information about user-defined types");
