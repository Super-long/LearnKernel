#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/init_task.h>

static int __init print_pcb(void){
    struct task_struct *task, *p;
    struct list_head *pos;

    // 打印目前系统中的进程数和每一个进程的一些数据

    int counts = 0; // 系统进程一共有多少个
    printk("begin:\n");

    task = &init_task;  // 零号进程的PCB

    list_for_each(pos,&task->tasks)
    {    // tasks把所有的进程连接起来
        p = list_entry(pos, struct task_struct, tasks); // 指向每个task_struct结构体
        counts++;
        printk("\n\n");
        printk("pid:%d; state:%lx; prio:%d; static_prio:%d; parent'pid:%d \n", p->pid, p->state, p->prio, p->static_prio, (p->parent)->pid);
        // 这里再打印进程的地址信息 因为内核线程这一项是NULL 所以我们需要进程判断
        if((p->mm)!=NULL)   printk("total_vm:%ld", (p->mm)->total_vm);
    }

    printk("进程的个数 ： %d\n", counts);

    return 0;
}

static void __exit exit_pcb(void){
    printk("exiting...\n");
}

module_init(print_pcb); 
module_exit(exit_pcb);