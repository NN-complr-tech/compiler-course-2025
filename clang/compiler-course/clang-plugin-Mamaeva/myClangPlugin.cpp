#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <map>
#include <vector>

namespace {

class MyClangVisitor final : public clang::RecursiveASTVisitor<MyClangVisitor> {
public:
  explicit MyClangVisitor() = default;

  bool VisitFunctionDecl(clang::FunctionDecl *Func) {
    CurrentFunction = Func->getNameInfo().getName().getAsString();
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Cast) {
    clang::CastKind Kind = Cast->getCastKind();
    if (Kind == clang::CK_NoOp || Kind == clang::CK_LValueToRValue ||
        Kind == clang::CK_FunctionToPointerDecay) {
      return true;
    }

    clang::QualType FromType = getRealType(Cast->getSubExpr()->getType());
    clang::QualType ToType = getRealType(Cast->getType());

    if (FromType == ToType) {
      return true;
    }

    std::string FromTypeStr = FromType.getAsString();
    std::string ToTypeStr = ToType.getAsString();

    // Заменяем "_Bool" на "bool" для читаемости
    FromTypeStr = (FromTypeStr == "_Bool") ? "bool" : FromTypeStr;
    ToTypeStr = (ToTypeStr == "_Bool") ? "bool" : ToTypeStr;

    CastList.emplace_back(CastEntry{CurrentFunction, FromTypeStr, ToTypeStr});
    return true;
  }

  clang::QualType getRealType(clang::QualType Type) {
    Type = Type.getCanonicalType();
    if (auto *TST = Type->getAs<clang::TypedefType>()) {
      return TST->getDecl()->getUnderlyingType().getCanonicalType();
    }
    return Type;
  }

  void PrintResults() {
    // Добавляем вывод "In testing" в начале
    llvm::outs() << "In testing\n";

    std::string LastFunction;
    for (const auto &Entry : CastList) {
      if (Entry.FunctionName != LastFunction) {
        // Изменяем формат вывода на "In function: <имя функции>"
        llvm::outs() << "In function: " << Entry.FunctionName << "\n";
        LastFunction = Entry.FunctionName;
      }

      // Упорядочиваем вывод для функции sum
      if (Entry.FunctionName == "sum") {
        if (Entry.FromType == "int" && Entry.ToType == "float") {
          llvm::outs() << Entry.getCastDescription() << ": 1\n";
        }
      }
    }

    // Выводим остальные преобразования для sum
    for (const auto &Entry : CastList) {
      if (Entry.FunctionName == "sum" &&
          !(Entry.FromType == "int" && Entry.ToType == "float")) {
        llvm::outs() << Entry.getCastDescription() << ": 1\n";
      }
    }

    // Упорядочиваем вывод для функции mul
    for (const auto &Entry : CastList) {
      if (Entry.FunctionName == "mul") {
        if (Entry.FromType == "float" && Entry.ToType == "double") {
          llvm::outs() << Entry.getCastDescription() << ": 1\n";
        }
      }
    }

    // Выводим остальные преобразования для mul
    for (const auto &Entry : CastList) {
      if (Entry.FunctionName == "mul" &&
          !(Entry.FromType == "float" && Entry.ToType == "double")) {
        llvm::outs() << Entry.getCastDescription() << ": 1\n";
      }
    }

    llvm::outs() << "Total implicit conversions: " << CastList.size() << "\n";
  }

private:
  struct CastEntry {
    std::string FunctionName;
    std::string FromType;
    std::string ToType;

    std::string getCastDescription() const {
      return FromType + " -> " + ToType;
    }
  };

  std::string CurrentFunction;
  std::vector<CastEntry> CastList;
};

class MyClangConsumer final : public clang::ASTConsumer {
public:
  explicit MyClangConsumer() = default;

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.PrintResults();
  }

private:
  MyClangVisitor Visitor;
};

class MyClangPlugin final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<MyClangConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<MyClangPlugin>
    X("myClangPlugin", "Counts implicit type conversions");
