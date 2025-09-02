#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

namespace {

class ScopePrefixAdder final 
    : public clang::RecursiveASTVisitor<ScopePrefixAdder> {
public:
  explicit ScopePrefixAdder(clang::Rewriter &CodeRewriter)
      : RewriterTool(CodeRewriter) {}

  bool VisitVarDecl(clang::VarDecl *VariableDeclaration) {
    if (VariableDeclaration->getName().empty()) {
      return true;
    }

    std::string ScopePrefix;
    
    if (VariableDeclaration->isStaticLocal()) {
      ScopePrefix = "static_";
    } else if (VariableDeclaration->isLocalVarDecl()) {
      ScopePrefix = "local_";
    } else if (VariableDeclaration->hasGlobalStorage()) {
      ScopePrefix = "global_";
    }

    if (!ScopePrefix.empty()) {
      std::string OriginalName = VariableDeclaration->getName().str();
      if (OriginalName.find(ScopePrefix) != 0) {
        std::string NewVariableName = ScopePrefix + OriginalName;
        RenamedVariablesMap[OriginalName] = NewVariableName;
        clang::SourceLocation NamePosition = VariableDeclaration->getLocation();
        RewriterTool.ReplaceText(NamePosition, OriginalName.length(),
                                 NewVariableName);
      }
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *FunctionParameter) {
    if (FunctionParameter->getName().empty()) {
      return true;
    }

    std::string OriginalName = FunctionParameter->getName().str();
    std::string NewParameterName = "param_" + OriginalName;
    RenamedVariablesMap[OriginalName] = NewParameterName;
    RewriterTool.ReplaceText(FunctionParameter->getLocation(),
                             OriginalName.size(), NewParameterName);
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *VariableReference) {
    clang::ValueDecl *ReferencedDeclaration = VariableReference->getDecl();
    if (!ReferencedDeclaration || ReferencedDeclaration->getName().empty()) {
      return true;
    }

    if (llvm::isa<clang::FunctionDecl>(ReferencedDeclaration)) {
      return true;
    }

    std::string OriginalName = ReferencedDeclaration->getName().str();
    auto MapIterator = RenamedVariablesMap.find(OriginalName);
    
    if (MapIterator != RenamedVariablesMap.end()) {
      clang::SourceLocation ReferencePosition =
          VariableReference->getLocation();
      RewriterTool.ReplaceText(ReferencePosition, OriginalName.length(),
                               MapIterator->second);
    } else {
      if (auto VarDecl =
              llvm::dyn_cast<clang::VarDecl>(ReferencedDeclaration)) {
        VarDecl = VarDecl->getCanonicalDecl();
        std::string NewName;

        if (VarDecl->hasGlobalStorage() && !VarDecl->isStaticLocal()) {
          NewName = "global_" + OriginalName;
        } else if (VarDecl->isStaticLocal()) {
          NewName = "static_" + OriginalName;
        } else if (llvm::isa<clang::ParmVarDecl>(VarDecl)) {
          NewName = "param_" + OriginalName;
        } else if (VarDecl->isLocalVarDecl()) {
          NewName = "local_" + OriginalName;
        }

        if (!NewName.empty()) {
          RenamedVariablesMap[OriginalName] = NewName;
          RewriterTool.ReplaceText(VariableReference->getLocation(),
                                   OriginalName.length(), NewName);
        }
      }
    }
    return true;
  }

private:
  clang::Rewriter &RewriterTool;
  std::map<std::string, std::string> RenamedVariablesMap;
};

class ASTTransformer final : public clang::ASTConsumer {
public:
  explicit ASTTransformer(clang::ASTContext *Context,
                          clang::Rewriter &CodeRewriter)
      : RewriterTool(CodeRewriter), PrefixVisitor(CodeRewriter) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    PrefixVisitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  clang::Rewriter &RewriterTool;
  ScopePrefixAdder PrefixVisitor;
};

class PrefixPlugin final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef) override {
    SourceRewriter.setSourceMgr(Compiler.getSourceManager(),
                                Compiler.getLangOpts());
    return std::make_unique<ASTTransformer>(&Compiler.getASTContext(), 
                                            SourceRewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &Compiler,
                 const std::vector<std::string> &Arguments) override {
    return true;
  }

  void EndSourceFileAction() override {
    SourceRewriter.getEditBuffer(SourceRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter SourceRewriter;
};

} // namespace

static clang::FrontendPluginRegistry::Add<PrefixPlugin>
    PluginRegistration("clangAstPrefix_1",
                        "Adds scope-based prefixes to variable names");