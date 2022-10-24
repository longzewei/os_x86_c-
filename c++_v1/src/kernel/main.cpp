#include<types.h>
#include<gdt.h>
#include<interrupt.h>
#include<physical_memory.h>
#include<task.h>


extern "C" {

    void kernel_init(u32 magic,u32 ards_count_ptr);
}

extern "C" {
void clock_init();

}
using namespace diy_os;

extern Gdt_manager G_gdt_manager;
extern Interrupt_manager G_interrupt_manager;
extern Physical_memory_manager G_physical_memory_manager;
extern Task_manager G_task_manager;

void kernel_init(u32 magic,u32 ards_count_ptr){
    char *t = (char *)0xB8006;
    *t = '2';

    G_gdt_manager.start();
    G_interrupt_manager.interrupt_init();
    G_physical_memory_manager.physical_memory_init(magic,ards_count_ptr);

    {
        for (size_t i = 0; i < 100; i++)
        {
            G_physical_memory_manager.put_page(G_physical_memory_manager.get_page());
        }
        
    }
    G_task_manager.task_init();
    
    clock_init();

    G_interrupt_manager.set_interrupt_state(true);

    char *temp = (char *)0xB8000;
    u8 counter = 0;
    while(true){
        *temp = counter++;
    }
}