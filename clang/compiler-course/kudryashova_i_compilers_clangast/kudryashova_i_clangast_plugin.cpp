#include "clang/AST/ParentMapContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class ImplicitConvVisitor 
  : public clang::RecursiveASTVisitor<ImplicitConvVisitor> {
	  
private:
  clang::ASTContext *m_context;
  std::map<const clang::FunctionDecl*, 
           std::map<std::pair<std::string, std::string>, int>> m_functionStats;
  int m_totalConversions = 0;
  
public:
  explicit ImplicitConvVisitor(clang::ASTContext *context) 
    : m_context(context) {}

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *ICE) {
	auto castKind = ICE->getCastKind();
    if (castKind == clang::CK_LValueToRValue || 
      castKind == clang::CK_NoOp ||
      castKind == clang::CK_FunctionToPointerDecay) {
      return true;
    }
    clang::QualType SourceType = ICE->getSubExpr()->getType();
    clang::QualType TargetType = ICE->getType();
	
	if (SourceType.getAsString() == TargetType.getAsString()) {
      return true;
    }

    auto Node = clang::DynTypedNode::create<clang::ImplicitCastExpr>(*ICE);
    auto Parents = m_context->getParents(Node);
    
    while (!Parents.empty()) {
      if (const clang::FunctionDecl *FD = 
          Parents[0].get<clang::FunctionDecl>()) {
          std::string From = SourceType.getAsString();
          std::string To = TargetType.getAsString();
            
          m_functionStats[FD][std::make_pair(From, To)]++;
          m_totalConversions++;
          break;
        }

      Node = Parents[0];
      Parents = m_context->getParents(Node);
    }
    return true;
  }


  void printStats(llvm::raw_ostream &OS) {
    for (const auto &funcEntry : m_functionStats) {
      OS << "Function `" << funcEntry.first->getName() << "`\n";
      for (const auto &conv : funcEntry.second) {
        OS << conv.first.first << " -> " 
           << conv.first.second << ": " 
           << conv.second << "\n";
      }
    }
    OS << "Total implicit conversions: " 
       << m_totalConversions << "\n";
  }
};

class ImplicitConvConsumer : public clang::ASTConsumer {
	
private:
  ImplicitConvVisitor m_visitor;

public:
  explicit ImplicitConvConsumer(clang::ASTContext *context) 
    : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_visitor.printStats(llvm::errs());
  }
};

class ImplicitConvAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ImplicitConvConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ImplicitConvAction>
X("ImplicitConvPlugin", "Count implicit type conversions");
