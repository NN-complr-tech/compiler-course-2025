#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <vector>

using namespace clang;

class MyClangVisitor : public RecursiveASTVisitor<MyClangVisitor> {
public:
    explicit MyClangVisitor(ASTContext *Context) : Context(Context) {}

    bool VisitFunctionDecl(FunctionDecl *Func) {
        currentFunction = Func->getNameInfo().getName().getAsString();
        if (conversions.find(currentFunction) == conversions.end()) {
            functionOrder.push_back(currentFunction);
        }
        return true;
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr *Expr) {
        if (!Expr || !Expr->getSubExpr()) {
            return true;
        }

        // Игнорируем преобразования, связанные с указателями на функции
        if (Expr->getCastKind() == clang::CK_FunctionToPointerDecay) {
            return true;
        }

        QualType SourceType = Expr->getSubExpr()->getType();
        QualType TargetType = Expr->getType();

        // Игнорируем тривиальные преобразования
        if (SourceType.getAsString() == TargetType.getAsString()) {
            return true;
        }

        std::string SourceTypeStr = SourceType.getAsString();
        std::string TargetTypeStr = TargetType.getAsString();

        // Заменяем "_Bool" на "bool" для читаемости
        SourceTypeStr = (SourceTypeStr == "_Bool") ? "bool" : SourceTypeStr;
        TargetTypeStr = (TargetTypeStr == "_Bool") ? "bool" : TargetTypeStr;

        std::string Conversion = SourceTypeStr + " -> " + TargetTypeStr;

        // Увеличиваем счетчик для данного преобразования в текущей функции
        auto &convList = conversions[currentFunction];
        bool found = false;
        for (auto &entry : convList) {
            if (entry.first == Conversion) {
                entry.second++;
                found = true;
                break;
            }
        }

        if (!found) {
            convList.push_back({Conversion, 1});
        }

        return true;
    }

    void PrintResults() {
        auto &os = llvm::outs();
        for (const auto &funcName : functionOrder) {
            os << "Function " << funcName << "\n";
            for (const auto &conv : conversions[funcName]) {
                os << "  " << conv.first << ": " << conv.second << "\n";
            }
            os << "\n";
        }
    }

private:
    ASTContext *Context;
    std::string currentFunction;
    std::vector<std::string> functionOrder;
    std::map<std::string, std::vector<std::pair<std::string, int>>> conversions;
};

class MyClangConsumer : public ASTConsumer {
public:
    explicit MyClangConsumer(ASTContext *Context) : Visitor(Context) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        Visitor.PrintResults();
    }

private:
    MyClangVisitor Visitor;
};

class MyClangPlugin : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<MyClangConsumer>(&CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &Args) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<MyClangPlugin>
    X("myClangPlugin", "Counts implicit type conversions");
