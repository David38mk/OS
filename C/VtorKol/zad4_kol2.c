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

char memorija[512];
int z,r,w;  // z-> 1 locked, z-> 0 unlcoked
char shifra[7];

static ssize_t read(struct file *filep, char *buff, size_t count, loff_t *offp){
    int k;
    if(r+count<=w){
        k = count;
    }else{
        k = w-r;
    }
    if(z==1){
        char a[k];
        shifiraj(shifra,memorija+r,a);
        copy_to_user(buff,a,k); //???
        return k;
    }else{  
        copy_to_user(buff,memorija+r,k);
        return k;
    }
}

static ssize_t write(struct file *filep, char *buff, size_t count, loff_t *offp){
    if(z==1){
        char pom[count];
        copy_from_user(pom,buff,count);
        if(strcmp(pom,sifra)==0){
            z=0;
        }
        return 0;
    }else{
        int k = 512 - w;
        if(count<k){
            copy_from_user(memorija+w,buff,count);
            w+=count;
            z=1;
            return count;
        }else{
            copy_from_user(memorija+w,buff,k);
            w+=k;
            z=1;
            return k;
        }
        
    }
}

int main(){
    return 0;
}