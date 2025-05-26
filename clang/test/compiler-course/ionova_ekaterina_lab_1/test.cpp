// RUN: %clang_cc1 -load %llvmshlibdir/ClassInfoVisitorPlugin_Ionova_Ekaterina_FIIT1_ClangAST%pluginext -plugin ClassInfoVisitorPlugin_Ionova_Ekaterina_FIIT1_ClangAST %s 1>&1 | FileCheck %s

// CHECK: Point3D
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_x (double|public)
// CHECK-NEXT: | |_ m_y (double|public)
// CHECK-NEXT: | |_ m_z (double|public)
struct Point3D {
  double m_x{};
  double m_y{};
  double m_z{};
};

// CHECK: Pair
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ first (T1|public)
// CHECK-NEXT: | |_ second (T2|public)
template <typename T1, typename T2>
struct Pair {
  T1 first{};
  T2 second{};
};

// CHECK: EmptyClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
class EmptyClass {};

// CHECK: NodeList
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ data (T|public)
// CHECK-NEXT: | |_ next (NodeList<T> *|public)
template <typename T>
struct NodeList {
  T data{};
  NodeList<T> *next{};
};

// CHECK: CheckSpecifiers
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ publicMember (Point3D|public)
// CHECK-NEXT: | |_ protectedMember (Pair<int, NodeList<Pair<short, char> > >|protected)
// CHECK-NEXT: | |_ privateMember (EmptyClass|private)
class CheckSpecifiers {
public:
  Point3D publicMember{};
protected:
  Pair<int, NodeList<Pair<short, char>>> protectedMember{};
private:
  EmptyClass privateMember{};
};

// CHECK: User
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_id (int|public)
// CHECK-NEXT: | |_ m_human (Human|public)
// CHECK: Human
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_age (int|public)
// CHECK-NEXT: | |_ m_cash (int|public)
struct User {
  struct Human {
    int m_age{};
    int m_cash{};
  };
  int m_id{};
  Human m_human{};
};

// CHECK: CheckStatic
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_staticMember (float|private|static)
class CheckStatic {
  static float m_staticMember;
};
float CheckStatic::m_staticMember = 0.0f;

// CHECK: CheckConst
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_constMember (const char|private)
class CheckConst {
  const char m_constMember{};
};

// CHECK: CheckStaticConstTemplate
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_member (const T|private|static)
template <typename T>
class CheckStaticConstTemplate {
  static const T m_member;
};
template <typename T>
const T CheckStaticConstTemplate<T>::m_member{};

// CHECK: MultipleInheritance
// CHECK-NEXT: |_Base Classes: Base1, Base2, Base3
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_value (int|public)
class Base1 {
public:
  int base1Value{};
};

class Base2 {
public:
  double base2Value{};
};

class Base3 {
public:
  char base3Value{};
};

class MultipleInheritance : public Base1, public Base2, private Base3 {
public:
  int m_value{};
};
