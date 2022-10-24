#include<interrupt.h>
#include<types.h>
#include<io.h>

extern "C" {

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

typedef struct test_interrupt_stack_t 
{
    int vector;
    u32 edi; 
    u32 esi; 
    u32 ebp; 
    u32 esp;
    u32 ebx; 
    u32 edx; 
    u32 ecx; 
    u32 eax;
    u32 gs; 
    u32 fs; 
    u32 es; 
    u32 ds;
    u32 vector0; 
    u32 error; 
    u32 eip; 
    u32 cs; 
    u32 eflags;
}__attribute__((__packed__)) test_interrupt_stack_t;
static test_interrupt_stack_t G_test_interrupt_stack;

static void exception_and_unshield_handler_default(int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags);

static void shield_handler_default(int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags);

}

#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI 0x20    // 通知中断控制器中断结束

#define EXCEPTION_AND_UNSHIELD_SIZE 0x20
#define SHIELD_SIZE 0x10
#define SOFT_SIZE 0xD0

#define NESSACERY_SIZE 0x30 // EXCEPTION_AND_UNSHIELD_SIZE + SHIELD_SIZE


extern "C" {

// idt数组，不仅包含中断例程的函数指针，还包含一些属性信息
gate_t idt[IDT_SIZE]; // 8*256 个bit 256
idtr_t idtr; // 48 个bit 6个地址

// 中断例程 后半部
handler_t handler_table[IDT_SIZE];

// idt实际储存的函数指针
extern handler_t handler_entry_table[NESSACERY_SIZE];

static void fill_idt_gate_info(gate_t *gate,
u32 handler,u8 selector,u8 reserved,u8 type,u8 segment,u8 DPL,u8 present);

}


using namespace diy_os;


void Interrupt_manager::interrupt_init(){
    this->regist_G_interrupt_manager();
    this->pic_init();
    this->idt_init();
    
}

void Interrupt_manager::regist_G_interrupt_manager(){
    this->irq_clock = 0;
    this->irq_keyboard = 1;
    this->irq_cascade = 2;
    this->irq_serial_2 = 3;
    this->irq_serial_1 = 4;
    this->irq_parallel_2 = 5;
    this->irq_floppy = 6;
    this->irq_parallel_1 = 7;
    this->irq_rtc = 8;
    this->irq_redirect = 9;
    this->irq_mouse = 12;
    this->irq_math = 13;
    this->irq_harddisk = 14;
    this->irq_harddisk2 = 15;


    this->irq_master_nr = 0x20;
    this->irq_slave_nr = 0x28;

    
    this->idtr_ptr = &idtr;
    this->idt_ptr = (gate_t*)idt;
    this->handler_table_ptr = (handler_t*)handler_table;


}

void Interrupt_manager::pic_init(){

    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 起始中断向量号 0x20
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始中断向量号 0x28
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断

}

void Interrupt_manager::idt_init()
{
    this->fill_exception_and_unshield_handler_default();
    this->fill_shield_handler_default();
    // fill_soft_handler_default();

    this->idtr_ptr->base = (u32)(this->idt_ptr);
    this->idtr_ptr->limit = sizeof(idt) - 1;
    asm volatile("lidt idtr\n");
}

void Interrupt_manager::fill_exception_and_unshield_handler_default(){
    for (size_t i = 0; i < EXCEPTION_AND_UNSHIELD_SIZE; i++)
    {
        /*
        gate->selector = 1 << 3; // 代码段
        gate->reserved = 0;      // 保留不用
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态
        gate->present = 1;       // 有效
        */
        fill_idt_gate_info(&this->idt_ptr[i],(u32)handler_entry_table[i],1<<3,0,0b1110,0,0,1);
    }

    for (size_t i = 0; i < EXCEPTION_AND_UNSHIELD_SIZE; i++)
    {
        this->handler_table_ptr[i] = (handler_t)exception_and_unshield_handler_default;
    }
}

void Interrupt_manager::fill_shield_handler_default(){
    for (size_t i = EXCEPTION_AND_UNSHIELD_SIZE; i < EXCEPTION_AND_UNSHIELD_SIZE+SHIELD_SIZE; i++)
    {
        /*
        gate->selector = 1 << 3; // 代码段
        gate->reserved = 0;      // 保留不用
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态
        gate->present = 1;       // 有效
        */
        fill_idt_gate_info(&this->idt_ptr[i],(u32)handler_entry_table[i],1<<3,0,0b1110,0,0,1);
    }

    for (size_t i = EXCEPTION_AND_UNSHIELD_SIZE; i < EXCEPTION_AND_UNSHIELD_SIZE+SHIELD_SIZE; i++)
    {
        this->handler_table_ptr[i] = (handler_t)shield_handler_default;
    }    
}

void Interrupt_manager::send_eoi(int vector){
    if (vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }
    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

void Interrupt_manager::set_shield_handler(u32 irq, handler_t handler){
    this->handler_table_ptr[this->irq_master_nr + irq] = handler;

}

void Interrupt_manager::set_shield_mask(u32 irq, bool enable)
{
    u16 port;
    if (irq < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
        irq -= 8;
    }
    if (enable)
    {
        outb(port, inb(port) & ~(1 << irq));
    }
    else
    {
        outb(port, inb(port) | (1 << irq));
    }
}


// 清除 IF 位(关中断的意思)，返回设置之前的值
bool Interrupt_manager::interrupt_disable()
{
    asm volatile(
        "pushfl\n"        // 将当前 eflags 压入栈中
        "cli\n"           // 清除 IF 位，此时外中断已被屏蔽
        "popl %eax\n"     // 将刚才压入的 eflags 弹出到 eax
        "shrl $9, %eax\n" // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n" // 只需要 IF 位
    );
}

// 获得 IF 位,检查是否是关中断什么的
bool Interrupt_manager::get_interrupt_state()
{
    asm volatile(
        "pushfl\n"        // 将当前 eflags 压入栈中
        "popl %eax\n"     // 将压入的 eflags 弹出到 eax
        "shrl $9, %eax\n" // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n" // 只需要 IF 位
    );
}

// 设置 IF 位
void Interrupt_manager::set_interrupt_state(bool state)
{
    if (state)
        asm volatile("sti\n");
    else
        asm volatile("cli\n");
}


void fill_idt_gate_info(gate_t *gate,
u32 handler,u8 selector,u8 reserved,u8 type,u8 segment,u8 DPL,u8 present){
    gate->offset0 = (u32)handler & 0xffff;
    gate->offset1 = ((u32)handler >> 16) & 0xffff;
    gate->selector = selector;
    gate->reserved = reserved;
    gate->type =type;
    gate->segment = segment;
    gate->DPL = DPL;
    gate->present = present;
}

Interrupt_manager G_interrupt_manager;

void exception_and_unshield_handler_default(int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags){
        
        BMB;

        G_test_interrupt_stack.vector = vector;
        G_test_interrupt_stack.edi = edi;
        G_test_interrupt_stack.esi = esi;
        G_test_interrupt_stack.ebp = ebp;
        G_test_interrupt_stack.esp = esp;
        G_test_interrupt_stack.ebx = ebx;
        G_test_interrupt_stack.edx = edx;
        G_test_interrupt_stack.ecx = ecx;
        G_test_interrupt_stack.eax = eax;
        G_test_interrupt_stack.gs = gs;
        G_test_interrupt_stack.fs = fs;
        G_test_interrupt_stack.es = es;
        G_test_interrupt_stack.ds = ds;
        G_test_interrupt_stack.vector0 = vector0;
        G_test_interrupt_stack.error = error;
        G_test_interrupt_stack.eip = eip;
        G_test_interrupt_stack.cs = cs;
        G_test_interrupt_stack.eflags = eflags;


        G_interrupt_manager.str_debug_buffer[0] = messages[vector];

    }

void shield_handler_default(int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags){
        
        BMB;

        G_interrupt_manager.send_eoi(vector);

        
        G_test_interrupt_stack.vector = vector;
        G_test_interrupt_stack.edi = edi;
        G_test_interrupt_stack.esi = esi;
        G_test_interrupt_stack.ebp = ebp;
        G_test_interrupt_stack.esp = esp;
        G_test_interrupt_stack.ebx = ebx;
        G_test_interrupt_stack.edx = edx;
        G_test_interrupt_stack.ecx = ecx;
        G_test_interrupt_stack.eax = eax;
        G_test_interrupt_stack.gs = gs;
        G_test_interrupt_stack.fs = fs;
        G_test_interrupt_stack.es = es;
        G_test_interrupt_stack.ds = ds;
        G_test_interrupt_stack.vector0 = vector0;
        G_test_interrupt_stack.error = error;
        G_test_interrupt_stack.eip = eip;
        G_test_interrupt_stack.cs = cs;
        G_test_interrupt_stack.eflags = eflags;


        G_interrupt_manager.str_debug_buffer[1] = messages[vector];

    }








