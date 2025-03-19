#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <string>

namespace anikin_m_ast {

std::string getAccessSpecifierString(clang::AccessSpecifier access) {
  if (access == clang::AS_public)
    return "public";
  else if (access == clang::AS_private)
    return "private";
  else if (access == clang::AS_protected)
    return "protected";
  return "";
}

bool containsFields(const clang::CXXRecordDecl *record) {
  for (const auto *decl : record->decls()) {
    if (llvm::isa<clang::FieldDecl>(decl))
      return true;
    if (const auto *var = llvm::dyn_cast<clang::VarDecl>(decl)) {
      return true;
    }
  }
  return false;
}

void processFields(const clang::CXXRecordDecl *record, llvm::raw_ostream &out) {
  for (const auto *decl : record->decls()) {
    if (const auto *field = llvm::dyn_cast<clang::FieldDecl>(decl)) {
      out << "| |_ " << field->getName() << " ("
          << field->getType().getAsString() << "|"
          << getAccessSpecifierString(field->getAccess()) << ")\n";
    } else if (const auto *var = llvm::dyn_cast<clang::VarDecl>(decl)) {
      std::string typeStr = var->getType().getAsString();
      out << "| |_ " << var->getName() << " (" << typeStr;
      if (var->isStaticDataMember())
        out << "|static";

      out << "|" << getAccessSpecifierString(var->getAccess());

      out << ")\n";
    }
  }
}

void processMethods(const clang::CXXRecordDecl *record, llvm::raw_ostream &out) {
  if (std::none_of(record->method_begin(), record->method_end(),
                   [](const clang::CXXMethodDecl *method) {
                     return !method->isImplicit();
                   })) {
    out << "| |_ (has no methods)\n";
    return;
  }

  for (auto &&method : record->methods()) {
    if (method->isImplicit())
      continue;

    out << "| |_ " << method->getNameAsString() << " ("
        << method->getReturnType().getAsString();
    out << "(";
    llvm::interleaveComma(method->parameters(), out,
                          [](const clang::ParmVarDecl *param) {
                            llvm::outs() << param->getType().getAsString();
                          }

    );
    out << ")";
    if (method->isStatic())
      out << "|static";
    out << "|" << getAccessSpecifierString(method->getAccess());
    if (record &&
        std::any_of(record->friends().begin(), record->friends().end(),
                    [method](const clang::FriendDecl *friendDecl) {
                      if (const auto *FD = friendDecl->getFriendDecl())
                        return FD == method;
                      return false;
                    }))
      out << "|friend";
    if (method->hasAttr<clang::OverrideAttr>())
      out << "|override";
    else if (method->isVirtual())
      out << (method->isPureVirtual() ? "|virtual|pure" : "|virtual");

    out << ")\n";
  }
}

class ASTWalker final : public clang::RecursiveASTVisitor<ASTWalker> {
public:
  explicit ASTWalker(clang::ASTContext *context) : context_(context) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *record) {
    auto &&out = llvm::outs();
    // is union
    if (record->isUnion()) {
      out << record->getNameAsString() << "(union)\n";
    } else {
      // is class or whatever
      out << record->getNameAsString();
      if (record->getDescribedClassTemplate()) {
        out << "(" << (record->isStruct() ? "struct" : "class")
            << "|template)";
      } else {
        out << "(" << (record->isStruct() ? "struct" : "class") << ")";
      }

      if (record->getNumBases()) {
        out << " -> ";
        llvm::interleaveComma(record->bases(), out,
                              [&](const clang::CXXBaseSpecifier &base) {
                                out
                                    << getAccessSpecifierString(base.getAccessSpecifier())
                                    << " " << base.getType().getAsString();
                              });
      }
      out << "\n";
    }
    out << "|_Fields\n";
    if (!containsFields(record)) {
      out << "| |_ (has no fields)\n";
    } else {
      processFields(record, out);
    }

    out << "|_Methods\n";
    processMethods(record, out);

    return true;
  }

private:
  clang::ASTContext *context_;
};

class ASTAnalyzer final : public clang::ASTConsumer {
public:
  explicit ASTAnalyzer(clang::ASTContext *context) : walker_(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    walker_.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  ASTWalker walker_;
};

class ASTPluginAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ASTAnalyzer>(&ci.getASTContext());
  }
  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace anikin_m_ast

static clang::FrontendPluginRegistry::Add<anikin_m_ast::ASTPluginAction>
    Y("data_type", "Analyzes AST and prints information about data types");