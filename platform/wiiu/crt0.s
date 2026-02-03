.section .init
.global _start
_start:
    # Set up stack pointer
    lis %r1, 0x1
    ori %r1, %r1, 0x8000
    subi %r1, %r1, 8
    
    # Set up arguments for main
    li %r3, 0      # argc
    li %r4, 0      # argv
    
    # Call main
    bl main
    
    # Exit with return code from main (in r3)
    li %r0, 1      # SYS_exit
    sc
