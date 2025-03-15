#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <vector>

namespace {

class MyClangVisitor final : public clang::RecursiveASTVisitor<MyClangVisitor> {
public:
    explicit MyClangVisitor(clang::ASTContext *context) : Context(context) {}

    bool VisitFunctionDecl(clang::FunctionDecl *func) {
        currentFunction = func->getNameInfo().getName().getAsString();
        if (conversions.find(currentFunction) == conversions.end()) {
            functionOrder.push_back(currentFunction);
        }
        return true;
    }

    bool VisitImplicitCastExpr(clang::ImplicitCastExpr *cast) {
        if (!cast || !cast->getSubExpr()) {
            return true;
        }

        clang::QualType fromType = cast->getSubExpr()->getType();
        clang::QualType toType = cast->getType();

        if (fromType == toType) {
            return true;
        }

        std::string fromTypeStr = fromType.getAsString();
        std::string toTypeStr = toType.getAsString();

        // Заменяем "_Bool" на "bool" для читаемости
        fromTypeStr = (fromTypeStr == "_Bool") ? "bool" : fromTypeStr;
        toTypeStr = (toTypeStr == "_Bool") ? "bool" : toTypeStr;

        std::string conversion = fromTypeStr + " -> " + toTypeStr;

        // Увеличиваем счетчик для данного преобразования в текущей функции
        auto &convList = conversions[currentFunction];
        bool found = false;
        for (auto &entry : convList) {
            if (entry.first == conversion) {
                entry.second++;
                found = true;
                break;
            }
        }

        if (!found) {
            convList.push_back({conversion, 1});
        }

        return true;
    }

    void PrintResults() {
        auto &os = llvm::outs();
        for (const auto &funcName : functionOrder) {
            os << "Function: " << funcName << "\n";
            for (const auto &conversion : conversions[funcName]) {
                os << "  " << conversion.first << ": " << conversion.second << "\n";
            }
            os << "\n";
        }
    }

private:
    clang::ASTContext *Context;
    std::string currentFunction;
    std::vector<std::string> functionOrder;
    std::map<std::string, std::vector<std::pair<std::string, int>>> conversions;
};

class MyClangConsumer final : public clang::ASTConsumer {
public:
    explicit MyClangConsumer(clang::ASTContext *context) : Visitor(context) {}

    void HandleTranslationUnit(clang::ASTContext &context) override {
        Visitor.TraverseDecl(context.getTranslationUnitDecl());
        Visitor.PrintResults();
    }

private:
    MyClangVisitor Visitor;
};

class MyClangPlugin final : public clang::PluginASTAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
        return std::make_unique<MyClangConsumer>(&ci.getASTContext());
    }

    bool ParseArgs(const clang::CompilerInstance &ci, const std::vector<std::string> &args) override {
        return true;
    }
};

} // namespace

static clang::FrontendPluginRegistry::Add<MyClangPlugin>
X("myClangPlugin", "Counts implicit type conversions");
