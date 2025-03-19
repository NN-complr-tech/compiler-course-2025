// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVariable_Solovyev_Danila_FIIT3_ClangAST%pluginext -plugin unused_variable_plugin -fsyntax-only %s 2>&1 | FileCheck %s

//CHECK: int foo(int a, int b, \[\[maybe_unused\]\] int c) {
//CHECK:      \[\[maybe_unused\]\] double value = 0.0;

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

//CHECK: bool bar(int a, bool b, \[\[maybe_unused\]\] double c=3.0){

bool bar(int a, bool b, double c=3.0){
    b=a++;
    return true;
}

//CHECK: \[\[maybe_unused\]\] int test;
//CHECK: \[\[maybe_unused\]\] bool c=b;

int test;
int b=1;
bool c=b;

//CHECK: A(int a,\[\[maybe_unused\]\] int b){

class A{
    A(int a,int b){
        a=1;
    }
};
