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
		explicit ImplicitCastVisitor() = default;

		bool VisitFunctionDecl(clang::FunctionDecl* Func) {
			CurrentFunction = Func->getNameInfo().getName().getAsString();
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

			CastList.emplace_back(CastEntry{ CurrentFunction, FromType.getAsString(), ToType.getAsString() });
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

			CastList.emplace_back(CastEntry{ CurrentFunction, FromType.getAsString(), ToType.getAsString() });
			return true;
		}

		void PrintResults() {
			std::string LastFunction;
			for (const auto& Entry : CastList) {
				if (Entry.FunctionName != LastFunction) {
					llvm::outs() << "Function " << Entry.FunctionName << "\n";
					LastFunction = Entry.FunctionName;
				}
				llvm::outs() << Entry.getCastDescription() << ": 1\n";
			}
			llvm::outs() << "Total implicit conversions: " << CastList.size() << "\n";
		}

	private:
		struct CastEntry {
			std::string FunctionName;
			std::string FromType;
			std::string ToType;

			std::string getCastDescription() const {
				return FromType + " -> " + ToType;
			}
		};

		std::string CurrentFunction;
		std::vector<CastEntry> CastList;
	};

	class ImplicitCastConsumer final : public clang::ASTConsumer {
	public:
		explicit ImplicitCastConsumer() = default;

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
			return std::make_unique<ImplicitCastConsumer>();
		}

		bool ParseArgs(const clang::CompilerInstance& CI, const std::vector<std::string>& Args) override {
			return true;
		}
	};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitCastAction>
X("ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST", "Counts implicit type conversions");
