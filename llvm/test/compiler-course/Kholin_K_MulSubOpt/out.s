	.text
	.file	"Kholin_K_MulSub_test.cpp"
                                        # Start of file scope inline assembly
	.globl	_ZSt21ios_base_library_initv

                                        # End of file scope inline assembly
	.globl	_Z20classic_mul_sub_testfff     # -- Begin function _Z20classic_mul_sub_testfff
	.p2align	4, 0x90
	.type	_Z20classic_mul_sub_testfff,@function
_Z20classic_mul_sub_testfff:            # @_Z20classic_mul_sub_testfff
	.cfi_startproc
# %bb.0:                                # %entry
	vfnmadd213ss	%xmm2, %xmm1, %xmm0     # xmm0 = -(xmm1 * xmm0) + xmm2
	retq
.Lfunc_end0:
	.size	_Z20classic_mul_sub_testfff, .Lfunc_end0-_Z20classic_mul_sub_testfff
	.cfi_endproc
                                        # -- End function
	.globl	_Z28reverse_classic_mul_sub_testfff # -- Begin function _Z28reverse_classic_mul_sub_testfff
	.p2align	4, 0x90
	.type	_Z28reverse_classic_mul_sub_testfff,@function
_Z28reverse_classic_mul_sub_testfff:    # @_Z28reverse_classic_mul_sub_testfff
	.cfi_startproc
# %bb.0:                                # %entry
	vfnmadd213ss	%xmm2, %xmm1, %xmm0     # xmm0 = -(xmm1 * xmm0) + xmm2
	retq
.Lfunc_end1:
	.size	_Z28reverse_classic_mul_sub_testfff, .Lfunc_end1-_Z28reverse_classic_mul_sub_testfff
	.cfi_endproc
                                        # -- End function
	.globl	_Z19return_mul_sub_testfff      # -- Begin function _Z19return_mul_sub_testfff
	.p2align	4, 0x90
	.type	_Z19return_mul_sub_testfff,@function
_Z19return_mul_sub_testfff:             # @_Z19return_mul_sub_testfff
	.cfi_startproc
# %bb.0:                                # %entry
	vfnmadd213ss	%xmm2, %xmm1, %xmm0     # xmm0 = -(xmm1 * xmm0) + xmm2
	retq
.Lfunc_end2:
	.size	_Z19return_mul_sub_testfff, .Lfunc_end2-_Z19return_mul_sub_testfff
	.cfi_endproc
                                        # -- End function
	.globl	_Z21multiple_mul_sub_testfff    # -- Begin function _Z21multiple_mul_sub_testfff
	.p2align	4, 0x90
	.type	_Z21multiple_mul_sub_testfff,@function
_Z21multiple_mul_sub_testfff:           # @_Z21multiple_mul_sub_testfff
	.cfi_startproc
# %bb.0:                                # %entry
	vmovaps	%xmm1, %xmm3
	vfnmadd213ss	%xmm2, %xmm0, %xmm3     # xmm3 = -(xmm0 * xmm3) + xmm2
	vfnmadd213ss	%xmm0, %xmm1, %xmm2     # xmm2 = -(xmm1 * xmm2) + xmm0
	vaddss	%xmm2, %xmm3, %xmm0
	retq
.Lfunc_end3:
	.size	_Z21multiple_mul_sub_testfff, .Lfunc_end3-_Z21multiple_mul_sub_testfff
	.cfi_endproc
                                        # -- End function
	.globl	_Z27classic_double_mul_sub_testddd # -- Begin function _Z27classic_double_mul_sub_testddd
	.p2align	4, 0x90
	.type	_Z27classic_double_mul_sub_testddd,@function
_Z27classic_double_mul_sub_testddd:     # @_Z27classic_double_mul_sub_testddd
	.cfi_startproc
# %bb.0:                                # %entry
	vfnmadd213sd	%xmm2, %xmm1, %xmm0     # xmm0 = -(xmm1 * xmm0) + xmm2
	retq
.Lfunc_end4:
	.size	_Z27classic_double_mul_sub_testddd, .Lfunc_end4-_Z27classic_double_mul_sub_testddd
	.cfi_endproc
                                        # -- End function
	.globl	_Z19nested_mul_sub_testffff     # -- Begin function _Z19nested_mul_sub_testffff
	.p2align	4, 0x90
	.type	_Z19nested_mul_sub_testffff,@function
_Z19nested_mul_sub_testffff:            # @_Z19nested_mul_sub_testffff
	.cfi_startproc
# %bb.0:                                # %entry
	vfnmadd213ss	%xmm2, %xmm1, %xmm0     # xmm0 = -(xmm1 * xmm0) + xmm2
	vmulss	%xmm3, %xmm0, %xmm0
	retq
.Lfunc_end5:
	.size	_Z19nested_mul_sub_testffff, .Lfunc_end5-_Z19nested_mul_sub_testffff
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function main
.LCPI6_0:
	.quad	0x4004000000000000              # double 2.5
.LCPI6_1:
	.quad	0xc01e000000000000              # double -7.5
.LCPI6_2:
	.quad	0x3ffe000000000000              # double 1.875
.LCPI6_3:
	.quad	0x4010000000000000              # double 4
	.text
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	pushq	%r14
	.cfi_def_cfa_offset 16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	pushq	%rax
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -24
	.cfi_offset %r14, -16
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	vmovsd	.LCPI6_0(%rip), %xmm0           # xmm0 = [2.5E+0,0.0E+0]
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB6_25
# %bb.1:                                # %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i
	cmpb	$0, 56(%rbx)
	je	.LBB6_3
# %bb.2:                                # %if.then.i4.i.i
	movzbl	67(%rbx), %ecx
	jmp	.LBB6_4
.LBB6_3:                                # %if.end.i.i.i
	movq	%rbx, %rdi
	movq	%rax, %r14
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%r14, %rax
.LBB6_4:                                # %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	vmovsd	.LCPI6_0(%rip), %xmm0           # xmm0 = [2.5E+0,0.0E+0]
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB6_25
# %bb.5:                                # %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i35
	cmpb	$0, 56(%rbx)
	je	.LBB6_7
# %bb.6:                                # %if.then.i4.i.i38
	movzbl	67(%rbx), %ecx
	jmp	.LBB6_8
.LBB6_7:                                # %if.end.i.i.i43
	movq	%rbx, %rdi
	movq	%rax, %r14
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%r14, %rax
.LBB6_8:                                # %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit48
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	vmovsd	.LCPI6_0(%rip), %xmm0           # xmm0 = [2.5E+0,0.0E+0]
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB6_25
# %bb.9:                                # %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i55
	cmpb	$0, 56(%rbx)
	je	.LBB6_11
# %bb.10:                               # %if.then.i4.i.i58
	movzbl	67(%rbx), %ecx
	jmp	.LBB6_12
.LBB6_11:                               # %if.end.i.i.i63
	movq	%rbx, %rdi
	movq	%rax, %r14
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%r14, %rax
.LBB6_12:                               # %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit68
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	vmovsd	.LCPI6_1(%rip), %xmm0           # xmm0 = [-7.5E+0,0.0E+0]
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB6_25
# %bb.13:                               # %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i75
	cmpb	$0, 56(%rbx)
	je	.LBB6_15
# %bb.14:                               # %if.then.i4.i.i78
	movzbl	67(%rbx), %ecx
	jmp	.LBB6_16
.LBB6_15:                               # %if.end.i.i.i83
	movq	%rbx, %rdi
	movq	%rax, %r14
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%r14, %rax
.LBB6_16:                               # %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit88
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	vmovsd	.LCPI6_2(%rip), %xmm0           # xmm0 = [1.875E+0,0.0E+0]
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB6_25
# %bb.17:                               # %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i95
	cmpb	$0, 56(%rbx)
	je	.LBB6_19
# %bb.18:                               # %if.then.i4.i.i98
	movzbl	67(%rbx), %ecx
	jmp	.LBB6_20
.LBB6_19:                               # %if.end.i.i.i103
	movq	%rbx, %rdi
	movq	%rax, %r14
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%r14, %rax
.LBB6_20:                               # %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit108
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	movq	_ZSt4cout@GOTPCREL(%rip), %rdi
	vmovsd	.LCPI6_3(%rip), %xmm0           # xmm0 = [4.0E+0,0.0E+0]
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	(%rax), %rcx
	movq	-24(%rcx), %rcx
	movq	240(%rax,%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB6_25
# %bb.21:                               # %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit.i.i115
	cmpb	$0, 56(%rbx)
	je	.LBB6_23
# %bb.22:                               # %if.then.i4.i.i118
	movzbl	67(%rbx), %ecx
	jmp	.LBB6_24
.LBB6_23:                               # %if.end.i.i.i123
	movq	%rbx, %rdi
	movq	%rax, %r14
	callq	_ZNKSt5ctypeIcE13_M_widen_initEv@PLT
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	movl	$10, %esi
	callq	*48(%rax)
	movl	%eax, %ecx
	movq	%r14, %rax
.LBB6_24:                               # %_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.exit128
	movsbl	%cl, %esi
	movq	%rax, %rdi
	callq	_ZNSo3putEc@PLT
	movq	%rax, %rdi
	callq	_ZNSo5flushEv@PLT
	xorl	%eax, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	retq
.LBB6_25:                               # %if.then.i.i.i
	.cfi_def_cfa_offset 32
	callq	_ZSt16__throw_bad_castv@PLT
.Lfunc_end6:
	.size	main, .Lfunc_end6-main
	.cfi_endproc
                                        # -- End function
	.ident	"clang version 19.1.6 (git@github.com:Kirius257/compiler-course-2025.git 8797851e87b0c3c25247c18dd167cbbea02581cd)"
	.section	".note.GNU-stack","",@progbits
