#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

static int irq;         // irq号
static char* devname;   // 设备名称

// 用于在命令行中传入参数
module_param(irq, int, 0644);
module_param(devname, charp, 0644); // charp相当于char*

struct myirq{   // 用于共享中断线的时候 用于区分同一个中断线上的不同中断处理程序
    int devid;
};

struct myirq mydev={1119};

static irqreturn_t myirq_handler(int irq, void* dev){
    struct myirq mydev;
    static int count = 1;
    mydev = *(struct myirq*)dev;
    printk("key: %d..\n", count++); // 进入中断一次count加一
    printk("devid: %d ISR is working..\n", mydev.devid);
    return IRQ_HANDLED;
}

static int __init myirq_init(void){
    printk("Module is worling...\n");
    // 这个irq对应的是中断控制器上irq的编号
    if(request_irq(irq, myirq_handler, IRQF_SHARED, devname, &mydev) != 0){
        printk("%s request IRQ: %d failed..\n", devname, irq);
        return -1;
    }
    printk("%s request IRQ: %d successful..\n", devname, irq);
    return 0;
}

static void __exit myirq_exit(void){
    printk("module is leaving...\n");
    free_irq(irq, &mydev);
    printk("Free the irq: %d..\n", irq);
}

module_init(myirq_init); //内核入口点，调用初始化函数，包含在module.h中
module_exit(myirq_exit); //出口点