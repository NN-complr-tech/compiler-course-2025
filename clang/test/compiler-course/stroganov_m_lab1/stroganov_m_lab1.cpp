// RUN: %clang_cc1 -load %llvmshlibdir/PrintDataTypePlugin_Stroganov_Mikhail_FIIT2_ClangAST%pluginext -plugin PrintDataTypePlugin %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK-LABEL: Alpha(struct)
struct Alpha {};

// CHECK-LABEL: Beta(struct)
// CHECK-NOT: |_Fields
// CHECK: Beta::Alpha(struct)
struct Beta {
  struct Alpha {};
};

// CHECK-LABEL: Variant(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ i_val (int|public)
// CHECK-NEXT: | |_ f_val (float|public)
union Variant {
  int i_val;
  float f_val;
};

// CHECK-LABEL: VisibilityTest(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (float|private)
// CHECK-NEXT: | |_ y (double|protected)
// CHECK-NEXT: | |_ z (long long|public)
class VisibilityTest {
private:
  float x;
protected:
  double y;
public:
  long long z;
};

// CHECK-LABEL: PlainData(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (int|public)
// CHECK-NEXT: | |_ b (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ calculate (int|public)
struct PlainData {
  int a;
  double b;
  int calculate(int p1, int p2) { return 0; }
};

// CHECK-LABEL: WithStatic(class)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ helper (void|private|static)
class WithStatic {
  static void helper() {}
};

// CHECK-LABEL: TemplateType(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ val (T|private)
// CHECK-NEXT: | |_ index (int|private)
template<typename T>
class TemplateType {
  T val;
  int index;
};

// CHECK-LABEL: DualTemplate(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ left (T|private)
// CHECK-NEXT: | |_ right (U|private)
template<typename T, typename U>
class DualTemplate {
  T left;
  U right;
};

// CHECK-LABEL: AbstractBase(class)
class AbstractBase {
public:
  virtual ~AbstractBase() = default;
};

// CHECK-LABEL: PublicExtension(class)
// CHECK-NEXT: PublicExtension -> public AbstractBase
class PublicExtension : public AbstractBase {};

// CHECK-LABEL: ProtectedExtension(class)
// CHECK-NEXT: ProtectedExtension -> protected AbstractBase
class ProtectedExtension : protected AbstractBase {};

// CHECK-LABEL: PrivateExtension(class)
// CHECK-NEXT: PrivateExtension -> private AbstractBase
class PrivateExtension : private AbstractBase {};

// CHECK-LABEL: Wrapper(class)
class Wrapper {};

// CHECK-LABEL: Container(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ obj (Wrapper|public)
struct Container {
  Wrapper obj;
};

// CHECK-LABEL: Composite(class)
// CHECK-NEXT: Composite -> public Wrapper
// CHECK-NEXT: Composite -> public Container
class Composite : public Wrapper, public Container {};

// CHECK-LABEL: Human(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-DAG: | |_ ~Human (void|public|virtual)
// CHECK-DAG: | |_ rest (void|public|virtual|pure)
// CHECK-DAG: | |_ feed (void|public|virtual|pure)
struct Human {
  unsigned int age;
  unsigned int height;
  virtual ~Human() = default;
  virtual void rest() = 0;
  virtual void feed() = 0;
};

// CHECK-LABEL: Learner(class)
// CHECK-NEXT: Learner -> public Human
// CHECK-NEXT: |_Methods
// CHECK-DAG: | |_ rest (void|public|virtual|override)
// CHECK-DAG: | |_ feed (void|public|virtual|pure|override)
// CHECK-DAG: | |_ ~Learner (void|public|virtual|override)
class Learner : public Human {
public:
  void rest() override {}
  void feed() override = 0;
  ~Learner() override = default;
};
