#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <vector>

namespace {

class PrefixRenamerVisitor
    : public clang::RecursiveASTVisitor<PrefixRenamerVisitor> {
public:
  PrefixRenamerVisitor(clang::ASTContext &Ctx, clang::Rewriter &R,
                       const std::string &S, const std::string &L,
                       const std::string &G, const std::string &P)
      : Context(Ctx), Rewrite(R), staticPref(S), localPref(L), globalPref(G),
        paramPref(P) {}

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getNameAsString().empty() || VD->getLocation().isMacroID())
      return true;

    std::string orig = VD->getNameAsString();
    std::string prefix;
    if (VD->isStaticLocal())
      prefix = staticPref;
    else if (VD->isLocalVarDecl())
      prefix = localPref;
    else if (VD->hasGlobalStorage())
      prefix = globalPref;

    if (prefix.empty())
      return true;

    std::string renamedName = prefix + orig;
    Rewrite.ReplaceText(VD->getLocation(), orig.size(), renamedName);
    renamedMap.insert({VD, renamedName});
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *PD) {
    if (PD->getNameAsString().empty() || PD->getLocation().isMacroID())
      return true;

    std::string orig = PD->getNameAsString();
    std::string renamedName = paramPref + orig;
    Rewrite.ReplaceText(PD->getLocation(), orig.size(), renamedName);
    renamedMap.insert({PD, renamedName});
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *DR) {
    clang::ValueDecl *D = DR->getDecl();
    auto it = renamedMap.find(D);
    if (it == renamedMap.end())
      return true;

    std::string orig = D->getNameAsString();
    Rewrite.ReplaceText(DR->getLocation(), orig.size(), it->second);
    return true;
  }

private:
  clang::ASTContext &Context;
  clang::Rewriter &Rewrite;
  std::string staticPref, localPref, globalPref, paramPref;
  llvm::DenseMap<clang::ValueDecl *, std::string> renamedMap;
};

class PrefixRenamerConsumer : public clang::ASTConsumer {
public:
  PrefixRenamerConsumer(clang::ASTContext &Ctx, clang::Rewriter &R,
                        const std::string &S, const std::string &L,
                        const std::string &G, const std::string &P)
      : Visitor(Ctx, R, S, L, G, P), Rewrite(R) {}

  void HandleTranslationUnit(clang::ASTContext &Ctx) override {
    Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
    const auto &SM = Ctx.getSourceManager();
    Rewrite.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

private:
  PrefixRenamerVisitor Visitor;
  clang::Rewriter &Rewrite;
};

class PrefixRenamerAction : public clang::PluginASTAction {
public:
  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    staticPref = "static_";
    localPref = "local_";
    globalPref = "global_";
    paramPref = "param_";

    for (const auto &arg : args) {
      if (arg.rfind("--static=", 0) == 0)
        staticPref = arg.substr(9);
      else if (arg.rfind("--local=", 0) == 0)
        localPref = arg.substr(8);
      else if (arg.rfind("--global=", 0) == 0)
        globalPref = arg.substr(9);
      else if (arg.rfind("--param=", 0) == 0)
        paramPref = arg.substr(8);
    }
    return true;
  }

  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef) override {
    RewriterForFile.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<PrefixRenamerConsumer>(
        CI.getASTContext(), RewriterForFile, staticPref, localPref, globalPref,
        paramPref);
  }

private:
  clang::Rewriter RewriterForFile;
  std::string staticPref, localPref, globalPref, paramPref;
};
} // namespace

static clang::FrontendPluginRegistry::Add<PrefixRenamerAction>
    X("prefix-renamer", "Adds prefixes static_, local_, global_, param_");
