#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
namespace {
	std::string getQualifier(clang::AccessSpecifier temp) {
		if (temp == clang::AS_public) return "public";
		else if (temp == clang::AS_private) return "private";
		else if (temp == clang::AS_protected) return "protected";
		return "";
	}


	class DataTypesVisitor final : public clang::RecursiveASTVisitor<DataTypesVisitor> {
	public:
		explicit DataTypesVisitor(clang::ASTContext* context) : m_context_(context) {}
		bool VisitCXXRecordDecl(clang::CXXRecordDecl* recordDecl) {
			auto&& output = llvm::outs();

			output << recordDecl->getNameAsString();
			if (recordDecl->getDescribedClassTemplate()) {
				output << "(" << (recordDecl->isStruct() ? "struct" : "class") << "|template)";
			}
			else {
				output << "(" << (recordDecl->isStruct() ? "struct" : "class") << ")";
			}

			if (recordDecl->getNumBases()) {
				output << " -> ";
				llvm::interleave(
					recordDecl->bases(), output,
					[&](const clang::CXXBaseSpecifier& base) {
						output << getQualifier(base.getAccessSpecifier()) << " "
							<< base.getType().getAsString();
					},
					","
				);
			}
			output << "\n";

			output << "|_Fields\n";
			bool hasFields = false;
			for (const auto& field : recordDecl->fields()) {
				hasFields = true;
				output << "| |_ " << field->getName() << " (" << field->getType().getAsString();
				output << '|' << getQualifier(field->getAccess()) << ")\n";
			}
			if (!hasFields) output << "| |_ (has no fields)\n";

			output << "|_Methods\n";
			bool hasMethods = false;
			for (auto&& method : recordDecl->methods()) {
				if (method->isImplicit()) continue;
				hasMethods = true;

				output << "| |_ " << method->getNameAsString() << " (" << method->getReturnType().getAsString();

				llvm::interleave(method->parameters(), output,
					[](const clang::ParmVarDecl* param) { llvm::outs() << param->getType().getAsString(); },
					",");

				output << "|" << getQualifier(method->getAccess());

				if (method->isStatic()) output << "|static";
				if (method->hasAttr<clang::OverrideAttr>()) output << "|override";
				else if (method->isVirtual()) {
					output << (method->isPureVirtual() ? "|virtual|pure" : "|virtual");
				}

				output << ")\n";
			}
			if (!hasMethods) output << "| |_ (has no methods)\n";

			return true;
		}
	private:
		clang::ASTContext* m_context_;
	};


	class DataTypesConsumer final : public clang::ASTConsumer {
	public:
		explicit DataTypesConsumer(clang::ASTContext* context) : visitor_(context) {}

		void HandleTranslationUnit(clang::ASTContext& context) override {
			visitor_.TraverseDecl(context.getTranslationUnitDecl());
		}

	private:
		DataTypesVisitor visitor_;
	};
	class DataTypesAction final : public clang::PluginASTAction {
	public:
		std::unique_ptr<clang::ASTConsumer>
			CreateASTConsumer(clang::CompilerInstance& ci, llvm::StringRef) override {
			return std::make_unique<DataTypesConsumer>(&ci.getASTContext());
		}
		bool ParseArgs(const clang::CompilerInstance& ci, const std::vector<std::string>& args) override {
			return true;
		}
	};
} // namespace

static clang::FrontendPluginRegistry::Add<DataTypesAction>
X("BaranovDataPlugin", "Traverses data types, print info and qualifiers");
