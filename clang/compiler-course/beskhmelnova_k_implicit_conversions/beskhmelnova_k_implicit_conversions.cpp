#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <vector>
#include <algorithm>

namespace {

class ImplicitCastVisitor final : public clang::RecursiveASTVisitor<ImplicitCastVisitor> {
public:
 explicit ImplicitCastVisitor(clang::ASTContext *Context)
     : Context(Context) {}

 bool VisitFunctionDecl(clang::FunctionDecl *Func) {
     CurrentFunction = Func->getNameInfo().getName().getAsString();
     if (FunctionOrder.find(CurrentFunction) == FunctionOrder.end()) {
         FunctionOrder[CurrentFunction] = FunctionOrder.size();
     }

     return true;
 }

 bool VisitImplicitCastExpr(clang::ImplicitCastExpr *Cast) {
     std::string FromType = Cast->getSubExpr()->getType().getAsString();
     std::string ToType = Cast->getType().getAsString();

     if (FromType == ToType || FromType.find("(*)") != std::string::npos || ToType.find("(*)") != std::string::npos) {
         return true;
     }

     unsigned LineNumber = Context->getSourceManager().getSpellingLineNumber(Cast->getExprLoc());
     unsigned FunctionIndex = FunctionOrder[CurrentFunction];
     CastList.push_back({FunctionIndex, LineNumber, CastCounter++, CurrentFunction, FromType + " -> " + ToType});
     return true;
 }

 void PrintResults() {
     std::sort(CastList.begin(), CastList.end());

     std::string LastFunction;
     for (const auto &Entry : CastList) {
         if (Entry.FunctionName != LastFunction) {
             llvm::outs() << "Function " << Entry.FunctionName << "\n";
             LastFunction = Entry.FunctionName;
         }
         llvm::outs() << Entry.CastDescription << ": 1\n";
     }
 }

private:
 struct CastEntry {
     unsigned FunctionIndex;
     unsigned Line;
     unsigned Order;
     std::string FunctionName;
     std::string CastDescription;

     bool operator<(const CastEntry &Other) const {
         return std::tie(FunctionIndex, Line, Order) < std::tie(Other.FunctionIndex, Other.Line, Other.Order);
     }
 };

 clang::ASTContext *Context;
 std::string CurrentFunction;
 std::map<std::string, unsigned> FunctionOrder;
 std::vector<CastEntry> CastList;
 unsigned CastCounter = 0;
};

class ImplicitCastConsumer final : public clang::ASTConsumer {
public:
 explicit ImplicitCastConsumer(clang::ASTContext *Context)
     : Visitor(Context) {}

 void HandleTranslationUnit(clang::ASTContext &Context) override {
     Visitor.TraverseDecl(Context.getTranslationUnitDecl());
     Visitor.PrintResults();
 }

private:
 ImplicitCastVisitor Visitor;
};

class ImplicitCastAction final : public clang::PluginASTAction {
public:
 std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
     return std::make_unique<ImplicitCastConsumer>(&CI.getASTContext());
 }

 bool ParseArgs(const clang::CompilerInstance &CI, const std::vector<std::string> &Args) override {
     return true;
 }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitCastAction>
 X("ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST", "Counts implicit type conversions"); 