#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>


static int __init lk_maxnum(void)
{
        int x = 1, y = 2;
        printk("max=%d\n", max(x++, y++));
        printk("x = %d, y = %d\n", x, y);
        return 0;
}

static void __exit lk_exit(void)
{
        printk("The maxnum moudle has exited!\n");
}

module_init(lk_maxnum); //内核入口点，调用初始化函数，包含在module.h中
module_exit(lk_exit); //出口点

//e1000
//insmod e1000_driver