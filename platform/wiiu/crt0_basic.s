.section .text
.global _start

_start:
    # Set up stack pointer (basic setup)
    lis %r1, 0x1
    ori %r1, %r1, 0x8000
    subi %r1, %r1, 8
    
    # Call main
    bl main
    
    # Exit (return code in r3)
    li %r0, 0
    sc
