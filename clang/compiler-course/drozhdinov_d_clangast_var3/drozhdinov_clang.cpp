#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

using namespace clang;

namespace {
class CastCounter : public clang::RecursiveASTVisitor<CastCounter> {
private:
  clang::ASTContext *m_context;
  std::string CurrentFunc;
  std::map<std::string, std::map<std::string, int>> transformations;
  std::map<const clang::FunctionDecl*, std::map<std::pair<std::string, std::string>, int>> tr;
public:
  explicit CastCounter(clang::ASTContext *context) : m_context(context) {}
  bool CheckImplicitCast(clang::ImplicitCastExpr *Expr) {
  	auto CastType = Expr->getCastKind();
  	/*
  	if (CastType == clang::CK_LValueToRValue ||
  		CastType == clang::CK_NoOp ||
  		CastType == clang::CK_FunctionToPointerDecay ||
  		CastType == clang::CK_ArrayToPointerDecay ||
  		CastType == clang::CK_NullToPointer) {
  		return true;
  	}*/
    clang::QualType SourceType = Expr->getSubExpr()->getType();
    clang::QualType DestType = Expr->getType();
    
    if (SourceType.getAsString() == DestType.getAsString()){
    	return true;
    }
    
    auto CurrentScope = clang::DynTypedNode::create<clang::ImplicitCastExpr>(*Expr);
    auto PrevScope = m_context->getParents(CurrentScope);
    
    while (!PrevScope.empty()) {
    	if (const clang::FunctionDecl *Decl = PrevScope[0].get<clang::FunctionDecl>()) {
    		std::string source = SourceType.getAsString();
    		std::string dest = DestType.getAsString();
    		tr[Decl][std::make_pair(source, dest)]++;
    		break;
    	}
    	CurrentScope = PrevScope[0];
    	PrevScope = m_context->getParents(CurrentScope);
    }
    return true;
  }
  
  void GetResult(){
  	for (const auto &t : tr) {
  		llvm::errs() << "Function `" << t.first->getName() << "`\n";
  		for (const auto &c : t.second) {
  			llvm::errs() << c.first.first << " -> " << c.first.second << ": " << c.second << "\n";
  		}
  	}
  }

};

class CastConsumer : public clang::ASTConsumer {
public:
  explicit CastConsumer(clang::ASTContext *context) : Counter(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    Counter.TraverseDecl(context.getTranslationUnitDecl());
    Counter.GetResult();
  }

private:
	CastCounter Counter;
};

class CastCounterAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<CastConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<CastCounterAction>
	X("CastCounter", "This plugin counts number of implicit casts");
