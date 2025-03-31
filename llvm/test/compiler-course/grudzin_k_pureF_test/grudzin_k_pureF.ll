; RUN: opt -load-pass-plugin %llvmshlibdir/MarkPurePass_Grudzin_Konstantin_FIIT1_LLVM_IR%pluginext -passes="mark-pure-pass" -S %s | FileCheck %s

; Данный комментарий появился здесь для того, чтобы не забыть это на будующей защите:
; Почему я использую #0 (т.к так помечает эти функции IR, в строке компиляции видно, что он ставит после объявления #0 и добавляет в конце attributes #0 = {"pure"})


; CHECK: define dso_local noundef i32 @f(i32 noundef %0) local_unnamed_addr #0
define dso_local noundef i32 @f(i32 noundef %0) local_unnamed_addr {
  %2 = mul nsw i32 %0, %0
  ret i32 %2
}

; CHECK: define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0
define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

; CHECK: define dso_local noundef i32 @_Z4addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0
define dso_local noundef i32 @_Z4addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0{
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

@x = dso_local local_unnamed_addr global i32 1, align 4
; CHECK-NOT: dso_local noundef i32 @f1() local_unnamed_addr #0
define dso_local noundef i32 @f1() local_unnamed_addr {
  %1 = load i32, ptr @x, align 4
  %2 = mul nsw i32 %1, %1
  ret i32 %2
}

; CHECK-NOT: dso_local noundef i32 @f2(i32*) local_unnamed_addr #0
define dso_local noundef i32 @f2(i32*) local_unnamed_addr {
  %2 = load i32, ptr %0, align 4
  ret i32 %2
}


@y = global i32 0, align 4
; CHECK-NOT: define dso_local noundef i32 @f3() local_unnamed_addr #0
define dso_local noundef i32 @f3() local_unnamed_addr {
entry:
  %1 = load i32, ptr @y, align 4 
  %2 = add nsw i32 %1, 1         
  store i32 %2, ptr @y, align 4  
  ret i32 %2                     
}

; CHECK: attributes #0 = { "pure" }
attributes #0 = { "pure" }
