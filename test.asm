BITS 64

global _start
_start:
    MOV RAX, 1
    DEC RAX

    MOV RAX, 60
    XOR RDI, RDI
    SYSCALL

