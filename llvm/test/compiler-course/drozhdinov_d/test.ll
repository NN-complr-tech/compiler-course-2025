; ModuleID = './llvm/test/compiler-course/drozhdinov_d/test.cpp'
source_filename = "./llvm/test/compiler-course/drozhdinov_d/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define dso_local float @_Z6fma_ssfff(float %0, float %1, float %2) local_unnamed_addr #0 {
  %4 = call float @llvm.fma.f32(float %0, float %1, float %2) #3
  ret float %4
}

; Function Attrs: nounwind readnone uwtable
define dso_local double @_Z6fma_sdddd(double %0, double %1, double %2) local_unnamed_addr #0 {
  %4 = call double @llvm.fma.f64(double %0, double %1, double %2) #3
  ret double %4
}

; Function Attrs: nounwind readnone uwtable
define dso_local <4 x float> @_Z6fma_psDv4_fS_S_(<4 x float> %0, <4 x float> %1, <4 x float> %2) local_unnamed_addr #0 {
  %4 = call <4 x float> @llvm.fma.v4f32(<4 x float> %0, <4 x float> %1, <4 x float> %2) #3
  ret <4 x float> %4
}

; Function Attrs: nounwind readnone uwtable
define dso_local <4 x double> @_Z6fma_pdDv4_dS_S_(<4 x double> %0, <4 x double> %1, <4 x double> %2) local_unnamed_addr #1 {
  %4 = call <4 x double> @llvm.fma.v4f64(<4 x double> %0, <4 x double> %1, <4 x double> %2) #3
  ret <4 x double> %4
}

; Function Attrs: nounwind readnone speculatable willreturn
declare float @llvm.fma.f32(float, float, float) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.fma.f64(double, double, double) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare <4 x float> @llvm.fma.v4f32(<4 x float>, <4 x float>, <4 x float>) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare <4 x double> @llvm.fma.v4f64(<4 x double>, <4 x double>, <4 x double>) #2

attributes #0 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+cx8,+fma,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+cx8,+fma,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
