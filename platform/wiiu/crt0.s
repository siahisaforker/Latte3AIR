.section .init
.global _start
_start:
    # Set up stack and call main
    lis %r1, 0x1
    ori %r1, %r1, 0x8000
    li %r3, 0
    li %r4, 0
    bl main
    li %r3, 0
    bl exit
