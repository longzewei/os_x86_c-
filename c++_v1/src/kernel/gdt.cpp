#include<gdt.h>
#include<types.h>

extern "C" {
    gdtr_t gdtr;
    gdt_descriptor_t gdt_table;
}

using namespace diy_os;

Gdt_manager::Gdt_manager(){
    
}
Gdt_manager::~Gdt_manager(){

}
void Gdt_manager::start(){
    Gdt_manager::regist_G_gdt_manager();
    Gdt_manager::fill_gdt_table();

    this->gdtr_ptr->base = (u32)(this->gdt_table_ptr);
    this->gdtr_ptr->limit = Gdt_table_size - 1;
    BMB;
    asm volatile("lgdt gdtr\n");
    BMB;
}

void Gdt_manager::regist_G_gdt_manager(){
    this->kernel_code_idx = 1;
    this->kernel_data_idx = 2;

    this->kernel_tss_idx = 3;

    this->user_code_idx = 4;
    this->user_data_idx = 5;

    this->kernel_code_selector = this->kernel_code_idx<<3;
    this->kernel_data_selector = this->kernel_data_idx<<3;

    this->kernel_tss_selector = this->kernel_tss_idx<<3;

    this->user_code_selector = this->user_code_idx<<3;
    this->user_data_selector = this->user_code_idx<<3;

    this->gdtr_ptr = &gdtr;
    this->gdt_table_ptr = &gdt_table;


}

void Gdt_manager::fill_gdt_table(){
    this->descriptor_init((gdt_descriptor_t *)(this->gdt_table_ptr) + this->kernel_code_idx,0xfffff,0,0b1010,1,0,1,0,0,1,1);

    // type = 0b0010;   // 数据 / 向上增长 / 可写 / 没有被访问过
    this->descriptor_init((gdt_descriptor_t *)(this->gdt_table_ptr) + this->kernel_data_idx,0xfffff,0,0b0010,1,0,1,0,0,1,1);

}

void Gdt_manager::descriptor_init(
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
    u8 granularity){

    desc->limit_low = LIMIT_LOW(limit);
    desc->base_low = BASE_LOW(base);
    desc->type = type;
    desc->segment = segment;
    desc->DPL = DPL;
    desc->present = present;
    desc->limit_high = LIMIT_HIGH(limit);
    desc->available = available;
    desc->long_mode = long_mode;
    desc->big = big;
    desc->granularity = granularity;
    desc->base_high = BASE_HIGH(base);

    }

Gdt_manager G_gdt_manager;
