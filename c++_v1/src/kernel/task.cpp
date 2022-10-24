#include<types.h>
#include<task.h>
#include<string.h>



extern "C" {

extern void kernel_thread_init();
extern void task_switch(task_t *next);
}


using namespace diy_os;
// FIXME
#include<physical_memory.h>
extern Physical_memory_manager G_physical_memory_manager;

void Task_manager::task_init(){

    this->regist_G_task_manager();
    this->task_setup();

    // FIXME
    kernel_thread_init();


}

void Task_manager::regist_G_task_manager(){
    this->indicator_for_search = 0;
    list_init(&this->block_list);
}

/*
面向返回编程
*/
void Task_manager::task_setup()
{
    task_t *task = this->running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;
    // // 初始的赋予一个内核页表内容
    // memcpy(&task->virtual_memory_manager,&G_kernel_virtual_memory_manager,sizeof(virtual_memory_manager_t));

    memset(this->task_table, 0, sizeof(this->task_table));
}

// 从 task_table 里获得一个空闲的任务
task_t *Task_manager::get_free_task()
{
    task_t *running_task = this->running_task();
    // virtual_memory_manager_t *virtual_memory_manager = &running_task->virtual_memory_manager;
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        if (this->task_table[i] == NULL)
        {
            // FIXME 
            // task_t *task = (task_t *)virtual_memory_manager->alloc_kpage(1,virtual_memory_manager);
            task_t *task = (task_t *)G_physical_memory_manager.get_page();
            memset(task, 0, PAGE_SIZE);
            this->task_table[i] = task;
            return task;
        }
    }
    
    
    {
        this->str_debug_buffer[0] = "No more tasks";
    }
    while (true)
    {
        /* code */
    }
    
}

task_t *Task_manager::task_create(target_t target, const char *name, u32 priority, u32 uid)
{
    task_t *task = this->get_free_task();

    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;

    frame->eip = (void (*)())(target);

    strcpy((char *)task->name, name);

    task->stack = (u32 *)stack;
    task->priority = priority;
    task->ticks = task->priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    task->uid = uid;
    // // 初始的赋予一个内核页表内容
    // memcpy(&task->virtual_memory_manager,&G_kernel_virtual_memory_manager,sizeof(virtual_memory_manager_t));
    task->magic = ONIX_MAGIC;

    return task;
}



void Task_manager::schedule()
{
    // assert(!G_interrupt_manager.get_interrupt_state()); // 不可中断

    task_t *current = this->running_task();
    task_t *next = this->task_search(TASK_READY);
    
    // assert(next != NULL);
    // assert(next->magic == ONIX_MAGIC);

    if (current->state == TASK_RUNNING)
    {
        current->state = TASK_READY;
    }

    if (!current->ticks)
    {
        current->ticks = current->priority;
    }

    next->state = TASK_RUNNING;
    if (next == current)
        return;

    // task_activate(next);
    task_switch(next);
}


// 从任务数组中查找某种状态的任务，自己除外(idle也能找自己)
task_t *Task_manager::task_search(task_state_t state)
{
    // assert(!G_interrupt_manager.get_interrupt_state());
    task_t *task = (task_t *)NULL;
    task_t *current = this->running_task();

    u32 index = 0;
    for (size_t i = 0; i < NR_TASKS; i++)
    {
        index = (i+this->indicator_for_search) % NR_TASKS;
        task_t *ptr = this->task_table[index];
        if (ptr == NULL)
            continue;

        if (ptr->state != state)
            continue;
        if (current == ptr)
            continue;
        if (task == NULL || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
            task = ptr;
    }

    if(task == NULL && state == TASK_READY){
        // FIXME 0号线程一定是idle task
        task = this->task_table[0];
    }

    if(task != NULL){
        this->indicator_for_search++;
    }

    return task;
}





/*一生下来栈esp就再也不会碰到栈底*/
task_t *Task_manager::running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}


Task_manager G_task_manager;