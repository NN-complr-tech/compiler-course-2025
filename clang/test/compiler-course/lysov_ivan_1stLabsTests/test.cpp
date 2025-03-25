// RUN: %clang_cc1 -load %llvmshlibdir/unUsedVarPlugin_LysovIvan_FIIT3_ClangAST%pluginext -plugin unUsedVarPlugin_LysovIvan_FIIT3_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

//CHECK: int FunctionFromTheProblemStatement(int a, int b, int c) {
//CHECK: \[\[maybe_unused\]\] double value = 0.0;
//CHECK: return a + b;
int FunctionFromTheProblemStatement(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

//CHECK: void test_params(int used, \[\[maybe_unused\]\] int unused) {
//CHECK: int x = used;
//CHECK: return x;
int test_params(int used, int unused) {
    int x = used;
    return x;
}

//CHECK: \[\[maybe_unused\]\] int globalVar = 6;
int globalVar = 6;

//CHECK: void checkWithoutReturn(int a, \[\[maybe_unused\]\] long d){
//CHECK: \[\[maybe_unused\]\] int g = a;
void checkWithoutReturn(int a, long d){
    int g = a;
}

//CHECK: void checkConstantParamsFunction(\[\[maybe_unused\]\] const int x, float j){
//CHECK: \[\[maybe_unused\]\] float y = j ;
void checkConstantParamsFunction(const int x, float j){
    float y = j;
}
