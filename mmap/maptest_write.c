#include <stdio.h>  
#include <unistd.h>  
#include <sys/mman.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#define LEN (10*4096)  
int main(void) 
{  
    int fd;
    char *vadr;  
  
    if ((fd = open("/dev/mapnopage", O_RDWR)) < 0) {  
        return 0;  
    }  
    // MAP_SHARED该映射区与其他映射区共享空间 
    // 因为我们需要写入 如果使用的是私有的话 写入只是在进程内部可见不被其他进程可见
    vadr = mmap(0, LEN, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);  
    
    sprintf(vadr, "write from userspace");
    
    while(1)
    {
       sleep(1);
    }     
    return 0;
}  