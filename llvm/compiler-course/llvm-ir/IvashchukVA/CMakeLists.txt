add_llvm_pass_plugin(IvashchukVA_lab2
  IvashchukVA_lab2.cpp
  
  DEPENDS
  intrinsics_gen
  )
  set(LLVM_LIT_COMPONENTS compiler-course)
