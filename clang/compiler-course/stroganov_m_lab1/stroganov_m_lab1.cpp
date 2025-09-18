#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class StructureDumper : public clang::RecursiveASTVisitor<StructureDumper> {
  clang::ASTContext *context_;

  std::string ToString(clang::AccessSpecifier access) {
    switch (access) {
    case clang::AS_public:
      return "public";
    case clang::AS_protected:
      return "protected";
    case clang::AS_private:
      return "private";
    default:
      return "none";
    }
  }

  void DumpMember(const clang::ValueDecl *decl) {
    auto &out = llvm::outs();
    out << "| |_ " << decl->getNameAsString() << " (";

    if (const auto *method = llvm::dyn_cast<clang::CXXMethodDecl>(decl)) {
      out << method->getReturnType().getAsString() << '|'
          << ToString(method->getAccess());

      if (method->isStatic())
        out << "|static";
      if (method->isVirtual())
        out << "|virtual";
      if (method->isPureVirtual())
        out << "|pure";
      if (method->size_overridden_methods() > 0)
        out << "|override";

    } else {
      out << decl->getType().getAsString() << '|'
          << ToString(decl->getAccess());
    }

    out << ")\n";
  }

public:
  explicit StructureDumper(clang::ASTContext *ctx) : context_(ctx) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *record) {
    auto &out = llvm::outs();
    std::string qualifiedName = record->getQualifiedNameAsString();

    out << qualifiedName
        << (record->isStruct()  ? "(struct"
            : record->isUnion() ? "(union"
                                : "(class")
        << (record->isTemplated() ? "|template)" : ")") << "\n";

    for (const auto &base : record->bases()) {
      if (const auto *baseType = base.getType()->getAsCXXRecordDecl()) {
        out << record->getNameAsString() << " -> "
            << ToString(base.getAccessSpecifier()) << " "
            << baseType->getNameAsString() << "\n";
      }
    }

    bool printedFields = false;
    for (const auto *decl : record->decls()) {
      if (auto *field = llvm::dyn_cast<clang::FieldDecl>(decl)) {
        if (!printedFields) {
          out << "|_Fields\n";
          printedFields = true;
        }
        DumpMember(field);
      }
    }

    if (!record->methods().empty()) {
      out << "|_Methods\n";
      for (const auto *method : record->methods()) {
        DumpMember(method);
      }
    }

    out << "\n";
    return true;
  }
};

class ASTPrinterConsumer : public clang::ASTConsumer {
public:
  explicit ASTPrinterConsumer(clang::ASTContext *ctx) : dumper_(ctx) {}

  void HandleTranslationUnit(clang::ASTContext &ctx) override {
    dumper_.TraverseDecl(ctx.getTranslationUnitDecl());
  }

private:
  StructureDumper dumper_;
};

class PluginExecutor : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ASTPrinterConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<PluginExecutor>
    X("PrintDataTypePlugin", "Print information about a custom data type");
