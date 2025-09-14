
; RUN: opt -load-pass-plugin %llvmshlibdir/FMAPass_VelievElvin_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="fmapass" -S %s | FileCheck %s

; TEST 1. Basic case of a*b + c
; CHECK-LABEL: @_Z2f1ddd
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: ret double %fma
define dso_local noundef double @_Z2f1ddd(double noundef %0, double noundef %1, double noundef %2) local_unnamed_addr {
  %mul = fmul double %0, %1
  %add = fadd double %mul, %2
  ret double %add
}

; TEST 2. Two fmuladd calls in sequence
; CHECK-LABEL: @_Z2f2ddddd
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: %fma1 = call double @llvm.fmuladd.f64(double %3, double %4, double %fma)
; CHECK-NEXT: ret double %fma1
define dso_local noundef double @_Z2f2ddddd(double noundef %0, double noundef %1, double noundef %2, double noundef %3, double noundef %4) local_unnamed_addr {
  %mul1 = fmul double %0, %1
  %add1 = fadd double %mul1, %2
  %mul2 = fmul double %3, %4
  %add2 = fadd double %add1, %mul2
  ret double %add2
}

; TEST 3. Reversed order c + a*b
; CHECK-LABEL: @_Z2f3ddd
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: ret double %fma
define dso_local noundef double @_Z2f3ddd(double noundef %0, double noundef %1, double noundef %2) local_unnamed_addr {
  %mul = fmul double %0, %1
  %add = fadd double %2, %mul
  ret double %add
}

; TEST 4. Constant multiplier in FMA a*2.0 + b
; CHECK-LABEL: @_Z2f4ff
; CHECK-NEXT: %fma = call float @llvm.fmuladd.f32(float %0, float 2.000000e+00, float %1)
; CHECK-NEXT: ret float %fma
define dso_local noundef float @_Z2f4ff(float noundef %0, float noundef %1) local_unnamed_addr {
  %mul = fmul float %0, 2.000000e+00
  %add = fadd float %mul, %1
  ret float %add
}

; TEST 5. FMA result used in subtraction
; CHECK-LABEL: @_Z2f5dddd
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: %sub = fsub double %fma, %3
; CHECK-NEXT: ret double %sub
define dso_local noundef double @_Z2f5dddd(double noundef %0, double noundef %1, double noundef %2, double noundef %3) local_unnamed_addr {
  %mul = fmul double %0, %1
  %add = fadd double %mul, %2
  %sub = fsub double %add, %3
  ret double %sub
}

; TEST 6. FMA result consumed by division
; CHECK-LABEL: @_Z2f6fff
; CHECK-NEXT: %fma = call float @llvm.fmuladd.f32(float %0, float %1, float %2)
; CHECK-NEXT: %div = fdiv float %fma, %0
; CHECK-NEXT: ret float %div
define dso_local noundef float @_Z2f6fff(float noundef %0, float noundef %1, float noundef %2) local_unnamed_addr {
  %mul = fmul float %0, %1
  %add = fadd float %mul, %2
  %div = fdiv float %add, %0
  ret float %div
}

; TEST 7. Same multiplication used twice, cannot fuse
; CHECK-LABEL: @_Z2f7ddd
; CHECK: fmul double %0, %1
; CHECK: fadd double
; CHECK: fadd double
define dso_local noundef double @_Z2f7ddd(double noundef %0, double noundef %1, double noundef %2) local_unnamed_addr {
  %mul = fmul double %0, %1
  %add1 = fadd double %mul, %2
  %add2 = fadd double %mul, %add1
  ret double %add2
}

; TEST 8. Type mismatch float * float + double
; CHECK-LABEL: @_Z2f8ffd
; CHECK: fmul float %0, %1
; CHECK: fpext float %{{.*}} to double
; CHECK: fadd double
define dso_local noundef double @_Z2f8ffd(float noundef %0, float noundef %1, double noundef %2) local_unnamed_addr {
  %mul = fmul float %0, %1
  %ext = fpext float %mul to double
  %add = fadd double %ext, %2
  ret double %add
}

; TEST 9. Integer multiplication + addition, no FMA
; CHECK-LABEL: @_Z2f9iii
; CHECK: mul nsw i32 %0, %1
; CHECK-NEXT: add nsw i32 %{{.*}}, %2
define dso_local noundef i32 @_Z2f9iii(i32 noundef %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr {
  %mul = mul nsw i32 %0, %1
  %add = add nsw i32 %mul, %2
  ret i32 %add
}

; TEST 10. Integer multiplication + addition, no FMA
; CHECK-LABEL: @_Z3f10xxx
; CHECK: mul nsw i64 %0, %1
; CHECK-NEXT: add nsw i64 %{{.*}}, %2
define dso_local noundef i64 @_Z3f10xxx(i64 noundef %0, i64 noundef %1, i64 noundef %2) local_unnamed_addr {
  %mul = mul nsw i64 %0, %1
  %add = add nsw i64 %mul, %2
  ret i64 %add
}