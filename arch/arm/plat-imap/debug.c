

#include <linux/kernel.h>


void print_mem(ulong level,void *data,ulong len,char * tag)
{
    ulong count;
    if(level>0){
        if(tag){
            printk("%s\n",tag);
        }
        if(!data)
            return;
            
        for(count=0;count<len;count++){
            if(len>32){
                if(count>16 && count<len-16){
                    printk("......");
                    count=len-16;
                    continue;
                }    
            }
        
            printk("%02x ",((char *)data)[count]);
            if(!((count+1)%32))
                printk("\n");
        }
        if(count%32)
            printk("\n");
    }
}

