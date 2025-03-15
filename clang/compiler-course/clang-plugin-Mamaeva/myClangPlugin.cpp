#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>

using namespace clang;

class MyClangVisitor : public RecursiveASTVisitor<MyClangVisitor> {
public:
    explicit MyClangVisitor(ASTContext *Context)
        : Context(Context) {}

    bool VisitImplicitCastExpr(ImplicitCastExpr *Expr) {
        QualType SourceType = Expr->getSubExpr()->getType();
        QualType TargetType = Expr->getType();

        // Игнорируем преобразования, связанные с указателями на функции
        if (SourceType->isFunctionType() || TargetType->isFunctionType()) {
            return true;
        }

        // Игнорируем преобразования, связанные с возвратом значений из функций
        if (isa<ReturnStmt>(Expr->getSubExpr()->IgnoreParenImpCasts())) {
            return true;
        }

        if (SourceType.getTypePtr() != TargetType.getTypePtr()) {
            std::string SourceTypeStr = SourceType.getAsString();
            std::string TargetTypeStr = TargetType.getAsString();
            std::string ConversionKey = SourceTypeStr + " -> " + TargetTypeStr;

            // Увеличиваем счетчик для данного преобразования
            ConversionCounts[ConversionKey]++;
            TotalConversions++;
        }

        return true;
    }

    void PrintConversionCounts(const std::string &FunctionName) {
        llvm::outs() << "Function " << FunctionName << "\n";
        for (const auto &Pair : ConversionCounts) {
            llvm::outs() << Pair.first << ": " << Pair.second << "\n";
        }
        ConversionCounts.clear(); // Очищаем счетчики для следующей функции
    }

    int GetTotalConversions() const {
        return TotalConversions;
    }

private:
    ASTContext *Context; // Указатель на ASTContext (не владеем им)
    std::map<std::string, int> ConversionCounts;
    int TotalConversions = 0;
};

class MyClangConsumer : public ASTConsumer {
public:
    explicit MyClangConsumer(ASTContext *Context)
        : Visitor(Context) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        // Обходим все функции в файле
        for (const auto *Decl : Context.getTranslationUnitDecl()->decls()) {
            if (const auto *FuncDecl = dyn_cast<FunctionDecl>(Decl)) {
                // Обрабатываем только функции sum и mul
                if (FuncDecl->getNameAsString() == "sum" || FuncDecl->getNameAsString() == "mul") {
                    Visitor.TraverseDecl(const_cast<FunctionDecl *>(FuncDecl));
                    Visitor.PrintConversionCounts(FuncDecl->getNameAsString());
                }
            }
        }

        // Убрали вывод общего количества преобразований
    }

private:
    MyClangVisitor Visitor;
};

class MyClangPlugin : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<MyClangConsumer>(&CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<MyClangPlugin>
    X("myClangPlugin", "Counts implicit type conversions");
