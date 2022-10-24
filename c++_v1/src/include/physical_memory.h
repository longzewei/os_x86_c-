#ifndef Physical_memory_h
#define Physical_memory_h

#include<types.h>



namespace diy_os {

class Physical_memory_manager {

private:
    u32 page_size;   // 4K
    u32 memory_base; // 可用内存基地址，应该等于 1M
    u32 memory_size; // 可用内存大小
    u32 total_pages; // 所有内存页数
    u32 free_pages;  // 空闲内存页数

    u32 start_page;   // 可分配物理内存起始位置
    u8 *memory_map;       // 物理内存数组
    u32 memory_map_pages; // 物理内存数组占用的页数

    char* str_debug_buffer[3];
    u32 u32_debug_record[3];

public:
    // 拿到信息
    // 填充内容
    void physical_memory_init(u32 magic, u32 addr);
    
    // 分配一页物理内存
    u32 get_page(); // return 一个u32,分配page的头地址
    // 释放一页物理内存
    void put_page(u32 addr);

private:
    void regist_G_physics_memory_manager();
    void get_enough_info_2_fill_essential_part(u32 magic, u32 addr);

    void get_raw_info_2fill_physics_memory_manager(u32 magic, u32 addr);
    void raw_info_to_build_physics_memory_manager();

};

}


#endif