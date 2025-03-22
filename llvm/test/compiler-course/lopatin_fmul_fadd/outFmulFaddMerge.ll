; ModuleID = 'testFmulFaddMerge.ll'
source_filename = "testFmulFaddMerge.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef double @_Z9fmaDoubleddd(double noundef %a, double noundef %b, double noundef %c) local_unnamed_addr #0 {
entry:
  %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  ret double %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @_Z8fmaFloatfff(float noundef %a, float noundef %b, float noundef %c) local_unnamed_addr #0 {
entry:
  %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  %1 = call float @llvm.fmuladd.f32(float %b, float %c, float %a)
  %mul3 = fmul float %1, %b
  %add4 = fadd float %0, %mul3
  ret float %add4
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 19.1.6 (git@github.com:ascannel/compiler-course-2025.git 58509a0178caa79f319e093e0ef1933d7b368d62)"}
