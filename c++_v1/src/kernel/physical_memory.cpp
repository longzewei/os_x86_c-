#include<types.h>
#include<physical_memory.h>
#include<string.h>
#include<stdlib.h>


extern "C" {

#define PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域
typedef struct ards_t
{
    u64 base; // 内存基地址
    u64 size; // 内存长度
    u32 type; // 类型
}__attribute__((__packed__)) ards_t;

#define IDX(addr) ((u32)addr >> 12)            // 获取 addr 的页索引
#define PAGE(idx) ((u32)idx << 12)             // 获取页索引 idx 对应的页开始的位置
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

}

using namespace diy_os;

void Physical_memory_manager::physical_memory_init(u32 magic, u32 addr){
    this->regist_G_physics_memory_manager();
    this->get_enough_info_2_fill_essential_part(magic,addr);

}

void Physical_memory_manager::regist_G_physics_memory_manager(){

}

void Physical_memory_manager::get_enough_info_2_fill_essential_part(u32 magic, u32 addr){
    this->page_size = PAGE_SIZE;
    this->get_raw_info_2fill_physics_memory_manager(magic,addr);
    this->raw_info_to_build_physics_memory_manager();
}

/*
只分配使用最大的一块连续内存,
填充属性信息
*/
void Physical_memory_manager::get_raw_info_2fill_physics_memory_manager(u32 magic, u32 addr){
    u32 count = 0;
    if (magic == ONIX_MAGIC)
    {
        count = *(u32 *)addr;
        ards_t *ptr = (ards_t *)(addr + 4);

        for (size_t i = 0; i < count; i++, ptr++)
        {
            if (ptr->type == ZONE_VALID && ptr->size > this->memory_size)
            {
                this->memory_base = (u32)ptr->base;
                this->memory_size = (u32)ptr->size;

            }

        }
    }

    this->total_pages = IDX(this->memory_size) + IDX(MEMORY_BASE);
    this->free_pages = IDX(this->memory_size);


}

/*
memmap的构建
*/
void Physical_memory_manager::raw_info_to_build_physics_memory_manager(){
    // 初始化物理内存数组
    this->memory_map = (u8 *)this->memory_base;

    // 计算物理内存数组占用的页数
    this->memory_map_pages = div_round_up(this->total_pages, PAGE_SIZE);


    this->free_pages -= this->memory_map_pages;

    // 清空物理内存数组
    memset((void *)this->memory_map, 0, this->memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 (前面杂七杂八的+内核代码不超过1M) 以及 物理内存数组已占用的页，已被占用
    this->start_page = IDX(MEMORY_BASE) + this->memory_map_pages;
    for (size_t i = 0; i < this->start_page; i++)
    {
        this->memory_map[i] = 1;
    }
}

/*
一页可以被255个线程占用,它们都是父子线程
*/
u32 Physical_memory_manager::get_page(){
    for (size_t i = this->start_page; i < this->total_pages; i++)
    {
        // 如果物理内存没有占用
        if (!this->memory_map[i])
        {
            this->memory_map[i] = 1;
            this->free_pages--;
            // assert(this->free_pages >= 0);
            u32 page = PAGE(i);
            {
                this->str_debug_buffer[0] = "GET page 0x%p\n";
                this->u32_debug_record[0] = (u32)page;
            }
            return page;
            
        }
    }

    this->str_debug_buffer[0] = "Out of Memory!!!";
    while (true)
    {
        /* code */
    }
    
    
}
void Physical_memory_manager::put_page(u32 addr){

    u32 idx = IDX(addr);

    // // idx 大于 1M 并且 小于 总页面数
    // assert(idx >= this->start_page && idx < this->total_pages);

    // // 保证只有一个引用
    // assert(this->memory_map[idx] >= 1);

    // 物理引用减一
    this->memory_map[idx]--;

    // 若为 0，则空闲页加一
    if (!this->memory_map[idx])
    {
        this->free_pages++;
    }

    // assert(this->free_pages > 0 && this->free_pages < this->total_pages);
    {
        this->str_debug_buffer[0] = "PUT page 0x%p\n";
        this->u32_debug_record[0] = (u32)addr;
    }
}


Physical_memory_manager G_physical_memory_manager;

