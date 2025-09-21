#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;

namespace {
struct Stats : RecursiveASTVisitor<Stats> {
  unsigned Ifs = 0, Fors = 0, Whiles = 0, Returns = 0, BinOps = 0;

  bool VisitIfStmt(IfStmt*) { ++Ifs; return true; }
  bool VisitForStmt(ForStmt*) { ++Fors; return true; }
  bool VisitWhileStmt(WhileStmt*) { ++Whiles; return true; }
  bool VisitReturnStmt(ReturnStmt*) { ++Returns; return true; }
  bool VisitBinaryOperator(BinaryOperator* BO) { (void)BO; ++BinOps; return true; }
};

class StatsConsumer : public ASTConsumer {
  Stats S;
public:
  void HandleTranslationUnit(ASTContext &Ctx) override {
    S.TraverseDecl(Ctx.getTranslationUnitDecl());
    llvm::outs() << "CC-CLANGAST-REPORT\n"
                 << "ifs: " << S.Ifs << "\n"
                 << "fors: " << S.Fors << "\n"
                 << "whiles: " << S.Whiles << "\n"
                 << "returns: " << S.Returns << "\n"
                 << "binops: " << S.BinOps << "\n";
  }
};

class StatsAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance&, llvm::StringRef) override {
    return std::make_unique<StatsConsumer>();
  }
};
} // namespace

static llvm::cl::OptionCategory Cat("compiler-course-clangast options");

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, Cat);
  if (!ExpectedParser) {
    llvm::errs() << "Failed to parse options\n";
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<StatsAction>().get());
}
