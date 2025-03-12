// RUN: %clang_cc1 -load %llvmshlibdir/LabPlugin_KolodkinGrigorii_FIIT3_ClangAST%pluginext -plugin LabPlugin_KolodkinGrigorii_FIIT3_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Shape
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ area (double()|public|virtual|pure)

struct Shape {
  virtual double area() = 0;
};

// CHECK: Circle -> Shape
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ radius (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ area (double()|public|override)

struct Circle : Shape {
public:
  double radius;

  double area() override {
    return 3.14 * radius * radius;
  }
};

// CHECK: Rectangle -> Shape
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ width (double|public)
// CHECK-NEXT: | |_ height (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ area (double()|public|override)

struct Rectangle : Shape {
public:
  double width, height;

  double area() override {
    return width * height;
  }
};

// CHECK: ShapeA
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ valueA (int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ displayA (void()|public)

struct ShapeA {
public:
    int valueA;

    void displayA() {}
};

// CHECK: ShapeB
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ valueB (int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ displayB (void()|public)

struct ShapeB {
public:
    int valueB;

    void displayB() {}
};

// CHECK: EmptyClass
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (no methods)

class EmptyClass{};

// CHECK: CombinedShape -> ShapeA, ShapeB
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ (no friends)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ valueA (int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ displayB (void()|public)

class CombinedShape : public ShapeA, public ShapeB {
public:
    int valueA;

    void displayB() {}
};

void FriendFunc();

// CHECK: FriendClass
// CHECK-NEXT: |_Friends
// CHECK-NEXT: | |_ FriendFunc
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (no methods)

class FriendClass {
    friend void FriendFunc();
};
