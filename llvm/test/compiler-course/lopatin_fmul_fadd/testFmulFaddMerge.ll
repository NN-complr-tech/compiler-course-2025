; ModuleID = 'testFmulFaddMerge.cpp'
source_filename = "testFmulFaddMerge.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef double @_Z9fmaDoubleddd(double noundef %a, double noundef %b, double noundef %c) local_unnamed_addr #0 {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  ret double %add
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @_Z8fmaFloatfff(float noundef %a, float noundef %b, float noundef %c) local_unnamed_addr #0 {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  %mul1 = fmul float %b, %c
  %add2 = fadd float %mul1, %a
  %mul3 = fmul float %add2, %b
  %add4 = fadd float %add, %mul3
  ret float %add4
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 19.1.6 (git@github.com:ascannel/compiler-course-2025.git 58509a0178caa79f319e093e0ef1933d7b368d62)"}
