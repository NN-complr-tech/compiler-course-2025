#include <sstream>
#include <string>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {

std::string accessString(clang::AccessSpecifier as) {
  switch (as) {
  case clang::AS_public:
    return "public";
  case clang::AS_protected:
    return "protected";
  case clang::AS_private:
    return "private";
  default:
    return "";
  }
}

class TypeExplorer final : public clang::RecursiveASTVisitor<TypeExplorer> {
public:
  explicit TypeExplorer(clang::ASTContext *) {}

  bool analyzeRecord(clang::CXXRecordDecl *record) {
    if (!record->isThisDeclarationADefinition() || record->isImplicit())
      return true;

    auto &output = llvm::outs();
    output << record->getName();

    if (record->getNumBases()) {
      output << " -> ";
      bool initial = true;
      for (const clang::CXXBaseSpecifier &base : record->bases()) {
        if (!initial)
          output << ", ";
        initial = false;
        output << base.getType()->getAsCXXRecordDecl()->getName();
      }
    }
    output << "\n";

    output << "|_Fields\n";
    bool fieldsExist = false;
    for (clang::FieldDecl *field : record->fields()) {
      fieldsExist = true;
      output << "| |_ " << field->getName() << " ("
             << field->getType().getAsString() << "|"
             << accessString(field->getAccess()) << ")\n";
    }
    if (!fieldsExist)
      output << "| |_ (no fields)\n";

    if (record->method_begin() != record->method_end()) {
      output << "|_Methods\n";
      for (clang::CXXMethodDecl *method : record->methods()) {
        if (method->isImplicit())
          continue;

        std::stringstream signatureBuilder;
        signatureBuilder << method->getReturnType().getAsString() << "(";
        for (unsigned i = 0; i < method->getNumParams(); ++i) {
          if (i > 0)
            signatureBuilder << ", ";
          signatureBuilder << method->getParamDecl(i)->getType().getAsString();
        }
        signatureBuilder << ")";

        llvm::SmallVector<std::string, 4> attributes;
        if (method->isVirtual() && !method->hasAttr<clang::OverrideAttr>())
          attributes.push_back("virtual");
        if (method->isPureVirtual())
          attributes.push_back("pure");
        if (method->hasAttr<clang::OverrideAttr>())
          attributes.push_back("override");
        if (method->isConst())
          attributes.push_back("const");

        output << "| |_ " << method->getNameAsString() << " ("
               << signatureBuilder.str() << "|"
               << accessString(method->getAccess());
        for (const auto &attr : attributes)
          output << "|" << attr;
        output << ")\n";
      }
    }
    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
    return analyzeRecord(decl);
  }
};

class TypeAnalysisConsumer final : public clang::ASTConsumer {
public:
  explicit TypeAnalysisConsumer(clang::ASTContext *ctx) : explorer(ctx) {}

  void HandleTranslationUnit(clang::ASTContext &ctx) override {
    explorer.TraverseDecl(ctx.getTranslationUnitDecl());
  }

private:
  TypeExplorer explorer;
};

class TypeAnalysisAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &compiler,
                    llvm::StringRef) override {
    return std::make_unique<TypeAnalysisConsumer>(&compiler.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<TypeAnalysisAction>
    PluginRegistration("DataTypes_Chastov_Vyacheslav_FIIT2_ClangAST",
                       "Analyzes user-defined types structure");
