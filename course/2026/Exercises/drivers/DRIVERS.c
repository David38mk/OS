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
char mem[512];

static int r_init(void){
    w = flag = r = 0;
    register_chrdev(60,"primer1",&myfops);
    return 0;
}

ssize_t read(struct file *f,char *buff,size_t count,loff_t *ptr){
    if(flag==1){
        if(512 - (512 - r)>=count){
            r-=count;
            copy_to_user(buff, mem+r,count);
            return count;
        }else{
            int x = 512 - (512 - r);
            r = 0;
            w = 0;
            flag = 0;
            copy_to_user(buff, mem, x);
            return x;
        }
    }else{
        return 0;
    }
}


ssize_t write(struct file *f,char *buff,size_t count,loff_t *ptr){
    if(flag == 0){
        if(512 - w >= count){
            copy_from_user(mem+w,buff,count);
            w+=count;
            return count;
        }else{
            int x = 512 - w;
            copy_from_user(mem+w,buff,x);
            w = 512;
            r = 0;
            //r = 512
            flag = 1;
            return x;
        }
    }else{
        return 0;
    }
}