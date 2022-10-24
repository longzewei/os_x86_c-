#include<io.h>
#include<interrupt.h>
#include<types.h>
#include<task.h>

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

u32 volatile jiffies = 0;
u32 jiffy = JIFFY;

using namespace diy_os;

extern Interrupt_manager G_interrupt_manager;
extern Task_manager G_task_manager;

void clock_handler(int vector)
{
    G_interrupt_manager.send_eoi(vector);
    task_t *task = G_task_manager.running_task();
    // assert(task->magic == ONIX_MAGIC);
    task->jiffies = jiffies;
    task->ticks--;
    if(!task->ticks){
        task->ticks = task->priority;
        G_task_manager.schedule();
    }
}

time_t sys_time()
{
    return (jiffies * JIFFY) / 1000;
}

void pit_init()
{
    // 配置计数器 0 时钟
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

}

// 告诉编译器
// 不要对 定义的 函数进行name mangling
extern "C" {

void clock_init()
{
    pit_init();
    G_interrupt_manager.set_shield_handler(G_interrupt_manager.irq_clock, (handler_t)clock_handler);
    G_interrupt_manager.set_shield_mask(G_interrupt_manager.irq_clock, true);
}

}
