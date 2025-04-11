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

  void HandleTypeConversion(const clang::QualType &FromType,
                            const clang::QualType &ToType) {
    std::string FromTypeStr = FromType.getAsString();
    std::string ToTypeStr = ToType.getAsString();

    // ради читаемости
    FromTypeStr = (FromTypeStr == "_Bool") ? "bool" : FromTypeStr;
    ToTypeStr = (ToTypeStr == "_Bool") ? "bool" : ToTypeStr;

    if (FromTypeStr == ToTypeStr)
      return;

    std::string Conversion = FromTypeStr + " -> " + ToTypeStr;
    auto &ConvList = Conversions[CurrentFunction];

    for (auto &entry : ConvList) {
      if (entry.first == Conversion) {
        entry.second++;
        return;
      }
    }

    ConvList.push_back({Conversion, 1});
  }

public:
  explicit ImplicitConvVisitor(clang::ASTContext *Context) : Context(Context) {}

  bool VisitFunctionDecl(clang::FunctionDecl *Func) {
    CurrentFunction = Func->getNameInfo().getName().getAsString();
    if (Conversions.find(CurrentFunction) == Conversions.end()) {
      FunctionOrder.push_back(CurrentFunction);
    }
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *Expr) {
    if (Expr->getNumArgs() == 1) {
      clang::QualType FromType = Expr->getArg(0)->getType();
      clang::QualType ToType = Expr->getType();
      HandleTypeConversion(FromType, ToType);
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Cast) {
    switch (Cast->getCastKind()) {
    case clang::CK_NoOp:
    case clang::CK_LValueToRValue:
    case clang::CK_FunctionToPointerDecay:
    case clang::CK_ArrayToPointerDecay:
      return true;
    default:
      break;
    }

    clang::QualType FromType = Cast->getSubExpr()->getType();
    clang::QualType ToType = Cast->getType();
    HandleTypeConversion(FromType, ToType);
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VarDecl) {
    if (!CurrentFunction.empty())
      return true;

    clang::QualType FromType = VarDecl->getType();
    clang::QualType ToType = VarDecl->getType();

    HandleTypeConversion(FromType, ToType);
    return true;
  }

  void printResults() {
    auto &Os = llvm::outs();

    Os << "In global scope:\n";
    for (const auto &[c1, c2] : Conversions["global_scope"]) {
      Os << "  " << c1 << ": " << c2 << "\n";
    }

    for (const auto &funcName : FunctionOrder) {
      Os << "Function: " << funcName << "\n";
      for (const auto &[c1, c2] : Conversions[funcName]) {
        Os << "  " << c1 << ": " << c2 << "\n";
      }
      Os << "\n";
    }
  }

private:
  clang::ASTContext *Context;
  std::string CurrentFunction = "global_scope";
  std::vector<std::string> FunctionOrder;
  std::map<std::string, std::vector<std::pair<std::string, int>>> Conversions;
};

class ConversionConsumer final : public clang::ASTConsumer {
public:
  explicit ConversionConsumer(clang::ASTContext *Context) : Visitor(Context) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.printResults();
  }

private:
  ImplicitConvVisitor Visitor;
};

class ConversionAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Ci, llvm::StringRef) override {
    return std::make_unique<ConversionConsumer>(&Ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &Ci,
                 const std::vector<std::string> &Args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ConversionAction>
    X("ImplicitConvPlugin", "Output the number of implicit Conversions in the "
                            "entire file, including global scope");
