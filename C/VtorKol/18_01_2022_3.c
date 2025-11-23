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

char mem[6*1024];
char dop[4*1024];

int r, w, flag;

static int r_init(void){
    w = r = flag =0;
    register_chrdev(60,"primer1",&myfops);
    return 0;
}

ssize_t read(file *f,char *buff,size_t count,...){
    if(w-r>count){
        copy_to_user(buff,mem+r,count);
        r+=count;
        return count;
    }else{
        int x = w - r;
        copy_to_user(buff,mem+r,x);
        r+=x;
        return x;
    }
}

ssize_t write(file *f,char *buff,size_t count,...){

}