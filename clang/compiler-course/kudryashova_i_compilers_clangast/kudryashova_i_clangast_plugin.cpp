#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ParentMapContext.h"
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
           std::map<std::pair<std::string, std::string>, int>>
      m_functionStats;
  int m_totalConversions = 0;
  
public:
  explicit ImplicitConvVisitor(clang::ASTContext *context)
      : m_context(context) {}

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *ICE) {

    auto castKind = ICE->getCastKind();
    if (castKind == clang::CK_LValueToRValue || castKind == clang::CK_NoOp ||
        castKind == clang::CK_FunctionToPointerDecay) {
      return true;
    }
    clang::QualType SourceType = ICE->getSubExpr()->getType();
    clang::QualType TargetType = ICE->getType();

    if (SourceType.getCanonicalType() == TargetType.getCanonicalType()) {
      return true;
    }

    auto Parents = m_context->getParents(*ICE);
    while (!Parents.empty()) {
      if (const auto *FD = Parents[0].get<clang::FunctionDecl>()) {
        recordConversion(FD, SourceType, TargetType);
        break;
      } else if (const auto *Lambda = Parents[0].get<clang::LambdaExpr>()) {
        if (const auto *CallOp = Lambda->getCallOperator()) {
          recordConversion(CallOp, SourceType, TargetType);
          break;
        }
      } else if (const auto *ME = Parents[0].get<clang::CXXMethodDecl>()) {
		recordConversion(ME, SourceType, TargetType);
        break;
      }
      Parents = m_context->getParents(Parents[0]);
    }
    return true;
  }

  std::string normalizeTypeName(std::string typeName) {
    const std::vector<std::string> prefixes = {"struct ", "class "};
    for (const auto &prefix : prefixes) {
      size_t pos = typeName.find(prefix);
      if (pos == 0) {
        typeName.erase(0, prefix.length());
        break;
      }
    }
    typeName.erase(std::remove_if(typeName.begin(), typeName.end(), ::isspace),
                   typeName.end());
    size_t pos;
    while ((pos = typeName.find("_Bool")) != std::string::npos) {
      typeName.replace(pos, 5, "bool");
    }
    return typeName;
  }

  void recordConversion(const clang::FunctionDecl *FD, clang::QualType From,
                        clang::QualType To) {
    std::string FromStr =
        normalizeTypeName(From.getCanonicalType().getAsString());
    std::string ToStr = normalizeTypeName(To.getCanonicalType().getAsString());
    m_functionStats[FD][std::make_pair(FromStr, ToStr)]++;
    m_totalConversions++;
  }

  void printStats(llvm::raw_ostream &OS) {
    for (const auto &[func, convs] : m_functionStats) {
      OS << "Function `" << func->getName() << "`\n";
      for (const auto &[conv, num] : convs) {
        OS << conv.first << " -> " << conv.second << ": " << num << "\n";
      }
    }
    OS << "Total implicit conversions: " << m_totalConversions << "\n";
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
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
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