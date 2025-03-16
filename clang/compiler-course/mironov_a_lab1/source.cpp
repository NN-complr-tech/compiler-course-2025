#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <unordered_map>
#include <utility>

namespace {
class CastVisitor final : public clang::RecursiveASTVisitor<CastVisitor> {
public:
  CastVisitor() {}

  bool VisitFunctionDecl(clang::FunctionDecl *function) {
    scope_name_ = function->getNameAsString();
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) {

    clang::CastKind cast_kind = cast->getCastKind();

    if (cast_kind == clang::CK_FunctionToPointerDecay ||
        cast_kind == clang::CK_NoOp || cast_kind == clang::CK_LValueToRValue) {
      return true;
    }

    // CanonicalType needs not to use any type qualifiers
    std::string source_type =
        cast->getSubExpr()->getType().getCanonicalType().getAsString();
    std::string target_type = cast->getType().getCanonicalType().getAsString();

    // needs if cast int->int (it is possible cuz type qualifiers);
    if (source_type == target_type) {
      return true;
    }

    casts_[scope_name_][std::make_pair(source_type, target_type)]++;
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *construct) {

    std::string target_type =
        construct->getType().getCanonicalType().getAsString();

    for (const auto *arg : construct->arguments()) {
      std::string source_type = arg->getType().getCanonicalType().getAsString();

      casts_[scope_name_][std::make_pair(source_type, target_type)]++;
    }
    return true;
  }

  void Results() {
    for (const auto &[func, table] : casts_) {
      if (func == "global_scope"){
        llvm::outs() << "In global scope\n";
      }
      else{
        llvm::outs() << "In function: " << func << '\n';
      }
      for (const auto &[types, value] : table) {
        llvm::outs() << types.first << " -> " << types.second << ": " << value
                     << '\n';
      }
    }
  }

private:
  // table: function: {type1, type2}: a number of implicit casts
  std::map<std::string, std::map<std::pair<std::string, std::string>, int>>
      casts_;
  std::string scope_name_ = "global_scope";
};

class CastConsumer final : public clang::ASTConsumer {
public:
  explicit CastConsumer() {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
    visitor_.Results();
  }

private:
  CastVisitor visitor_;
};

class CastAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<CastConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<CastAction>
    X("Lab1_AST_Mironov_Arseniy_FIIT1_ClangAST",
      "The plugin counts the number of implicit conversations and returns it");
