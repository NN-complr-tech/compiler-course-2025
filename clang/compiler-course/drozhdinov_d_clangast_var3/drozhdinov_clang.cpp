#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <vector>
#include <string>

using namespace std;
using namespace clang;
using namespace llvm;

namespace {
	class CastCounter final : public RecursiveASTVisitor<CastCounter> {
	public:
		explicit CastCounter() = default;
		bool VisitFunctionDecl(FunctionDecl* Expr) {
			CurrentFunction = Expr->getNameAsString();
			return true;
		}

		bool VisitCXXConstructExpr(CXXConstructExpr* Expr) {
			if (Expr->getNumArgs() < 1) {
				return true;
			}

			QualType SourceType = Expr->getArg(0)->getType();
			QualType DestType = Expr->getType();

			if (SourceType == DestType) {
				return true;
			}
			CastMap[CurrentFunction][make_pair(SourceType.getAsString(), DestType.getAsString())]++;
			return true;
		}

		bool VisitImplicitCastExpr(ImplicitCastExpr* Expr) {
			CastKind Kind = Expr->getCastKind();
			
			if (Kind == CK_LValueToRValue || Kind == CK_FunctionToPointerDecay){
				return true;
			}
			
			QualType SourceType = Expr->getSubExpr()->getType();
			QualType DestType = Expr->getType();

			if (SourceType == DestType) {
				return true;
			}
			CastMap[CurrentFunction][make_pair(SourceType.getAsString(), DestType.getAsString())]++;
			return true;
		}

		bool getResult() {
			for (auto iter = CastMap.rbegin(); iter != CastMap.rend(); iter++){
				outs() << "Function " << (*iter).first << "\n";
				for (auto [cast, val] : (*iter).second){
					outs() << cast.first << " -> " << cast.second << ": " << val << "\n";
				}
			}
			return true;
		}

	private:
		map<string, map<pair<string, string>, int>> CastMap;
		string CurrentFunction;
	};

	class CastCounterConsumer final : public ASTConsumer {
	private:
		CastCounter cc;
	public:
		explicit CastCounterConsumer() = default;

		void HandleTranslationUnit(ASTContext& Context) override {
			cc.TraverseDecl(Context.getTranslationUnitDecl());
			cc.getResult();
		}

	
	};

	class CastCounterAction final : public PluginASTAction {
	public:
		unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, StringRef) override {
			return make_unique<CastCounterConsumer>();
		}

		bool ParseArgs(const CompilerInstance& CI, const vector<string>& Args) override {
			return true;
		}
	};

} // namespace

static FrontendPluginRegistry::Add<CastCounterAction>
X("CastCounter", "Counts implicit casts");
