#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/spinlock_types.h>
#include <linux/workqueue.h>
#include <linux/slab.h>        /*kmalloc的头文件*/
#include <linux/kthread.h>
#include <linux/kallsyms.h>

#define THREADS 200

struct my_data{
    struct list_head list;
    int id;
    int pid;
};

static struct work_struct queue;
static struct timer_list mytimer;   /* 用于定时器队列 */
static LIST_HEAD(mine);  /* sharelist头 */
static unsigned int list_len = 0; 
static DEFINE_SEMAPHORE(sem);  /* 内核线程启动器之间进行同步的信号量,4.15内核适用*/
static DEFINE_SPINLOCK(my_lock); /* 保护对链表的操作,4.15内核适用 */
static atomic_t my_count = ATOMIC_INIT(0); /* 以原子方式进行追加 */
static int count = 0;

// 这个函数会交给内核线程来跑
static int insert_queue(void* data){
    struct my_data* p;
    if(count++%4 == 0) printk("\n");

    spin_lock(&my_lock);
    if(list_len < 50){
        if((p = kmalloc(sizeof(struct my_data), GFP_KERNEL)) == NULL){
            return -ENOMEM; /* Out of memory */
        }
		p->id = atomic_read(&my_count); /* 原子变量操作 */
		atomic_inc(&my_count);
		p->pid = current->pid;  // 运行这个工作队列项的是哪一个内核线程
		list_add(&p->list, &mine); /* 向队列中添加新字节 */
		list_len++;
		printk("THREAD ADD:%-5d\t", p->id);
    } else { // 队列长度超过50已经进行删除
		struct my_data *my = NULL;
		my = list_entry(mine.prev, struct my_data, list);
		list_del(mine.prev); /* 从队列尾部删除节点 */ // 这是一个循环双链表
		list_len--;
		printk("THREAD DEL:%-5d\t", my->id);
		kfree(my);
    }
    spin_unlock(&my_lock);
    return 0;
}

// 直接调度到工作队列中 工作队列的回调是创建一个内核线程
static void start_kthread(void){
    down(&sem); // 信号量减一，因为向工作对列中重复插入后续都是无效的
    schedule_work(&queue);  // 对这个工作队列进行调度
}

static void kthread_handler(struct work_struct* q){
    kthread_run(insert_queue, NULL, "%d", count);
    up(&sem);
}

void qt_task(struct timer_list *timer)
{
    spin_lock(&my_lock);
	if (!list_empty(&mine)) {
		struct my_data *i;
		if (count++ % 4 == 0)
			printk("\n");
		i = list_entry(mine.next, struct my_data, list); /* 取下一个节点 */
		list_del(mine.next); /* 删除节点 */
		list_len--;
		printk("TIMER DEL:%-5d\t", i->id);
		kfree(i);
	}
    spin_unlock(&my_lock);
    // msecs_to_jiffies可以把ms转化成时钟tick
	mod_timer(timer, jiffies + msecs_to_jiffies(1000));
    return;
}

static int shared_init(void){
    printk("share list enter!\n");
    int i;
    INIT_WORK(&queue, kthread_handler);
    timer_setup(&mytimer, qt_task, 0);
    add_timer(&mytimer);
    // 不能直接在循环中直接赋值
    // loop initial declarations are only allowed in C99 or C11 mode
    for(i = 0; i < THREADS; i++){
        start_kthread();    // 调度queue THREADS次
    }
    return 0;
}

static void shared_exit(void)
{
	struct list_head *n, *p = NULL;
	struct my_data *my = NULL;
	printk("\nshare list exit\n");
	del_timer(&mytimer);
	spin_lock(&my_lock); /* 上锁，以保护临界区 */
	list_for_each_safe(p, n, &mine)
        { /* 删除所有节点，销毁链表 */
		if (count++ % 4 == 0)
			printk("\n");
		my = list_entry(p, struct my_data, list); /* 取下一个节点 */
		list_del(p);
		printk("SYSCALL DEL: %d\t", my->id);
		kfree(my);
	}
    printk("Max count : %d\n", atomic_read(&my_count));
	spin_unlock(&my_lock); /* 开锁 */	
	printk(KERN_INFO"Over \n");
}

module_init(shared_init);
module_exit(shared_exit);
