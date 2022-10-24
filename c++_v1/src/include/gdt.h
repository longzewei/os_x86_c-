#ifndef Gdt_h
#define Gdt_h

#include<types.h>


#define Gdt_table_size 128

#define LIMIT_HIGH(base_ptr)  ((base_ptr>>16) & 0xf)
#define LIMIT_LOW(base_ptr)  ((base_ptr) & 0xffff)

#define BASE_HIGH(base_ptr)  ((base_ptr>>24) & 0xff)
#define BASE_LOW(base_ptr)  ((base_ptr) & 0xffffff)


extern "C" {

typedef struct gdt_descriptor_t
{
    u16 limit_low;      // 段界限 0 ~ 15 位
    u32 base_low : 24;    // 基地址 0 ~ 23 位 16M
    u8 type : 4;        // 段类型
    u8 segment : 1;     // 1 表示代码段或数据段，0 表示系统段
    u8 DPL : 2;         // Descriptor Privilege Level 描述符特权等级 0 ~ 3
    u8 present : 1;     // 存在位，1 在内存中，0 在磁盘上
    u8 limit_high : 4;  // 段界限 16 ~ 19;
    u8 available : 1;   // 该安排的都安排了，送给操作系统吧
    u8 long_mode : 1;   // 64 位扩展标志
    u8 big : 1;         // 32 位 还是 16 位;
    u8 granularity : 1; // 粒度 4KB 或 1B
    u8 base_high;       // 基地址 24 ~ 31 位

}__attribute__((__packed__)) gdt_descriptor_t;

typedef struct tss_t
{
    u32 backlink; // 前一个任务的链接，保存了前一个任状态段的段选择子
    u32 esp0;     // ring0 的栈顶地址
    u32 ss0;      // ring0 的栈段选择子
    u32 esp1;     // ring1 的栈顶地址
    u32 ss1;      // ring1 的栈段选择子
    u32 esp2;     // ring2 的栈顶地址
    u32 ss2;      // ring2 的栈段选择子
    u32 cr3;
    u32 eip;
    u32 flags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldtr;          // 局部描述符选择子
    u16 trace : 1;     // 如果置位，任务切换时将引发一个调试异常
    u16 reversed : 15; // 保留不用
    u16 iobase;        // I/O 位图基地址，16 位从 TSS 到 IO 权限位图的偏移
    u32 ssp;           // 任务影子栈指针
}__attribute__((__packed__)) tss_t;

// 段选择子
typedef struct gdt_selector_t
{
    u8 RPL : 2; // Request Privilege Level
    u8 TI : 1;  // Table Indicator
    u16 index : 13;
}__attribute__((__packed__)) gdt_selector_t;

typedef struct gdtr_t
{
    u16 limit;
    u32 base;
}__attribute__((__packed__)) gdtr_t;

}

namespace diy_os {



class Gdt_manager{

private:
    gdtr_t *gdtr_ptr;
    gdt_descriptor_t *gdt_table_ptr;

private:
    u32 kernel_code_idx;
    u32 kernel_data_idx;

    u32 kernel_tss_idx;

    u32 user_code_idx;
    u32 user_data_idx;

    u32 kernel_code_selector;
    u32 kernel_data_selector;
    u32 kernel_tss_selector;
    u32 user_code_selector;
    u32 user_data_selector;
    char* str_debug_buffer[3];
    u32 u32_debug_record[3];


public:
    Gdt_manager();           //构造函数
    ~Gdt_manager(); 
    void start();


private:
    void regist_G_gdt_manager();
    void fill_gdt_table();
    void descriptor_init(
    gdt_descriptor_t *desc,
    u32 limit,
    u32 base,
    u8 type,
    u8 segment,
    u8 DPL,
    u8 present,
    u8 available,
    u8 long_mode,
    u8 big,
    u8 granularity);
};


}

#endif