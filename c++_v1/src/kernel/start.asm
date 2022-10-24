[bits 32]

magic   equ 0xe85250d6
length  equ header_end - header_start

;给 multiboot2 段 64*8bit 记录内容
section .multiboot2
header_start:
    dd magic
    dd header_end - header_start
    ; 填充 0
    times 64 - 2*32/8 -($ - $$) db 0
header_end:


extern kernel_init


;凭什么把它排到text段的最前面??根据调用顺序??
section .text
global _start
_start:

    mov esp, 0x10000; 修改栈顶
    xchg bx, bx; 断点
    push ebx; ards_count
    push eax; magic
    xchg bx, bx; 断点
    ; xchg bx, bx
    call kernel_init    ; 内核初始化

    jmp $; 阻塞
