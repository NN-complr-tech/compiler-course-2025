#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <vector>
#include <algorithm>

namespace {

class MyClangVisitor final : public clang::RecursiveASTVisitor<MyClangVisitor> {
public:
  explicit MyClangVisitor() = default;

  bool VisitFunctionDecl(clang::FunctionDecl *Func) {
    CurrentFunction = Func->getNameInfo().getName().getAsString();
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *Var) {
    if (!Var->getType()->isFloatingType() && !Var->getType()->isIntegerType()) {
      return true;
    }

    if (Var->hasInit()) {
      clang::Expr *Init = Var->getInit();
      clang::QualType FromType = Init->getType();
      clang::QualType ToType = Var->getType();

      if (FromType != ToType) {
        std::string FromTypeStr = FromType.getAsString();
        std::string ToTypeStr = ToType.getAsString();

        FromTypeStr = (FromTypeStr == "_Bool") ? "bool" : FromTypeStr;
        ToTypeStr = (ToTypeStr == "_Bool") ? "bool" : ToTypeStr;

        std::string Conversion = FromTypeStr + " -> " + ToTypeStr;
        GlobalConversions.push_back(Conversion);
      }
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Cast) {
    clang::CastKind Kind = Cast->getCastKind();
    if (Kind == clang::CK_NoOp || Kind == clang::CK_LValueToRValue || Kind == clang::CK_FunctionToPointerDecay) {
      return true;
    }

    clang::QualType FromType = Cast->getSubExpr()->getType();
    clang::QualType ToType = Cast->getType();

    if (FromType == ToType) {
      return true;
    }

    std::string FromTypeStr = FromType.getAsString();
    std::string ToTypeStr = ToType.getAsString();

    FromTypeStr = (FromTypeStr == "_Bool") ? "bool" : FromTypeStr;
    ToTypeStr = (ToTypeStr == "_Bool") ? "bool" : ToTypeStr;

    std::string Conversion = FromTypeStr + " -> " + ToTypeStr;

    auto &ConvList = FunctionConversions[CurrentFunction];
    bool Found = false;
    for (auto &Entry : ConvList) {
      if (Entry.first == Conversion) {
        Entry.second++;
        Found = true;
        break;
      }
    }

    if (!Found) {
      ConvList.push_back({Conversion, 1});
    }

    return true;
  }

  void PrintResults() {
    auto &os = llvm::outs();

    // Вывод глобальных преобразований
    if (!GlobalConversions.empty()) {
      os << "In testing\n";
      for (const auto &Conv : GlobalConversions) {
        os << Conv << ": 1\n";
      }
      os << "\n";
    }

    // Вывод преобразований в функциях
    for (const auto &FuncEntry : FunctionConversions) {
      os << "In function: " << FuncEntry.first << "\n";
      for (const auto &ConvEntry : FuncEntry.second) {
        os << ConvEntry.first << ": " << ConvEntry.second << "\n";
      }
      os << "\n";
    }
  }

private:
  std::string CurrentFunction;
  std::vector<std::string> GlobalConversions;
  std::map<std::string, std::vector<std::pair<std::string, int>>> FunctionConversions;
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
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<MyClangConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &CI, const std::vector<std::string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<MyClangPlugin>
    X("myClangPlugin", "Counts implicit type conversions");
