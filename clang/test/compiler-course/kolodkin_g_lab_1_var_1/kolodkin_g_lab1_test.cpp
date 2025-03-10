// RUN: %clang_cc1 -load %llvmshlibdir/LabPlugin_KolodkinGrigorii_FIIT3_ClangAST%pluginext -plugin LabPlugin_KolodkinGrigorii_FIIT3_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Shape
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ area (double()|public|virtual|pure)

// CHECK: Circle -> Shape
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ radius (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ area (double()|public|override)

// CHECK: Rectangle -> Shape
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ width (double|public)
// CHECK-NEXT: | |_ height (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ area (double()|public|override)

struct Shape {
  virtual double area() = 0;
};

struct Circle : Shape {
public:
  double radius;

  double area() override {
    return 3.14 * radius * radius;
  }
};

struct Rectangle : Shape {
public:
  double width, height;

  double area() override {
    return width * height;
  }
};






