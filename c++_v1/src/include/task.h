#ifndef Task_h
#define Task_h

#include<types.h>
#include<list.h>


extern "C" {

typedef void target_t();

#define KERNEL_USER 0
#define NORMAL_USER 1

#define TASK_NAME_LEN 16

#define NR_TASKS 64

#define PAGE_SIZE 0x1000     // 一页的大小 4K

typedef enum task_state_t
{
    TASK_INIT,     // 初始化
    TASK_RUNNING,  // 执行
    TASK_READY,    // 就绪
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠
    TASK_WAITING,  // 等待
    TASK_DIED,     // 死亡
} task_state_t;


/*
内核线程
*/
typedef struct task_t
{
    u32 *stack;               // 内核栈
    list_node_t node;         // 任务阻塞节点
    task_state_t state;       // 任务状态
    u32 priority;             // 任务优先级
    int ticks;                // 剩余时间片
    u32 jiffies;              // 上次执行时全局时间片
    char name[TASK_NAME_LEN]; // 任务名
    u32 uid;                  // 用户 id
    // u32 pde;                  // 页目录物理地址
    // struct bitmap_t *vmap;    // 进程虚拟内存位图
    // virtual_memory_manager_t virtual_memory_manager;
    int status;               // 进程特殊状态
    u32 magic;                // 内核魔数，用于检测栈溢出
} task_t;

/*
面向返回编程：
    本质上是构造 接收者 期待的结果，去返回
    */
typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    void (*eip)(void);
} task_frame_t;


}


namespace diy_os {

class Task_manager{

private:
    // 具体的内容放在内核栈里
    task_t *task_table[NR_TASKS]; // 任务表

    list_t block_list;

    

public:
    void task_init();

    task_t *running_task();
    void schedule();

    task_t *task_create(target_t target, const char *name, u32 priority, u32 uid);


    char* str_debug_buffer[3];
    u32 u32_debug_record[3];

private:
    void regist_G_task_manager();
    /*task概念的诞生,前面只有一个"任务",也可以认为没有任务,因为不需要区别*/
    void task_setup();

    task_t *get_free_task();
    task_t *task_search(task_state_t state);

    // void task_block(task_t *task, list_t *blist, task_state_t state);
    // void task_unblock(task_t *task);
    
    u32 indicator_for_search;

};

}


#endif