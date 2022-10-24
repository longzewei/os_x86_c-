#include<task.h>
#include<string.h>
#include<types.h>
#include<interrupt.h>

using namespace diy_os;

extern Interrupt_manager G_interrupt_manager;
extern Task_manager G_task_manager;

static void idle(){
    G_interrupt_manager.set_interrupt_state(true);
    while (true)
    {
        asm volatile(
            "sti\n"
            "hlt\n"
        );
        G_task_manager.u32_debug_record[1]++;
    }
    
}

static void idle2(){
    G_interrupt_manager.set_interrupt_state(true);
    while (true)
    {
        G_task_manager.u32_debug_record[1]++;
    }
    
}


static void idle3(){
    G_interrupt_manager.set_interrupt_state(true);
    while (true)
    {
        // asm volatile(
        //     "sti\n"
        //     "hlt\n"
        // );
        G_task_manager.u32_debug_record[2]++;
    }
    
}

// static void test_user_mode(){
//     // 里面中断返回,iret会开中断
//     G_task_manager.task_to_user_mode(idle3);
    
// }

extern "C" {

void kernel_thread_init(){
    G_task_manager.task_create(idle, "idle", 1, KERNEL_USER);
    G_task_manager.task_create(idle2, "idle2", 1, KERNEL_USER);
    G_task_manager.task_create(idle3, "test_user_mode", 2, NORMAL_USER);
    
}

}

