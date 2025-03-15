#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/STLExtras.h>
#include <sstream>

namespace {
std::string getAccessLevel(clang::AccessSpecifier access) {
  switch (static_cast<int>(access)) {
  case clang::AccessSpecifier::AS_public:
    return "public";
  case clang::AccessSpecifier::AS_protected:
    return "protected";
  case clang::AccessSpecifier::AS_private:
    return "private";
  default:
    return "";
  }
}

class LabPluginVisitor final
    : public clang::RecursiveASTVisitor<LabPluginVisitor> {
public:
  explicit LabPluginVisitor(clang::ASTContext *context) {}

  bool VisitCXXRecordDecl(const clang::CXXRecordDecl *record) {
    if (!record->isCompleteDefinition())
      return true;

    llvm::outs() << record->getName();

    if (!record->bases().empty()) {
      llvm::outs() << " -> ";
      for (auto base = record->bases_begin(); base != record->bases_end(); 
           ++base) {
        if (base != record->bases_begin()) {
          llvm::outs() << ", ";
        }
        llvm::outs() << base->getType()->getAsCXXRecordDecl()->getName();
      }
    }

    llvm::outs() << "\n";

    llvm::outs() << "|_Friends\n";
    if (record->friend_begin() == record->friend_end()) {
      llvm::outs() << "| |_ (no friends)\n";
    } else {
      for (const auto *friendDecl : record->friends()) {
        const clang::Decl *friendDeclType = friendDecl->getFriendDecl();
        if (const auto* friendClass =
                llvm::dyn_cast<clang::CXXRecordDecl>(friendDeclType)) {
          llvm::outs() << "| |_ " << friendClass->getName() << "\n";
        } else if (const auto *friendFunction =
                       llvm::dyn_cast<clang::FunctionDecl>(friendDeclType)) {
          llvm::outs() << "| |_ " << friendFunction->getName() << "\n";
        } else {
          llvm::outs() << "| |_ (unknown friend kind)\n";
        }
      }
    }

    llvm::outs() << "|_Fields\n";
    if (record->field_empty()) {
      llvm::outs() << "| |_ (no fields)\n";
    } else {
      for (const auto *field : record->fields()) {
        llvm::outs() << "| |_ " << field->getName() << " ("
                     << field->getType().getAsString() << "|"
                     << ::getAccessLevel(field->getAccess()) << ")\n";
      }
    }

    llvm::outs() << "|_Methods\n";
    if (record->methods().empty()) {
      llvm::outs() << "| |_ (no methods)\n";
    } else {
      for (const auto *method : record->methods()) {
        if (method->isImplicit()) continue;

        llvm::SmallVector<std::string, 4> specs;

        if (method->isVirtual() && !method->hasAttr<clang::OverrideAttr>()) {
          specs.push_back("virtual");
        }
        if (method->isPureVirtual()) {
          specs.push_back("pure");
        }
        if (method->hasAttr<clang::OverrideAttr>()) {
          specs.push_back("override");
        }

        std::string oss_str;
        llvm::raw_string_ostream oss(oss_str);
        oss << method->getReturnType().getAsString() << "(";

        std::vector<std::string> paramTypes;
        for (unsigned i = 0; i < method->getNumParams(); ++i) {
          const auto *param = method->getParamDecl(i);
          paramTypes.push_back(param->getType().getAsString());
        }

        llvm::interleaveComma(paramTypes, oss,
                              [&](const std::string& type) { return type; });
        oss << ")";

        llvm::outs() << "| |_ " << method->getName() << " (" << oss.str() << "|"
                     << ::getAccessLevel(method->getAccess());

        for (const auto &s : specs) {
          llvm::outs() << "|" << s;
        }
        llvm::outs() << ")\n";
      }
    }

    llvm::outs() << "\n";

    return true;
  }
};

class LabPluginConsumer final : public clang::ASTConsumer {
public:
  explicit LabPluginConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  LabPluginVisitor m_visitor;
};

class LabPluginAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<LabPluginConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string>& args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<::LabPluginAction>
    X("LabPlugin_KolodkinGrigorii_FIIT3_ClangAST",
      "Prints info about user defined data types");