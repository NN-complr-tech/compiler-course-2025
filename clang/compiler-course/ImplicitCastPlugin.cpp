#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <string>

using namespace clang;

class ImplicitCastVisitor : public RecursiveASTVisitor<ImplicitCastVisitor> {
public:
    explicit ImplicitCastVisitor() = default;

    bool VisitFunctionDecl(const FunctionDecl *Func) {
        CurrentFunction = Func->getNameInfo().getName().getAsString();
        return true;
    }

    bool VisitImplicitCastExpr(const ImplicitCastExpr *Cast) {
        CastKind Kind = Cast->getCastKind();
        if (Kind == CK_NoOp || Kind == CK_LValueToRValue || Kind == CK_FunctionToPointerDecay) {
            return true;
        }

        QualType FromType = getRealType(Cast->getSubExpr()->getType());
        QualType ToType = getRealType(Cast->getType());

        if (FromType == ToType) {
            return true;
        }

        CastList.emplace_back(CastEntry{CurrentFunction, FromType.getAsString(), ToType.getAsString()});
        return true;
    }

    void PrintResults() {
        if (CastList.empty()) {
            llvm::outs() << "No implicit conversions found.\n";
            return;
        }

        llvm::StringRef LastFunction;
        for (const auto &Entry : CastList) {
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
        llvm::StringRef FunctionName;
        std::string FromType;
        std::string ToType;

        std::string getCastDescription() const {
            return FromType + " -> " + ToType;
        }
    };

    QualType getRealType(QualType Type) {
        Type = Type.getCanonicalType();
        if (const auto *TST = Type->getAs<TypedefType>()) {
            return TST->getDecl()->getUnderlyingType().getCanonicalType();
        }
        return Type;
    }

    std::string CurrentFunction;
    std::vector<CastEntry> CastList;
};

class ImplicitCastConsumer : public ASTConsumer {
public:
    explicit ImplicitCastConsumer() = default;

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        Visitor.PrintResults();
    }

private:
    ImplicitCastVisitor Visitor;
};

class ImplicitCastPlugin : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<ImplicitCastConsumer>();
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<ImplicitCastPlugin>
    X("implicit-cast-plugin", "Counts implicit type casts");
