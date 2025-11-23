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

int w,r;
char memorija[1024];

static ssize_t read(struct file *filep, char *buff, size_t count, loff_t *offp){
    if(r+count<w){
        copy_to_user(buff, memorija+r, count);
        r+=count;
        return count;
    }else{
        int k = w-r;
        copy_to_user(buff, memorija+r, k);
        return k;
    }
}

static ssize_t write(struct file *filep, char *buff, size_t count, loff_t *offp){
    int MAX = (1024 - w)/2;
    if(count > MAX){
        copy_from_user(memorija+w, buff, MAX);
        w+=MAX;
        return MAX;
    }else{
        copy_from_user(memorija+w, buff, count);
        w+=count;
        return count; 
    }
}

int main(){
    
    return 0;
}