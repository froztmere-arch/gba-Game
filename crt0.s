    .section .text.start, "ax"
    .arm
    .global _start
_start:
    b       start_vector
    .fill   156,1,0              @ Nintendo logo (gbafix fills this in)
    .ascii  "TINYFARM\0\0\0\0"   @ title (12 bytes)
    .ascii  "ATFE"               @ game code
    .ascii  "01"                 @ maker
    .byte   0x96
    .byte   0,0
    .fill   7,1,0
    .byte   0                    @ version
    .byte   0                    @ header checksum (gbafix)
    .byte   0,0
start_vector:
    mov     r0, #0x12            @ IRQ mode
    msr     cpsr_c, r0
    ldr     sp, =0x03007FA0
    mov     r0, #0x1F            @ System mode
    msr     cpsr_c, r0
    ldr     sp, =0x03007F00
    @ copy .data from ROM to IWRAM
    ldr     r0, =__data_lma
    ldr     r1, =__data_start
    ldr     r2, =__data_end
1:  cmp     r1, r2
    ldrlo   r3, [r0], #4
    strlo   r3, [r1], #4
    blo     1b
    @ zero .bss
    ldr     r1, =__bss_start
    ldr     r2, =__bss_end
    mov     r3, #0
2:  cmp     r1, r2
    strlo   r3, [r1], #4
    blo     2b
    ldr     r0, =main
    bx      r0
3:  b       3b
    .ltorg
