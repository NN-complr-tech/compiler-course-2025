#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
class ImplicitVisitor final
    : public clang::RecursiveASTVisitor<ImplicitVisitor> {
public:
  ImplicitVisitor() noexcept {}

  bool VisitFunctionDecl(clang::FunctionDecl *Func) {
    FunctionToCheck = Func->getNameInfo().getName().getAsString();
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *Ctor) {
    if (!Ctor || Ctor->getNumArgs() == 0) {
      return true;
    }

    const auto *FirstArg = Ctor->getArg(0);
    if (!FirstArg) {
      return true;
    }

    clang::QualType FromType = FirstArg->getType();
    clang::QualType ToType = Ctor->getType();

    static std::unordered_map<std::string, bool> TypeComparisonCache;
    std::string FromStr = FromType.getAsString();
    std::string ToStr = ToType.getAsString();

    if (TypeComparisonCache.count(FromStr + ToStr) == 0) {
      TypeComparisonCache[FromStr + ToStr] = (FromType == ToType);
    }

    if (TypeComparisonCache[FromStr + ToStr]) {
      return true;
    }

    CastInfo.emplace_back(FunctionToCheck, std::move(FromStr),
                          std::move(ToStr));
    return true;
  }

  clang::QualType getRealType(clang::QualType Type) {
    while (Type->getAs<clang::TypedefType>()) {
      Type = Type->getAs<clang::TypedefType>()->getDecl()->getUnderlyingType();
    }
    return Type.getCanonicalType();
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Cast) {
    if (!Cast) {
      return true;
    }

    if (const auto *SubExpr = Cast->getSubExpr()) {
      clang::CastKind Kind = Cast->getCastKind();
      if ((Kind == clang::CK_NoOp) || (Kind == clang::CK_LValueToRValue) ||
          (Kind == clang::CK_FunctionToPointerDecay)) {
        return true;
      }

      clang::QualType FromType = getRealType(SubExpr->getType());
      clang::QualType ToType = getRealType(Cast->getType());

      if (!FromType.isNull() && !ToType.isNull() &&
          FromType.getTypePtr() != ToType.getTypePtr()) {
        CastInfo.emplace_back(FunctionToCheck, FromType.getAsString(),
                              ToType.getAsString());
      }
    }
    return true;
  }

  std::string getCastDescription(const std::string &from,
                                 const std::string &to) {
    return from + " -> " + to;
  }

  void CastsResult() {
    std::string LastFunction;
    for (const auto &Entry : CastInfo) {
      if (std::get<0>(Entry) != LastFunction) {
        llvm::outs() << "Function " << std::get<0>(Entry) << "\n";
        LastFunction = std::get<0>(Entry);
      }
      llvm::outs() << getCastDescription(std::get<1>(Entry), std::get<2>(Entry))
                   << ": 1\n";
    }
    llvm::outs() << "Summary of total conversions: " << CastInfo.size() << "\n";
  }

private:
  std::string FunctionToCheck;
  std::vector<std::tuple<std::string, std::string, std::string>> CastInfo;
};

class ImplicitCastConsumer final : public clang::ASTConsumer {
public:
  explicit ImplicitCastConsumer() noexcept {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.CastsResult();
  }

private:
  ImplicitVisitor Visitor;
};

class ImplicitCastAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<ImplicitCastConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitCastAction>
X("ClangAST_Lab_ShulpinIlya_FIIT1_ClangAST",
  "Counts implicit type conversions");
