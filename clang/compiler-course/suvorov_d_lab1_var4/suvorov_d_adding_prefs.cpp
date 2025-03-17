#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <vector>

namespace {
class RenameVisitor final : public clang::RecursiveASTVisitor<RenameVisitor> {
public:
  explicit RenameVisitor(clang::ASTContext *context, clang::Rewriter &rewriter,
                         bool logChanges)
      : m_rewriter(rewriter), m_logChanges(logChanges) {}

  std::string getPrefixForVarDecl(const clang::VarDecl *varDecl) const {
    if (varDecl->isStaticLocal()) {
      return "static_";
    } else if (varDecl->hasGlobalStorage() && !varDecl->isStaticLocal()) {
      return "global_";
    } else if (varDecl->isLocalVarDecl() && !varDecl->isStaticLocal()) {
      return "local_";
    }
    return "";
  }

  bool VisitVarDecl(clang::VarDecl *varDecl) {
    if (varDecl->getLocation().isInvalid() || !varDecl->getIdentifier())
      return true;

    const std::string originalName = varDecl->getNameAsString();
    const std::string prefix = getPrefixForVarDecl(varDecl);

    if (!prefix.empty() && originalName.find(prefix) != 0) {
      const std::string updatedName = prefix + originalName;
      m_nameMap[varDecl] = updatedName;
      m_rewriter.ReplaceText(varDecl->getLocation(), originalName.size(),
                             updatedName);

      if (m_logChanges) {
        llvm::outs() << "Renamed variable at "
                     << varDecl->getLocation().printToString(
                            m_rewriter.getSourceMgr())
                     << ": " << originalName << " -> " << updatedName << "\n";
      }
    }
    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *paramDecl) {
    if (paramDecl->getLocation().isInvalid() || !paramDecl->getIdentifier())
      return true;

    const std::string originalName = paramDecl->getNameAsString();
    const std::string prefix = "param_";

    if (originalName.find(prefix) != 0) {
      const std::string updatedName = prefix + originalName;
      m_nameMap[paramDecl] = updatedName;
      m_rewriter.ReplaceText(paramDecl->getLocation(), originalName.size(),
                             updatedName);

      if (m_logChanges) {
        llvm::outs() << "Renamed parameter at "
                     << paramDecl->getLocation().printToString(
                            m_rewriter.getSourceMgr())
                     << ": " << originalName << " -> " << updatedName << "\n";
      }
    }
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *declRef) {
    if (auto *varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
      auto it = m_nameMap.find(varDecl);
      if (it != m_nameMap.end()) {
        const std::string originalName = varDecl->getNameAsString();
        const std::string updatedName = it->second;
        m_rewriter.ReplaceText(declRef->getLocation(), originalName.size(),
                               updatedName);

        if (m_logChanges) {
          llvm::outs() << "Updated reference at "
                       << declRef->getLocation().printToString(
                              m_rewriter.getSourceMgr())
                       << ": " << originalName << " -> " << updatedName << "\n";
        }
      }
    }
    return true;
  }

private:
  clang::Rewriter &m_rewriter;
  bool m_logChanges;
  std::unordered_map<clang::VarDecl *, std::string> m_nameMap;
};

class RenameConsumer final : public clang::ASTConsumer {
public:
  explicit RenameConsumer(clang::ASTContext *context, clang::Rewriter &rewriter,
                          bool logChanges)
      : m_visitor(context, rewriter, logChanges) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  RenameVisitor m_visitor;
};

class RenameAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<RenameConsumer>(&ci.getASTContext(), m_rewriter,
                                            m_logChanges);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg == "--help") {
        llvm::outs()
            << "Rename Plugin - Adds prefixes to variables and parameters.\n";
        llvm::outs() << "Usage:\n";
        llvm::outs() << "  --no-log         Disable logging of changes\n";
        llvm::outs() << "  --help           Show this help message\n";
        return true;
      } else if (arg == "--no-log") {
        m_logChanges = false;
      }
    }
    return true;
  }

  void EndSourceFileAction() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  clang::Rewriter m_rewriter;
  bool m_logChanges = true;
};

} // namespace

static clang::FrontendPluginRegistry::Add<RenameAction>
    X("AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST",
      "Renames variables and parameters with appropriate prefixes");
