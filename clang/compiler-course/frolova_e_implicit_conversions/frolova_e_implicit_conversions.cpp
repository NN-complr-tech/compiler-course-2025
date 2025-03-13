#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace {
class ImplicitConvVisitor final : public clang::RecursiveASTVisitor<ImplicitConvVisitor> {
public:
  explicit ImplicitConvVisitor(clang::ASTContext *context) : Context(context) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {

    currentFunction = func->getNameInfo().getName().getAsString();
    return true;
  } 

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) {
    if (!cast || !cast->getSubExpr()) {
        return true;
    }

    clang::QualType fromType = cast->getSubExpr()->getType();
    clang::QualType toType = cast->getType();

    std::string fromTypeStr = fromType.getAsString();
    std::string toTypeStr = toType.getAsString();

    if (fromTypeStr != toTypeStr) {
        std::string conversion = fromTypeStr + " -> " + toTypeStr;
        auto &convList = conversions[currentFunction];

        bool found = false;
        for (auto &entry : convList) {
            if (entry.first == conversion) {
                entry.second++; 
                found = true;
                break;
            }
        }

        if (!found) {
            convList.push_back({conversion, 1});
        }
    }

    return true;
}

  bool VisitCallExpr(clang::CallExpr *call) {
    if (auto *callee = call->getDirectCallee()) {
        llvm::outs() << "Function call detected: " << callee->getNameAsString() << "\n";
    }
    return true;
  }

  void printResults() {
    for (const auto &func : conversions) {
        llvm::outs() << "Function: " << func.first << "\n";
        
        for (const auto &conv : func.second) {
            llvm::outs() << "  " << conv.first << ": " << conv.second << "\n";
        }

        llvm::outs() << "\n";
    }
}

private:
  clang::ASTContext *Context;
  std::string currentFunction;
  std::map<std::string, std::vector<std::pair<std::string, int>>> conversions;

};

class ExampleConsumer final : public clang::ASTConsumer {
public:
  explicit ExampleConsumer(clang::ASTContext *context) : Visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
    Visitor.printResults();
  }

private:
ImplicitConvVisitor Visitor;
};

class ExampleAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ExampleConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<ExampleAction>
    X("ImplicitConvPlugin", "Output the number of implicit conversions in the entire file");
