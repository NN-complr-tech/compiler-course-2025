#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace agafeev_ast_1 {
std::string parseSpecifier(clang::AccessSpecifier AS_tmp) {
  switch (AS_tmp) {
  case clang::AS_public:
    return "public";
  case clang::AS_private:
    return "private";
  case clang::AS_protected:
    return "protected";
  default:
    return "";
  }
}

bool checkForFields(const clang::CXXRecordDecl *record) {
  for (const auto *Decl : record->decls()) {
    if (llvm::isa<clang::FieldDecl>(Decl))
      return true;
    if (const auto *Var = llvm::dyn_cast<clang::VarDecl>(Decl))
      return true;
  }
  return false;
}

void handleFields(const clang::CXXRecordDecl *record,
                  llvm::raw_ostream &output) {
  for (const auto *Decl : record->decls()) {
    if (const auto *Field = llvm::dyn_cast<clang::FieldDecl>(Decl)) {
      output << "| |_ " << Field->getName() << " ("
             << Field->getType().getAsString() << "|"
             << parseSpecifier(Field->getAccess()) << ")\n";
    } else if (const auto *Var = llvm::dyn_cast<clang::VarDecl>(Decl)) {
      std::string typeStr = Var->getType().getAsString();
      output << "| |_ " << Var->getName() << " (" << typeStr;
      if (Var->isStaticDataMember())
        output << "|static";
      output << "|" << parseSpecifier(Var->getAccess());
      output << ")\n";
    }
  }
}

void handleMethods(const clang::CXXRecordDecl *record,
                   llvm::raw_ostream &output) {
  if (std::none_of(record->method_begin(), record->method_end(),
                   [](const clang::CXXMethodDecl *method) {
                     return !method->isImplicit();
                   })) {
    output << "| |_ (has no methods)\n";
    return;
  }

  for (auto &&method : record->methods()) {
    if (method->isImplicit())
      continue;
    output << "| |_ " << method->getNameAsString() << " ("
           << method->getReturnType().getAsString();
    output << "(";
    llvm::interleaveComma(method->parameters(), output,
                          [](const clang::ParmVarDecl *param) {
                            llvm::outs() << param->getType().getAsString();
                          });
    output << ")";
    if (method->isStatic())
      output << "|static";
    output << "|" << parseSpecifier(method->getAccess());
    if (record &&
        std::any_of(record->friends().begin(), record->friends().end(),
                    [method](const clang::FriendDecl *friendD) {
                      if (const auto *FD = friendD->getFriendDecl())
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
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *record) {
    auto &&output = llvm::outs();
    // is union
    if (record->isUnion()) {
      output << record->getNameAsString() << "(union)\n";
    } else {
      // is class or whatever
      output << record->getNameAsString();
      if (record->getDescribedClassTemplate()) {
        output << "(" << (record->isStruct() ? "struct" : "class")
               << "|template)";
      } else {
        output << "(" << (record->isStruct() ? "struct" : "class") << ")";
      }
      if (record->getNumBases()) {
        output << " -> ";
        llvm::interleaveComma(
            record->bases(), output, [&](const clang::CXXBaseSpecifier &base) {
              output << parseSpecifier(base.getAccessSpecifier()) << " "
                     << base.getType().getAsString();
            });
      }
      output << "\n";
    }
    output << "|_Fields\n";
    if (!checkForFields(record)) {
      output << "| |_ (has no fields)\n";
    } else {
      handleFields(record, output);
    }
    output << "|_Methods\n";
    handleMethods(record, output);
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
} // namespace agafeev_ast_1

static clang::FrontendPluginRegistry::Add<agafeev_ast_1::DataTypesAction>
    X("AgafeevPlugin", "Traverses data types, print info and qualifiers");
