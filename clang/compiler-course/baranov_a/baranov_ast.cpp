#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <string>

namespace baranov_ast_lab_1 {
std::string getQualifier(clang::AccessSpecifier temp) {
  if (temp == clang::AS_public)
    return "public";
  else if (temp == clang::AS_private)
    return "private";
  else if (temp == clang::AS_protected)
    return "protected";
  return "";
}

bool hasAnyField(const clang::CXXRecordDecl *recordDecl) {
  for (const auto *Decl : recordDecl->decls()) {
    if (llvm::isa<clang::FieldDecl>(Decl))
      return true;
    if (const auto *Var = llvm::dyn_cast<clang::VarDecl>(Decl)) {
      return true;
    }
  }
  return false;
}
void handleFields(const clang::CXXRecordDecl *recordDecl,
                  llvm::raw_ostream &output) {
  for (const auto *Decl : recordDecl->decls()) {
    if (const auto *Field = llvm::dyn_cast<clang::FieldDecl>(Decl)) {
      output << "| |_ " << Field->getName() << " ("
             << Field->getType().getAsString() << "|"
             << getQualifier(Field->getAccess()) << ")\n";
    } else if (const auto *Var = llvm::dyn_cast<clang::VarDecl>(Decl)) {
      std::string typeStr = Var->getType().getAsString();
      output << "| |_ " << Var->getName() << " (" << typeStr;
      if (Var->isStaticDataMember())
        output << "|static";

      output << "|" << getQualifier(Var->getAccess());

      output << ")\n";
    }
  }
}

void handleMethods(const clang::CXXRecordDecl *recordDecl,
                   llvm::raw_ostream &output) {
  if (std::none_of(recordDecl->method_begin(), recordDecl->method_end(),
                   [](const clang::CXXMethodDecl *method) {
                     return !method->isImplicit();
                   })) {
    output << "| |_ (has no methods)\n";
    return;
  }

  for (auto &&method : recordDecl->methods()) {
    if (method->isImplicit())
      continue;

    output << "| |_ " << method->getNameAsString() << " ("
           << method->getReturnType().getAsString();
    output << "(";
    llvm::interleaveComma(method->parameters(), output,
                          [](const clang::ParmVarDecl *param) {
                            llvm::outs() << param->getType().getAsString();
                          }

    );
    output << ")";
    if (method->isStatic())
      output << "|static";
    output << "|" << getQualifier(method->getAccess());
    if (recordDecl &&
        std::any_of(recordDecl->friends().begin(), recordDecl->friends().end(),
                    [method](const clang::FriendDecl *friendDecl) {
                      if (const auto *FD = friendDecl->getFriendDecl())
                        return FD == method;
                      return false;
                    }))
      output << "|friend";
    if (method->hasAttr<clang::OverrideAttr>())
      output << "|override";
    else if (method->isVirtual())
      output << (method->isPureVirtual() ? "|virtual|pure" : "|virtual");

    output << ")\n";
  }
}

class DataTypesVisitor final
    : public clang::RecursiveASTVisitor<DataTypesVisitor> {
public:
  explicit DataTypesVisitor(clang::ASTContext *context) : m_context_(context) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
    auto &&output = llvm::outs();
    // is union
    if (recordDecl->isUnion()) {
      output << recordDecl->getNameAsString() << "(union)\n";
    } else {
      // is class or whatever
      output << recordDecl->getNameAsString();
      if (recordDecl->getDescribedClassTemplate()) {
        output << "(" << (recordDecl->isStruct() ? "struct" : "class")
               << "|template)";
      } else {
        output << "(" << (recordDecl->isStruct() ? "struct" : "class") << ")";
      }

      if (recordDecl->getNumBases()) {
        output << " -> ";
        llvm::interleaveComma(recordDecl->bases(), output,
                              [&](const clang::CXXBaseSpecifier &base) {
                                output
                                    << getQualifier(base.getAccessSpecifier())
                                    << " " << base.getType().getAsString();
                              });
      }
      output << "\n";
    }
    output << "|_Fields\n";
    if (!hasAnyField(recordDecl)) {
      output << "| |_ (has no fields)\n";
    } else {
      handleFields(recordDecl, output);
    }

    output << "|_Methods\n";
    handleMethods(recordDecl, output);

    return true;
  }

private:
  clang::ASTContext *m_context_;
};

class DataTypesConsumer final : public clang::ASTConsumer {
public:
  explicit DataTypesConsumer(clang::ASTContext *context) : visitor_(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  DataTypesVisitor visitor_;
};
class DataTypesAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<DataTypesConsumer>(&ci.getASTContext());
  }
  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace baranov_ast_lab_1

static clang::FrontendPluginRegistry::Add<baranov_ast_lab_1::DataTypesAction>
    X("BaranovDataPlugin", "Traverses data types, print info and qualifiers");
