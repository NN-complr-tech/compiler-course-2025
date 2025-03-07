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
		explicit ImplicitCastVisitor(clang::ASTContext* Context)
			: Context(Context) {}

		bool VisitFunctionDecl(clang::FunctionDecl* Func) {
			CurrentFunction = Func->getNameInfo().getName().getAsString();
			if (FunctionOrder.find(CurrentFunction) == FunctionOrder.end()) {
				FunctionOrder[CurrentFunction] = FunctionOrder.size();
			}

			return true;
		}

		bool VisitCXXConstructExpr(clang::CXXConstructExpr* Ctor) {
			if (Ctor->getNumArgs() < 1) {
				return true;
			}

			clang::QualType FromType = Ctor->getArg(0)->getType();
			clang::QualType ToType = Ctor->getType();

			if (FromType == ToType) {
				return true;
			}

			unsigned LineNumber = Context->getSourceManager().getSpellingLineNumber(Ctor->getExprLoc());
			unsigned FunctionIndex = FunctionOrder[CurrentFunction];
			CastEntry Entry{ FunctionIndex, LineNumber, CastCounter++, CurrentFunction, FromType.getAsString(), ToType.getAsString() };
			CastList.push_back(Entry);
			return true;
		}

		clang::QualType getRealType(clang::QualType Type) {
			Type = Type.getCanonicalType();
			if (auto* TST = Type->getAs<clang::TypedefType>()) {
				return TST->getDecl()->getUnderlyingType().getCanonicalType();
			}
			return Type;
		}

		bool VisitImplicitCastExpr(clang::ImplicitCastExpr* Cast) {
			clang::CastKind Kind = Cast->getCastKind();
			if (Kind == clang::CK_NoOp || Kind == clang::CK_LValueToRValue || Kind == clang::CK_FunctionToPointerDecay) {
				return true;
			}

			clang::QualType FromType = getRealType(Cast->getSubExpr()->getType());
			clang::QualType ToType = getRealType(Cast->getType());

			if (FromType == ToType) {
				return true;
			}
			unsigned LineNumber = Context->getSourceManager().getSpellingLineNumber(Cast->getExprLoc());
			unsigned FunctionIndex = FunctionOrder[CurrentFunction];
			CastEntry Entry{ FunctionIndex, LineNumber, CastCounter++, CurrentFunction, FromType.getAsString(), ToType.getAsString() };
			CastList.push_back(Entry);
			return true;
		}

		void PrintResults() {
			std::string LastFunction;
			for (const auto& Entry : CastList) {
				if (Entry.FunctionName != LastFunction) {
					llvm::outs() << "Function " << Entry.FunctionName << "\n";
					LastFunction = Entry.FunctionName;
				}
				if (Entry.FromType != Entry.ToType) {
					llvm::outs() << Entry.getCastDescription() << ": 1\n";
				}
			}
			llvm::outs() << "Total implicit conversions: " << CastList.size() << "\n";
		}

	private:
		struct CastEntry {
			unsigned FunctionIndex;
			unsigned Line;
			unsigned Order;
			std::string FunctionName;
			std::string FromType;
			std::string ToType;

			std::string getCastDescription() const {
				return FromType + " -> " + ToType;
			}
		};

		clang::ASTContext* Context;
		std::string CurrentFunction;
		std::map<std::string, unsigned> FunctionOrder;
		std::vector<CastEntry> CastList;
		unsigned CastCounter = 0;
	};

	class ImplicitCastConsumer final : public clang::ASTConsumer {
	public:
		explicit ImplicitCastConsumer(clang::ASTContext* Context)
			: Visitor(Context) {}

		void HandleTranslationUnit(clang::ASTContext& Context) override {
			Visitor.TraverseDecl(Context.getTranslationUnitDecl());
			Visitor.PrintResults();
		}

	private:
		ImplicitCastVisitor Visitor;
	};

	class ImplicitCastAction final : public clang::PluginASTAction {
	public:
		std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef) override {
			return std::make_unique<ImplicitCastConsumer>(&CI.getASTContext());
		}

		bool ParseArgs(const clang::CompilerInstance& CI, const std::vector<std::string>& Args) override {
			return true;
		}
	};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitCastAction>
X("ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST", "Counts implicit type conversions");