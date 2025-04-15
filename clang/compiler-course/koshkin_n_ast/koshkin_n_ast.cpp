#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Attr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {
	class MaybeUnusedVisitor final : public clang::RecursiveASTVisitor<MaybeUnusedVisitor> {
	public:
		explicit MaybeUnusedVisitor(clang::ASTContext* context, Rewriter& Rewrite) : m_context(context), TheRewriter(Rewrite) {}

		bool VisitFunctionDecl(FunctionDecl* FD) {
			if (!FD->hasBody() || FD->isImplicit())
				return true;

			for (unsigned i = 0, N = FD->getNumParams(); i < N; ++i) {
				ParmVarDecl* PVD = FD->getParamDecl(i);
				if (PVD && PVD->getName() == "unused" &&
					!PVD->hasAttr<UnusedAttr>()) {
					SourceLocation Loc = PVD->getBeginLoc();
					if (Loc.isValid())
						TheRewriter.InsertText(Loc, "[[maybe_unused]] ", /*InsertBefore=*/true, /*IndentNewLines=*/true);
				}
			}

			Stmt* FuncBody = FD->getBody();
			if (FuncBody)
				processLocalVars(FuncBody);

			return true;
		}

	private:
		void processLocalVars(Stmt* S) {
			if (!S) 
				return;

			if (auto* DS = dyn_cast<DeclStmt>(S)) {
				for (auto* D : DS->decls()) {
					if (auto* VD = dyn_cast<VarDecl>(D)) {
						if (VD->isLocalVarDecl() && !VD->isImplicit() &&
							VD->getName() == "unused" &&
							!VD->hasAttr<UnusedAttr>()) {
							SourceLocation Loc = VD->getBeginLoc();
							if (Loc.isValid())
								TheRewriter.InsertText(Loc, "[[maybe_unused]] ", true, true);
						}
					}
				}
			}
			for (Stmt* Child : S->children()) {
				processLocalVars(Child);
			}
		}

		clang::ASTContext *m_context;
		clang::Rewriter& TheRewriter;
	};

	class MaybeUnusedConsumer final : public clang::ASTConsumer {
	public:
		explicit MaybeUnusedConsumer(clang::ASTContext* context, Rewriter& Rewrite) : Visitor(context, Rewrite) {}

		void HandleTranslationUnit(clang::ASTContext& context) override {
			Visitor.TraverseDecl(context.getTranslationUnitDecl());
		}

	private:
		MaybeUnusedVisitor Visitor;
	};

	class MaybeUnusedAction final : public clang::PluginASTAction {
	public:
		std::unique_ptr<clang::ASTConsumer>
			CreateASTConsumer(clang::CompilerInstance& ci, llvm::StringRef) override {
			TheRewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
			return std::make_unique<MaybeUnusedConsumer>(&ci.getASTContext(), TheRewriter);
		}

		bool ParseArgs(const clang::CompilerInstance& ci,
			const std::vector<std::string>& args) override {
			return true;
		}

		void EndSourceFileAction() override {
			auto& SM = TheRewriter.getSourceMgr();
			TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
		}
	private:
		Rewriter TheRewriter;
	};
} // namespace

static clang::FrontendPluginRegistry::Add<MaybeUnusedAction>
X("koshkin_n_MaybeUnused_plugin", "Marks unused parameters/variables with [[maybe_unused]] attribute");