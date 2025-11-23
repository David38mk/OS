#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <sys/wait.h>

int r, w, flag;
char mem[4*1024];

static int r_init(void){
    w = flag = r = 0;
    register_chrdev(60,"primer1",&myfops);
    return 0;
}

ssize_t read(struct file *f,char *buff,size_t count,loff_t *ptr){
    if(r+flag <= 4096){
        copy_to_user(buff,mem+r, flag);
        r += flag;
        return flag;
        
    }else{
        copy_to_user(buff,mem+r, 4096-r);
        r = 0;
        return 4096 - r;
    }
}


ssize_t write(struct file *f,char *buff,size_t count,loff_t *ptr){
    if(count < 4* 1024){
        int sporedba = 64;
        for(int i=0;i<64;i++){
            if(count < sporedba){
                flag = sporedba;
                copy_from_user(mem, buff, count);
                return count;
            }else{
                sporedba+=64;
            }
        }
    }else{
        flag = 4 * 1024;
        copy_from_user(mem, buff, flag);
        return flag;
    }
    
    
}