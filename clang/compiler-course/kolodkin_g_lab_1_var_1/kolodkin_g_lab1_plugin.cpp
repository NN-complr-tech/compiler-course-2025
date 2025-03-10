#include <string>
#include <sstream>
#include <vector>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace kolodkin_g_lab1 {
std::string getAccessSpelling(clang::AccessSpecifier access) {
    if (access == clang::AS_public) {
        return "public";
    } else if (access == clang::AS_protected) {
        return "protected";
    } else if (access == clang::AS_private) {
        return "private";
    } else {
        return "";
    }
}

class LabPluginVisitor final : public clang::RecursiveASTVisitor<LabPluginVisitor> {
public:
    explicit LabPluginVisitor(clang::ASTContext *context) : m_context(context) {}
    bool VisitCXXRecordDecl(clang::CXXRecordDecl *record) {
    if (record->isCompleteDefinition()) {
        llvm::outs() << record->getName();
        if (record->isPolymorphic() && !record->bases().empty()) {
            llvm::outs() << " -> " << record->bases().begin()->getType()->getAsCXXRecordDecl()->getName();
        }
        
        llvm::outs() << "\n";

        llvm::outs() << "|_Fields\n";
        if (record->field_empty()) {
            llvm::outs() << "| |_ (no fields)\n";
        } else {
            for (const auto *field : record->fields()) {
                llvm::outs() << "| |_ " << field->getName() << " ("
                              << field->getType().getAsString()
                              << "|" << kolodkin_g_lab1::getAccessSpelling(field->getAccess()) << ")\n";
            }
        }

        llvm::outs() << "|_Methods\n";
        bool hasMethods = false;
        for (const auto *method : record->methods()) {
            if (method->isImplicit()) continue;

            hasMethods = true;
            llvm::SmallVector<std::string, 4> specs;

            if (method->isVirtual() && !method->hasAttr<clang::OverrideAttr>()) specs.push_back("virtual");
            if (method->isPureVirtual()) specs.push_back("pure");
            if (method->hasAttr<clang::OverrideAttr>()) specs.push_back("override");

            std::ostringstream oss;
            oss << method->getReturnType().getAsString() << "(";
            for (unsigned i = 0; i < method->getNumParams(); ++i) {
                if (i > 0) oss << ", ";
                oss << method->getParamDecl(i)->getType().getAsString();
            }
            oss << ")";

            llvm::outs() << "| |_ " << method->getName() << " (" << oss.str() << "|"
                          << kolodkin_g_lab1::getAccessSpelling(method->getAccess());

            for (const auto &s : specs) {
                llvm::outs() << "|" << s;
            }
            llvm::outs() << ")\n";
        }

        if (!hasMethods) {
            llvm::outs() << "| |_ (no methods)\n";
        }

        llvm::outs() << "\n";
    }
    return true;
    }

private:
    clang::ASTContext *m_context;
};

class LabPluginConsumer final : public clang::ASTConsumer {
public:
    explicit LabPluginConsumer(clang::ASTContext *context) : m_visitor(context) {}

    void HandleTranslationUnit(clang::ASTContext &context) override {
        m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    LabPluginVisitor m_visitor;
};

class LabPluginAction final : public clang::PluginASTAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
        return std::make_unique<LabPluginConsumer>(&ci.getASTContext());
    }

    bool ParseArgs(const clang::CompilerInstance &ci, const std::vector<std::string> &args) override {
        return true;
    }
};
} //namespace kolodkin_g_lab1

static clang::FrontendPluginRegistry::Add<kolodkin_g_lab1::LabPluginAction> X("LabPlugin_KolodkinGrigorii_FIIT3_ClangAST", "Prints info about user defined data types");
