; ModuleID = '/home/liza/code/compiler-course-2025/llvm/test/compiler-course/frolova_e_fma_tests/test.cpp'
source_filename = "/home/liza/code/compiler-course-2025/llvm/test/compiler-course/frolova_e_fma_tests/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @_Z14test_fma_basicfff(float noundef %a, float noundef %b, float noundef %c) local_unnamed_addr #0 {
entry:
  %0 = tail call float @llvm.fma.f32(float %a, float %b, float %c)
  ret float %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @_Z17test_fma_same_regff(float noundef %a, float noundef %b) local_unnamed_addr #0 {
entry:
  %0 = tail call float @llvm.fma.f32(float %a, float %a, float %b)
  ret float %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @_Z11test_no_fmafff(float noundef %a, float noundef %b, float noundef %c) local_unnamed_addr #0 {
entry:
  %mul.i = fmul float %a, %b
  ret float %mul.i
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef float @_Z12test_mul_addfff(float noundef %a, float noundef %b, float noundef %c) local_unnamed_addr #0 {
entry:
  %mul.i = fmul float %a, %b
  %add.i = fadd float %mul.i, %c
  ret float %add.i
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fma.f32(float, float, float) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="128" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+cmov,+crc32,+cx8,+fma,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 19.1.6 (git@github.com:/ElizavetaFrolova/compiler-course-2025.git a1b8bd0d399561f504927f4fea7ce2e0b1709728)"}
