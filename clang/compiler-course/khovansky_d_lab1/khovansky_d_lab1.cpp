#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <vector>

namespace {

class ImplicitConvVisitor final
    : public clang::RecursiveASTVisitor<ImplicitConvVisitor> {

  void HandleTypeConversion(const clang::QualType &from_type, // NOLINT
                            const clang::QualType &to_type) { // NOLINT
    std::string from_type_str = from_type.getAsString();
    std::string to_type_str = to_type.getAsString();

    // ради читаемости
    from_type_str = (from_type_str == "_Bool") ? "bool" : from_type_str;
    to_type_str = (to_type_str == "_Bool") ? "bool" : to_type_str;

    if (from_type_str == to_type_str)
      return;

    std::string conversion = from_type_str + " -> " + to_type_str;
    auto &conv_list = conversions[current_function];

    for (auto &entry : conv_list) {
      if (entry.first == conversion) {
        entry.second++;
        return;
      }
    }

    conv_list.push_back({conversion, 1});
  }

public:
  explicit ImplicitConvVisitor(clang::ASTContext *astcontext)
      : context(astcontext) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) { // NOLINT
    current_function = func->getNameInfo().getName().getAsString();
    if (conversions.find(current_function) == conversions.end()) {
      function_order.push_back(current_function);
    }
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *expr) { // NOLINT
    if (expr->getNumArgs() == 1) {
      clang::QualType from_type = expr->getArg(0)->getType();
      clang::QualType to_type = expr->getType();
      HandleTypeConversion(from_type, to_type);
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) { // NOLINT
    switch (cast->getCastKind()) {
    case clang::CK_NoOp:
    case clang::CK_LValueToRValue:
    case clang::CK_FunctionToPointerDecay:
    case clang::CK_ArrayToPointerDecay:
      return true;
    default:
      break;
    }

    clang::QualType from_type = cast->getSubExpr()->getType();
    clang::QualType to_type = cast->getType();
    HandleTypeConversion(from_type, to_type);
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *varDecl) { // NOLINT
    if (!current_function.empty())
      return true;

    clang::QualType from_type = varDecl->getType();
    clang::QualType to_type = varDecl->getType();

    HandleTypeConversion(from_type, to_type);
    return true;
  }

  void PrintResults() { // NOLINT
    auto &os = llvm::outs();

    os << "In global scope:\n";
    for (const auto &[c1, c2] : conversions["global_scope"]) {
      os << "  " << c1 << ": " << c2 << "\n";
    }

    for (const auto &func_name : function_order) {
      os << "Function: " << func_name << "\n";
      for (const auto &[c1, c2] : conversions[func_name]) {
        os << "  " << c1 << ": " << c2 << "\n";
      }
      os << "\n";
    }
  }

private:
  clang::ASTContext *context;
  std::string current_function = "global_scope";
  std::vector<std::string> function_order;
  std::map<std::string, std::vector<std::pair<std::string, int>>> conversions;
};

class ConversionConsumer final : public clang::ASTConsumer {
public:
  explicit ConversionConsumer(clang::ASTContext *context) : Visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
    Visitor.PrintResults();
  }

private:
  ImplicitConvVisitor Visitor;
};

class ConversionAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ConversionConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ConversionAction>
    X("ImplicitConvPlugin", "Output the number of implicit conversions in the "
                            "entire file, including global scope");
