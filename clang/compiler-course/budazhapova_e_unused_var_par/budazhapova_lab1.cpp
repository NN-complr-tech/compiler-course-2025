#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>

using namespace clang;

namespace {
class UnusedVarVisitor : public RecursiveASTVisitor<UnusedVarVisitor> {
public:
  explicit UnusedVarVisitor(Rewriter &rw) : rewriter(rw) {}

  bool VisitVarDecl(VarDecl *vd) {
    if (isa<ParmVarDecl>(vd)) {
      return true;
    }
    if (!vd->isUsed()) {
      rewriter.InsertText(vd->getBeginLoc(), "[[maybe_unused]] ", true, true);
    }
    return true;
  }

  bool VisitParmVarDecl(ParmVarDecl *pvd) {
    if (!pvd->isUsed()) {
      rewriter.InsertText(pvd->getBeginLoc(), "[[maybe_unused]] ", true, true);
    }
    return true;
  }

private:
  Rewriter &rewriter;
};

class UnusedVPConsumer : public ASTConsumer {
public:
  explicit UnusedVPConsumer(Rewriter &rw) : visitor(rw) {}

  void HandleTranslationUnit(ASTContext &ctx) override {
    visitor.TraverseDecl(ctx.getTranslationUnitDecl());
  }

private:
  UnusedVarVisitor visitor;
};

class UnusedVarPlugin : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci,
                                                 llvm::StringRef) override {
    rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<UnusedVPConsumer>(rewriter);
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    auto &buffer =
        rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID());
    std::string code;
    llvm::raw_string_ostream stream(code);
    buffer.write(stream);

    std::stringstream ss(code);
    std::string line;
    std::string result;
    while (std::getline(ss, line)) {
      if (line.find("// RUN:") == 0 || line.find("// CHECK:") == 0) {
        continue;
      }
      result += line + "\n";
    }
    llvm::outs() << result;
  }

private:
  Rewriter rewriter;
};
} // namespace

static FrontendPluginRegistry::Add<UnusedVarPlugin>
    X("unused_var_plugin",
      "Add [[maybe_unused]] to unused variables and parameters");
