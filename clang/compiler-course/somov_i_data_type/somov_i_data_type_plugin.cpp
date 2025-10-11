#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/STLExtras.h"

namespace {

static std::string AccessString(clang::AccessSpecifier AS) {
  switch (AS) {
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

class TypeInspector final : public clang::RecursiveASTVisitor<TypeInspector> {
public:
  explicit TypeInspector(clang::ASTContext *ctx) : context(ctx) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *RD) {
    if (!RD->isThisDeclarationADefinition() || RD->isImplicit())
      return true;

    llvm::raw_ostream &out = llvm::outs();
    out << RD->getQualifiedNameAsString();

    emitBaseList(out, RD);
    out << "\n|_Fields\n";
    emitFieldList(out, RD);
    out << "|\n|_Methods\n";
    emitMethodList(out, RD);
    out << "|\n|_Nested Types\n";
    emitNestedTypes(out, RD);
    out << "\n";

    return true;
  }

private:
  clang::ASTContext *context;

  void emitBaseList(llvm::raw_ostream &out, const clang::CXXRecordDecl *RD) {
    if (RD->getNumBases() == 0)
      return;

    llvm::SmallVector<std::string, 4> baseNames;
    for (const auto &B : RD->bases()) {
      if (auto *BD = B.getType()->getAsCXXRecordDecl())
        baseNames.push_back(BD->getQualifiedNameAsString());
    }

    if (!baseNames.empty())
      out << " -> " << llvm::join(baseNames, ", ");
  }

  void emitFieldList(llvm::raw_ostream &out, const clang::CXXRecordDecl *RD) {
    if (RD->field_empty()) {
      out << "| |_ (no fields)\n";
      return;
    }

    for (const auto *F : RD->fields()) {
      out << "| |_ " << F->getNameAsString() << " ("
          << F->getType().getAsString() << "|" << AccessString(F->getAccess())
          << ")\n";
    }
  }

  void emitMethodList(llvm::raw_ostream &out, const clang::CXXRecordDecl *RD) {
    if (RD->method_begin() == RD->method_end()) {
      out << "| |_ (no methods)\n";
      return;
    }

    for (const auto *M : RD->methods()) {
      if (M->isImplicit())
        continue;

      out << "| |_ " << M->getNameAsString() << " ("
          << M->getReturnType().getAsString() << "(";

      // Параметры через interleaveComma, выводим типы параметров
      llvm::interleaveComma(M->parameters(), out,
                            [](const clang::ParmVarDecl *P) {
                              llvm::outs() << P->getType().getAsString();
                            });

      out << ")|" << AccessString(M->getAccess());

      if (M->isVirtual() && !M->hasAttr<clang::OverrideAttr>())
        out << "|virtual";
      if (M->isPureVirtual())
        out << "|pure";
      if (M->hasAttr<clang::OverrideAttr>())
        out << "|override";
      if (M->isConst())
        out << "|const";

      out << ")\n";
    }
  }

  void emitNestedTypes(llvm::raw_ostream &out, const clang::CXXRecordDecl *RD) {
    bool anyNested = false;

    for (const auto *D : RD->decls()) {
      if (const auto *Nested = llvm::dyn_cast<clang::CXXRecordDecl>(D)) {
        if (!Nested->isThisDeclarationADefinition() || Nested->isImplicit())
          continue;

        if (!anyNested)
          anyNested = true;

        out << "| |_ " << Nested->getNameAsString() << "\n";
      }
    }

    if (!anyNested)
      out << "| |_ (no nested types)\n";
  }
};

class InspectorConsumer final : public clang::ASTConsumer {
public:
  explicit InspectorConsumer(clang::ASTContext *ctx) : inspector(ctx) {}

  void HandleTranslationUnit(clang::ASTContext &Ctx) override {
    inspector.TraverseDecl(Ctx.getTranslationUnitDecl());
  }

private:
  TypeInspector inspector;
};

class DataTypeAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<InspectorConsumer>(&CI.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    // Плагин не ожидает аргументов — всегда успешно.
    (void)CI;
    (void)args;
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<DataTypeAction>
    Y("data_type_plugin",
      "Prints information about user-defined data types: fields, methods, bases, nested types");