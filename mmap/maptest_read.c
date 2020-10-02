#include <stdio.h>  
#include <unistd.h>  
#include <sys/mman.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#define LEN (10*4096)  // 内核模块中定义的
int main(void)  
{  
    int fd,loop;  
    char *vadr;  
  
    if ((fd = open("/dev/mapnopage", O_RDWR)) < 0) {  
        return 0;  
    }  
    // 映射区地址/映射区长度/期望的内存保护标志 PROT_READ页内容可以被读取
    // MAP_PRIVATE 写入时拷贝的私有映射区
    // MAP_LOCKED 锁定页面 防止被换出内存
    // 最后一位表示被映射的对象从哪里开始 从哪一个文件偏移量开始
    vadr = mmap(0, LEN, PROT_READ, MAP_PRIVATE | MAP_LOCKED, fd, 0); 
    // 读取了此映射区中两页的数据 证明此时应该触发两次缺页中断      
    for(loop=0;loop<2;loop++){
        // 打印页框的内容与地址
        printf("[%-10s----%lx]\n",vadr+4096*loop,vadr+4096*loop);
    }
    while(1)
    {
        sleep(1);
    }
     
}  