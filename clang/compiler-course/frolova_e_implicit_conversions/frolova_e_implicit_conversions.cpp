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
  void handleTypeConversion(const clang::QualType &fromType,
                            const clang::QualType &toType) {
    std::string fromTypeStr = fromType.getAsString();
    std::string toTypeStr = toType.getAsString();

    fromTypeStr = (fromTypeStr == "_Bool") ? "bool" : fromTypeStr;
    toTypeStr = (toTypeStr == "_Bool") ? "bool" : toTypeStr;

    if (fromTypeStr == toTypeStr)
      return;

    std::string conversion = fromTypeStr + " -> " + toTypeStr;
    auto &convList = conversions[currentFunction];

    for (auto &entry : convList) {
      if (entry.first == conversion) {
        entry.second++;
        break;
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
      handleTypeConversion(fromType, toType);
    }
    return true;
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) {
    if (!cast || !cast->getSubExpr() ||
        cast->getCastKind() == clang::CK_FunctionToPointerDecay) {
      return true;
    }

    clang::QualType fromType = cast->getSubExpr()->getType();
    clang::QualType toType = cast->getType();
    handleTypeConversion(fromType, toType);
    return true;
  }

  void printResults() {
    auto &os = llvm::outs();
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
  std::string currentFunction;
  std::vector<std::string> functionOrder;
  std::map<std::string, std::vector<std::pair<std::string, int>>> conversions;
};

class ConversionConsumer final : public clang::ASTConsumer {
public:
  explicit ConversionConsumer(clang::ASTContext *context) : Visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
    Visitor.printResults();
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
    X("ImplicitConvPlugin",
      "Output the number of implicit conversions in the entire file");
