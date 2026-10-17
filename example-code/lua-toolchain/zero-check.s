# GNU assembler input: interoperability and zero-filled storage, chapter 60.
.text
.global _start
_start:
 call helper
 mov scratch(%rip), %edi
 add %eax, %edi
 mov $60, %eax
 syscall
.bss
.align 16
scratch: .skip 32
