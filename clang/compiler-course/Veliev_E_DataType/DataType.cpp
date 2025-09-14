#include "clang/AST/AST.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <unordered_set>
#include <vector>

namespace {

static std::string accessToString(clang::AccessSpecifier AS) {
  switch (AS) {
  case clang::AS_public:
    return "public";
  case clang::AS_protected:
    return "protected";
  case clang::AS_private:
    return "private";
  case clang::AS_none:
    return "none";
  }
  return "none";
}

static clang::AccessSpecifier normalizeBaseAS(clang::AccessSpecifier AS) {
  if (AS == clang::AS_none) {
    return clang::AS_public;
  }
  return AS;
}

static clang::AccessSpecifier
applyInheritance(clang::AccessSpecifier inheritAS,
                 clang::AccessSpecifier memberAS) {
  if (memberAS == clang::AS_private || memberAS == clang::AS_none) {
    return clang::AS_none;
  }
  switch (inheritAS) {
  case clang::AS_public:
    return memberAS;
  case clang::AS_protected:
    return clang::AS_protected;
  case clang::AS_private:
    return clang::AS_private;
  case clang::AS_none:
    return memberAS;
  }
  return memberAS;
}

static clang::AccessSpecifier
applyPath(const std::vector<clang::AccessSpecifier> &path,
          clang::AccessSpecifier memberAS) {
  clang::AccessSpecifier acc = memberAS;
  for (clang::AccessSpecifier ias : path) {
    acc = applyInheritance(ias, acc);
    if (acc == clang::AS_none) {
      break;
    }
  }
  return acc;
}

class TypePrinterVisitor
    : public clang::RecursiveASTVisitor<TypePrinterVisitor> {
public:
  explicit TypePrinterVisitor(clang::ASTContext *ctx)
      : PP(ctx->getPrintingPolicy()) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *RD) {
    if (!shouldProcess(RD)) {
      return true;
    }
    printHeader(RD);
    printBases(RD);
    llvm::outs() << "\n";
    printFields(RD);
    printMethods(RD);
    return true;
  }

private:
  const clang::PrintingPolicy &PP;

  bool shouldProcess(const clang::CXXRecordDecl *RD) {
    return RD->isCompleteDefinition() && !RD->isImplicit() &&
           !RD->isAnonymousStructOrUnion();
  }

  void printHeader(const clang::CXXRecordDecl *RD) {
    bool isTemplate = RD->getDescribedClassTemplate() != nullptr ||
                      llvm::isa<clang::ClassTemplateSpecializationDecl>(RD);
    std::string kind;
    if (RD->isUnion()) {
      kind = "union";
    } else if (RD->isStruct()) {
      kind = "struct";
    } else {
      kind = "class";
    }
    llvm::outs() << RD->getNameAsString() << "("
                 << (isTemplate ? "class|template" : kind) << ")";
  }

  void printBases(const clang::CXXRecordDecl *RD) {
    bool first = true;
    for (const clang::CXXBaseSpecifier &BS : RD->bases()) {
      llvm::outs() << (first ? " -> " : ", ");
      first = false;
      if (BS.isVirtual()) {
        llvm::outs() << "virtual ";
      }
      std::string accStr = accessToString(BS.getAccessSpecifier());
      if (accStr != "none") {
        llvm::outs() << accStr << " ";
      }
      llvm::outs() << BS.getType().getAsString(PP);
    }
  }

  void collectOverriddenInSelf(
      const clang::CXXRecordDecl *RD,
      std::unordered_set<const clang::CXXMethodDecl *> &overridden) {
    for (const clang::CXXMethodDecl *MD : RD->methods()) {
      for (auto *OM : MD->overridden_methods()) {
        overridden.insert(OM->getCanonicalDecl());
      }
    }
  }

  void traverseBases(
      const clang::CXXRecordDecl *RD, std::vector<clang::AccessSpecifier> &path,
      std::unordered_set<const clang::CXXRecordDecl *> &visited,
      const std::function<void(const clang::CXXRecordDecl *,
                               const std::vector<clang::AccessSpecifier> &)>
          &fn) {
    for (const clang::CXXBaseSpecifier &BS : RD->bases()) {
      const clang::Type *BT = BS.getType().getTypePtrOrNull();
      if (!BT) {
        continue;
      }
      const clang::CXXRecordDecl *BD = BT->getAsCXXRecordDecl();
      if (!BD || !BD->hasDefinition()) {
        continue;
      }
      if (visited.count(BD)) {
        continue;
      }
      visited.insert(BD);
      path.push_back(normalizeBaseAS(BS.getAccessSpecifier()));
      fn(BD, path);
      traverseBases(BD, path, visited, fn);
      path.pop_back();
      visited.erase(BD);
    }
  }

  void printFields(const clang::CXXRecordDecl *RD) {
    llvm::outs() << "|_Fields\n";
    std::vector<std::string> lines;
    std::unordered_set<const void *> seen;

    for (const clang::FieldDecl *FD : RD->fields()) {
      if (!FD->isImplicit()) {
        seen.insert(FD->getCanonicalDecl());
        lines.push_back("| |_ " + FD->getNameAsString() + " (" +
                        FD->getType().getAsString(PP) + "|" +
                        accessToString(FD->getAccess()) + ")");
      }
    }
    for (const clang::Decl *D : RD->decls()) {
      if (const auto *VD = llvm::dyn_cast<clang::VarDecl>(D)) {
        if (VD->isStaticDataMember()) {
          seen.insert(VD->getCanonicalDecl());
          lines.push_back("| |_ " + VD->getNameAsString() + " (" +
                          VD->getType().getAsString(PP) + "|static|" +
                          accessToString(VD->getAccess()) + ")");
        }
      }
    }

    std::vector<clang::AccessSpecifier> path;
    std::unordered_set<const clang::CXXRecordDecl *> visited;
    auto collectFieldsFromBase =
        [&](const clang::CXXRecordDecl *BD,
            const std::vector<clang::AccessSpecifier> &p) {
          for (const clang::FieldDecl *FD : BD->fields()) {
            if (FD->isImplicit()) {
              continue;
            }
            if (seen.count(FD->getCanonicalDecl())) {
              continue;
            }
            clang::AccessSpecifier eff = applyPath(p, FD->getAccess());
            if (eff == clang::AS_none) {
              continue;
            }
            seen.insert(FD->getCanonicalDecl());
            lines.push_back("| |_ " + FD->getNameAsString() + " (" +
                            FD->getType().getAsString(PP) + "|" +
                            accessToString(eff) + ")");
          }
          for (const clang::Decl *D : BD->decls()) {
            if (const auto *VD = llvm::dyn_cast<clang::VarDecl>(D)) {
              if (!VD->isStaticDataMember()) {
                continue;
              }
              if (seen.count(VD->getCanonicalDecl())) {
                continue;
              }
              clang::AccessSpecifier eff = applyPath(p, VD->getAccess());
              if (eff == clang::AS_none) {
                continue;
              }
              seen.insert(VD->getCanonicalDecl());
              lines.push_back("| |_ " + VD->getNameAsString() + " (" +
                              VD->getType().getAsString(PP) + "|static|" +
                              accessToString(eff) + ")");
            }
          }
        };
    traverseBases(RD, path, visited, collectFieldsFromBase);

    if (lines.empty()) {
      llvm::outs() << "| |_ (has no fields)\n";
    } else {
      for (auto &L : lines) {
        llvm::outs() << L << "\n";
      }
    }
  }

  std::string methodSignature(const clang::CXXMethodDecl *MD) {
    std::string sig = MD->getReturnType().getAsString(PP) + "(";
    bool first = true;
    for (const auto *P : MD->parameters()) {
      if (!first) {
        sig += ", ";
      }
      sig += P->getType().getAsString(PP);
      first = false;
    }
    sig += ")";
    return sig;
  }

  std::string methodAttrs(const clang::CXXMethodDecl *MD) {
    std::string attrs = accessToString(MD->getAccess());
    if (MD->isStatic()) {
      attrs = "static|" + attrs;
    }
    if (MD->isVirtual()) {
      if (MD->isPureVirtual()) {
        attrs += "|virtual|pure";
      } else if (MD->getAttr<clang::OverrideAttr>() ||
                 MD->size_overridden_methods() > 0) {
        attrs += "|override";
      } else {
        attrs += "|virtual";
      }
    }
    if (MD->isConst()) {
      attrs += "|const";
    }
    if (MD->isConstexpr()) {
      attrs += "|constexpr";
    }
    return attrs;
  }

  void printMethods(const clang::CXXRecordDecl *RD) {
    llvm::outs() << "|_Methods\n";
    std::vector<std::string> lines;
    std::unordered_set<const void *> seen;
    std::unordered_set<const clang::CXXMethodDecl *> overriddenInSelf;
    collectOverriddenInSelf(RD, overriddenInSelf);

    for (const clang::CXXMethodDecl *MD : RD->methods()) {
      if (MD->isImplicit()) {
        continue;
      }
      seen.insert(MD->getCanonicalDecl());
      std::string name = MD->getNameAsString();
      if (name.empty()) {
        name = "(anonymous)";
      }
      lines.push_back("| |_ " + name + " (" + methodSignature(MD) + "|" +
                      methodAttrs(MD) + ")");
    }

    std::vector<clang::AccessSpecifier> path;
    std::unordered_set<const clang::CXXRecordDecl *> visited;
    auto collectMethodsFromBase =
        [&](const clang::CXXRecordDecl *BD,
            const std::vector<clang::AccessSpecifier> &p) {
          for (const clang::CXXMethodDecl *MD : BD->methods()) {
            if (MD->isImplicit()) {
              continue;
            }
            const clang::CXXMethodDecl *Canon = MD->getCanonicalDecl();
            if (seen.count(Canon)) {
              continue;
            }
            if (overriddenInSelf.count(Canon)) {
              continue;
            }
            clang::AccessSpecifier eff = applyPath(p, MD->getAccess());
            if (eff == clang::AS_none) {
              continue;
            }
            std::string name = MD->getNameAsString();
            if (name.empty()) {
              name = "(anonymous)";
            }
            std::string attrs = accessToString(eff);
            if (MD->isStatic()) {
              attrs = "static|" + attrs;
            }
            if (MD->isVirtual() && MD->isPureVirtual()) {
              attrs += "|virtual|pure";
            } else if (MD->isVirtual()) {
              attrs += "|virtual";
            }
            if (MD->isConst()) {
              attrs += "|const";
            }
            if (MD->isConstexpr()) {
              attrs += "|constexpr";
            }
            seen.insert(Canon);
            lines.push_back("| |_ " + name + " (" + methodSignature(MD) + "|" +
                            attrs + ")");
          }
        };
    traverseBases(RD, path, visited, collectMethodsFromBase);

    if (lines.empty()) {
      llvm::outs() << "| |_ (has no methods)\n";
    } else {
      for (auto &L : lines) {
        llvm::outs() << L << "\n";
      }
    }
  }
};

class PrintTypesConsumer : public clang::ASTConsumer {
public:
  explicit PrintTypesConsumer(clang::ASTContext *ctx) : Visitor(ctx) {}
  void HandleTranslationUnit(clang::ASTContext &context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  TypePrinterVisitor Visitor;
};

class PrintTypesAction : public clang::PluginASTAction {
protected:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<PrintTypesConsumer>(&CI.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    (void)CI;
    (void)args;
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<PrintTypesAction>
    X("DataTypeLab_VelievElvin_FIIT1_ClangAST",
      "Print user type fields/methods/base classes");
