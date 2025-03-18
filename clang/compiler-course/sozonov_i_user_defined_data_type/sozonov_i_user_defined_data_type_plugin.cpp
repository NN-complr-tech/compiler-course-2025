#include "clang/AST/ASTConsumer.h" 
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

namespace {

	std::string AccessToString(clang::AccessSpecifier access) {
		switch (access) {
		case clang::AS_public: return "public";
		case clang::AS_protected: return "protected";
		case clang::AS_private: return "private";
		default: return "";
		}
	}

	class ClassInfoVisitor final : public clang::RecursiveASTVisitor<ClassInfoVisitor> {
	public:
		explicit ClassInfoVisitor(clang::ASTContext* context) {}
		bool VisitCXXRecordDecl(clang::CXXRecordDecl* record) {
			if (!record->isThisDeclarationADefinition() || record->isImplicit())
				return true;

			auto& output = llvm::outs();
			output << record->getQualifiedNameAsString();

			PrintBaseClassesInfo(output, record);
			output << "\n|_Fields\n";
			PrintFieldsInfo(output, record);
			output << "|\n|_Methods\n";
			PrintMethodsInfo(output, record);
			output << "\n";

			return true;
		}

	private:
		void PrintBaseClassesInfo(llvm::raw_ostream& output, const clang::CXXRecordDecl* record) {
			if (record->getNumBases() == 0) return;

			llvm::SmallVector<std::string, 4> bases;
			for (const auto& bases : record->bases())
				if (auto* base_decl = bases.getType()->getAsCXXRecordDecl())
					bases.push_back(base_decl->getQualifiedNameAsString());

			output << " -> " << llvm::join(bases, ", ");
		}

		void PrintFieldsInfo(llvm::raw_ostream& output, const clang::CXXRecordDecl* record) {
			if (record->field_empty()) {
				output << "| |_ (no fields)\n";
				return;
			}

			for (const auto* field : record->fields())
				output << "| |_ " << field->getNameAsString()
				<< " (" << field->getType().getAsString()
				<< "|" << AccessToString(field->getAccess()) << ")\n";
		}

		void PrintMethodsInfo(llvm::raw_ostream& output, const clang::CXXRecordDecl* record) {
			if (record->method_begin() == record->method_end()) {
				output << "| |_ (no methods)\n";
				return;
			}

			for (const auto* method : record->methods()) {
				if (method->isImplicit()) continue;

				output << "| |_ " << method->getNameAsString()
					<< " (" << method->getReturnType().getAsString() << "(";

				unsigned int n = method->getNumParams();
				for (unsigned int i = 0; i < n; ++i) {
					if (i > 0) output << ", ";
					output << method->getParamDecl(i)->getType().getAsString();
				}

				output << ")|" << AccessToString(method->getAccess());

				if (method->isVirtual() && !method->hasAttr<clang::OverrideAttr>())
					output << "|virtual";
				if (method->isPureVirtual())
					output << "|pure";
				if (method->hasAttr<clang::OverrideAttr>())
					output << "|override";
				if (method->isConst())
					output << "|const";

				output << ")\n";
			}
		}
	};

	class ClassInfoConsumer final : public clang::ASTConsumer {
	public:
		explicit ClassInfoConsumer(clang::ASTContext* context) : class_visitor(context) {}

		void HandleTranslationUnit(clang::ASTContext& context) override {
			class_visitor.TraverseDecl(context.getTranslationUnitDecl());
		}

	private:
		ClassInfoVisitor class_visitor;
	};

	class ClassInfoAction final : public clang::PluginASTAction {
	public:
		std::unique_ptr<clang::ASTConsumer>
			CreateASTConsumer(clang::CompilerInstance& ci, llvm::StringRef) override {
			return std::make_unique<ClassInfoConsumer>(&ci.getASTContext());
		}

		bool ParseArgs(const clang::CompilerInstance& ci,
			const std::vector<std::string>& args) override {
			return true;
		}
	};
} // namespace

static clang::FrontendPluginRegistry::Add<ClassInfoAction>
X("user_defined_data_type_plugin", "Prints information about the user defined data type: 1) Fields; 2) Methods; 3) Basic classes");