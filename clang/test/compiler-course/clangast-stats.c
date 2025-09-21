// RUN: %clang_cc1 %s -triple x86_64-unknown-linux-gnu -emit-obj -o /dev/null
// RUN: compiler-course-clangast %s -- -std=c11 | FileCheck %s

int f(int n){
  int s = 0;
  for(int i=0;i<n;i++){       // for
    if(i%2==0) s += i;        // if + binops
    while(0){}                // while
  }
  return s;                   // return
}

// CHECK: CC-CLANGAST-REPORT
// CHECK: ifs: 1
// CHECK: fors: 1
// CHECK: whiles: 1
// CHECK: returns: 1
// CHECK: binops: 3
