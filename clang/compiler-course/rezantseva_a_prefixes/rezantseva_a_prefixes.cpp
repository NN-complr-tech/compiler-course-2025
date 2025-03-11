#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

namespace {
    class PrefixVisitor : public clang::RecursiveASTVisitor<PrefixVisitor> {
    public:
        explicit PrefixVisitor(clang::ASTContext* context, clang::Rewriter& rewriter)
            : m_rewriter(rewriter) {}

        bool VisitVarDecl(clang::VarDecl* var) {
            if (var->isLocalVarDecl()) {
                renameVar(var, "local_");
            }
            else if (var->isStaticLocal()) {
                renameVar(var, "static_");
            }
            else if (var->hasGlobalStorage()) {
                renameVar(var, "global_");
            }
            return true;
        }

        bool VisitParmVarDecl(clang::ParmVarDecl* param) {
            renameVar(param, "param_");
            return true;
        }

        bool VisitCallExpr(clang::CallExpr* call) {
            for (auto* arg : call->arguments()) {
                if (auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(arg)) {
                    if (auto* var = clang::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
                        renameVar(var, getPrefixForVar(var));
                    }
                }
            }
            return true;
        }

    private:
        clang::Rewriter& m_rewriter;

        void renameVar(clang::VarDecl* var, const std::string& prefix) {
            std::string newName = prefix + var->getName().str();
            m_rewriter.ReplaceText(var->getLocation(), var->getName().size(), newName);
        }

        std::string getPrefixForVar(clang::VarDecl* var) {
            if (var->isLocalVarDecl()) {
                return "local_";
            }
            else if (var->isStaticLocal()) {
                return "static_";
            }
            else if (var->hasGlobalStorage()) {
                return "global_";
            }
            return "";
        }
    };

    class PrefixConsumer : public clang::ASTConsumer {
    public:
        explicit PrefixConsumer(clang::ASTContext* context, clang::Rewriter& rewriter)
            : m_visitor(context, rewriter), m_rewriter(rewriter) {}

        void HandleTranslationUnit(clang::ASTContext& context) override {
            m_visitor.TraverseDecl(context.getTranslationUnitDecl());
            m_rewriter.getEditBuffer(context.getSourceManager().getMainFileID()).write(llvm::outs());
        }

    private:
        PrefixVisitor m_visitor;
        clang::Rewriter& m_rewriter;
    };

    class PrefixAction : public clang::PluginASTAction {
    public:
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance& ci, llvm::StringRef) override {
            m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
            return std::make_unique<PrefixConsumer>(&ci.getASTContext(), m_rewriter);
        }

        bool ParseArgs(const clang::CompilerInstance& ci,
                       const std::vector<std::string>& args) override {
            return true;
        }

    private:
        clang::Rewriter m_rewriter;
    };
}// namespace

static clang::FrontendPluginRegistry::Add<PrefixAction>
X("PrefixesPlugin_RezantsevaAnastasia_FIIT1_ClangAST", "Adds appropriate prefixes to objects and parameters");
