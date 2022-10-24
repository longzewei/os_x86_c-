#ifndef Interrupt_h
#define Interrupt_h

#include<types.h>

#define IDT_SIZE 256

/*
开放注册方法,
各自局部注册
*/
typedef void *handler_t; // 中断处理函数,定义了一个函数指针类型

extern "C" {

// 全局描述符表指针
typedef struct idtr_t
{
    u16 limit;
    u32 base;
}__attribute__((__packed__)) idtr_t;

typedef struct gate_t
{
    u16 offset0;    // 段内偏移 0 ~ 15 位
    u16 selector;   // 代码段选择子
    u8 reserved;    // 保留不用
    u8 type : 4;    // 任务门/中断门/陷阱门
    u8 segment : 1; // segment = 0 表示系统段
    u8 DPL : 2;     // 使用 int 指令访问的最低权限
    u8 present : 1; // 是否有效
    u16 offset1;    // 段内偏移 16 ~ 31 位
}__attribute__((__packed__)) gate_t;

}


namespace diy_os {

class Interrupt_manager{

private:
    idtr_t *idtr_ptr;
    gate_t *idt_ptr;
    handler_t *handler_table_ptr;
    
public:
    u8 irq_clock;
    u8 irq_keyboard;
    u8 irq_cascade;
    u8 irq_serial_2;
    u8 irq_serial_1;
    u8 irq_parallel_2;
    u8 irq_floppy;
    u8 irq_parallel_1;
    u8 irq_rtc;
    u8 irq_redirect;
    u8 irq_mouse;
    u8 irq_math;
    u8 irq_harddisk;
    u8 irq_harddisk2;

    u8 irq_master_nr;
    u8 irq_slave_nr;

    char* str_debug_buffer[3];
    u32 u32_debug_record[3];

private:
    void regist_G_interrupt_manager();
    void pic_init();
    void idt_init();
public:
    void interrupt_init();

    // 设置中断处理函数
    void set_shield_handler(u32 irq, handler_t handler);
    void set_shield_mask(u32 irq, bool enable);

    bool interrupt_disable();             // 清除 IF 位，返回设置之前的值
    bool get_interrupt_state();           // 获得 IF 位
    void set_interrupt_state(bool state); // 设置 IF 位

    // 通知中断控制器，中断处理结束, 定制的 io中断函数 需要这个
    void send_eoi(int vector);


private:
    void fill_exception_and_unshield_handler_default();
    void fill_shield_handler_default();
    // void fill_soft_handler_default();

};


}

#endif