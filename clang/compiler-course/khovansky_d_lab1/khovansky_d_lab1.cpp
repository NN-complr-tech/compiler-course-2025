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

  void HandleTypeConversion(const clang::QualType &fromType,
                            const clang::QualType &toType) {
    std::string fromTypeStr = fromType.getAsString();
    std::string toTypeStr = toType.getAsString();

    // ради читаемости
    fromTypeStr = (fromTypeStr == "_Bool") ? "bool" : fromTypeStr;
    toTypeStr = (toTypeStr == "_Bool") ? "bool" : toTypeStr;

    if (fromTypeStr == toTypeStr)
      return;

    std::string conversion = fromTypeStr + " -> " + toTypeStr;
    auto &convList = conversions[currentFunction];

    for (auto &entry : convList) {
      if (entry.first == conversion) {
        entry.second++;
        return;
      }
    }

    convList.push_back({conversion, 1});
  }

public:
  explicit ImplicitConvVisitor(clang::ASTContext *context) : Context(context) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    currentFunction = func->getNameInfo().getName().getAsString();
    if (conversions.find(currentFunction) == conversions.end()) {
      functionOrder.push_back(currentFunction);
    }
    return true;
  }

  bool VisitCXXConstructExpr(clang::CXXConstructExpr *expr) {
    if (expr->getNumArgs() == 1) {
      clang::QualType fromType = expr->getArg(0)->getType();
      clang::QualType toType = expr->getType();
      HandleTypeConversion(fromType, toType);
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) {
    switch (cast->getCastKind()) {
    case clang::CK_NoOp:
    case clang::CK_LValueToRValue:
    case clang::CK_FunctionToPointerDecay:
    case clang::CK_ArrayToPointerDecay:
      return true;
    default:
      break;
    }    

    clang::QualType fromType = cast->getSubExpr()->getType();
    clang::QualType toType = cast->getType();
    HandleTypeConversion(fromType, toType);
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *varDecl) {
    if (!currentFunction.empty())
      return true;

    clang::QualType fromType = varDecl->getType();
    clang::QualType toType = varDecl->getType();

    HandleTypeConversion(fromType, toType);
    return true;
  }

  void PrintResults() {
    auto &os = llvm::outs();

    os << "In global scope:\n";
    for (const auto &[c1, c2] : conversions["global_scope"]) {
      os << "  " << c1 << ": " << c2 << "\n";
    }

    for (const auto &funcName : functionOrder) {
      os << "Function: " << funcName << "\n";
      for (const auto &[c1, c2] : conversions[funcName]) {
        os << "  " << c1 << ": " << c2 << "\n";
      }
      os << "\n";
    }
  }

private:
  clang::ASTContext *Context;
  std::string currentFunction = "global_scope";
  std::vector<std::string> functionOrder;
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