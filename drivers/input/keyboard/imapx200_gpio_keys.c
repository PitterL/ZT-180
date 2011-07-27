/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <asm/io.h>
#include <asm-generic/bitops/non-atomic.h>
#include <asm/gpio.h>
#include <mach/imapx_gpio.h>
#include <mach/irqs.h>
#include <mach/imapx_intr.h>
#include <mach/imapx_sysmgr.h>
#include <linux/syscalls.h>

//#define DEBUG
#if defined(DEBUG)
#define DBG printk
#else
#define DBG(x...)
#endif

#define POWER_CODE  116
#define HOME_CODE   102
#define MENU_CODE   59
#define BACK_CODE   158
#define VOL0_CODE   114
#define VOL1_CODE   115
#define LOCK_CODE   400
#define UNLOCK_CODE   401


#if defined(CONFIG_BOARD_B0)
    #define BACK_EINT_GRP       rEINTG1PEND
    #define BACK_EINT_MSK       rEINTG1MASK
    #define BACK_EINT_SHIFT     3
    #define BACK_KEY            IMAPX200_GPA(3)
    
    #define MENU_EINT_GRP       rEINTG6PEND
    #define MENU_EINT_MSK       rEINTG6MASK
    #define MENU_EINT_SHIFT     29
    #define MENU_KEY            IMAPX200_GPL(2)

    #define HOME_EINT_GRP       rEINTG5PEND
    #define HOME_EINT_MSK       rEINTG5MASK
    #define HOME_EINT_SHIFT     26
    #define HOME_KEY            IMAPX200_GPL(7)
    
    #define K_HDMI              K_MENU
#endif


#if (defined(CONFIG_BOARD_E2)||defined(CONFIG_BOARD_E3)||    \
        defined(CONFIG_BOARD_G0)||defined(CONFIG_BOARD_G0_3G)|| \
        defined(CONFIG_BOARD_I0)||defined(CONFIG_BOARD_J0))
    #define BACK_EINT_GRP       rEINTG1PEND
    #define BACK_EINT_MSK       rEINTG1MASK
    #define BACK_EINT_SHIFT     3
    #define BACK_KEY            IMAPX200_GPA(3)
    
    #define MENU_EINT_GRP       rEINTG2PEND
    #define MENU_EINT_MSK       rEINTG2MASK
    #define MENU_EINT_SHIFT     2
    #define MENU_KEY            IMAPX200_GPB(2)
    
    #define HOME_EINT_GRP       rEINTG2PEND
    #define HOME_EINT_MSK       rEINTG2MASK
    #define HOME_EINT_SHIFT     1
    #define HOME_KEY            IMAPX200_GPB(1)

    #define K_HDMI              K_MENU
#endif

#if defined(CONFIG_BOARD_E4)
    #define BACK_EINT_GRP       rEINTG1PEND
    #define BACK_EINT_MSK       rEINTG1MASK
    #define BACK_EINT_SHIFT     3
    #define BACK_KEY            IMAPX200_GPA(3)

    #define MENU_EINT_GRP       rEINTG2PEND
    #define MENU_EINT_MSK       rEINTG2MASK
    #define MENU_EINT_SHIFT     1
    #define MENU_KEY            IMAPX200_GPB(1)

    
    #define HOME_EINT_GRP       rEINTG2PEND
    #define HOME_EINT_MSK       rEINTG2MASK
    #define HOME_EINT_SHIFT     2
    #define HOME_KEY            IMAPX200_GPB(2)
    
    #define K_HDMI              K_MENU
#endif

#if defined(CONFIG_BOARD_E5)
    #define VIRTUAL_BACK_X0         3120
    #define VIRTUAL_BACK_X1         4095
    #define VIRTUAL_BACK_Y0         600
    #define VIRTUAL_BACK_Y1         1000

    #define VIRTUAL_MENU_X0         3120
    #define VIRTUAL_MENU_X1         4095
    #define VIRTUAL_MENU_Y0         2730
    #define VIRTUAL_MENU_Y1         3140

    #define VIRTUAL_HOME_X0         3120
    #define VIRTUAL_HOME_X1         4095
    #define VIRTUAL_HOME_Y0         1720
    #define VIRTUAL_HOME_Y1         2080

    #define VOL0_EINT_GRP       rEINTG2PEND
    #define VOL0_EINT_MSK       rEINTG2MASK
    #define VOL0_EINT_SHIFT     1
    #define VOL0_KEY            IMAPX200_GPB(1)
    
    #define VOL1_EINT_GRP       rEINTG2PEND
    #define VOL1_EINT_MSK       rEINTG2MASK
    #define VOL1_EINT_SHIFT     2
    #define VOL1_KEY            IMAPX200_GPB(2)

     #define K_HDMI              V_MENU + K_MAX
#endif

#if defined(CONFIG_BOARD_F0)
    #define BACK_EINT_GRP       rEINTG2PEND
    #define BACK_EINT_MSK       rEINTG2MASK
    #define BACK_EINT_SHIFT     2
    #define BACK_KEY            IMAPX200_GPB(2)

    #define MENU_EINT_GRP       rEINTG2PEND
    #define MENU_EINT_MSK       rEINTG2MASK
    #define MENU_EINT_SHIFT     1
    #define MENU_KEY            IMAPX200_GPB(1)

    #define VOL0_EINT_GRP       rEINTG4PEND
    #define VOL0_EINT_MSK       rEINTG4MASK
    #define VOL0_EINT_SHIFT     7
    #define VOL0_KEY            IMAPX200_GPE(7)

    #define VOL1_EINT_GRP       rEINTG1PEND
    #define VOL1_EINT_MSK       rEINTG1MASK
    #define VOL1_EINT_SHIFT     3
    #define VOL1_KEY            IMAPX200_GPA(3)
#endif

#if defined(CONFIG_BOARD_H0)
    #define BACK_EINT_GRP       rEINTG1PEND
    #define BACK_EINT_MSK       rEINTG1MASK
    #define BACK_EINT_SHIFT     3
    #define BACK_KEY            IMAPX200_GPA(3)

    #define MENU_EINT_GRP       rEINTG1PEND
    #define MENU_EINT_MSK       rEINTG1MASK
    #define MENU_EINT_SHIFT     2
    #define MENU_KEY            IMAPX200_GPA(2)

    #define HOME_EINT_GRP       rEINTG2PEND
    #define HOME_EINT_MSK       rEINTG2MASK
    #define HOME_EINT_SHIFT     4
    #define HOME_KEY            IMAPX200_GPB(4)

    #define VOL0_EINT_GRP       rEINTG2PEND
    #define VOL0_EINT_MSK       rEINTG2MASK
    #define VOL0_EINT_SHIFT     2
    #define VOL0_KEY            IMAPX200_GPB(2)
            
    #define VOL1_EINT_GRP       rEINTG2PEND
    #define VOL1_EINT_MSK       rEINTG2MASK
    #define VOL1_EINT_SHIFT     1
    #define VOL1_KEY            IMAPX200_GPB(1)
    
    #define LOCK_EINT_GRP       rEINTG2PEND
    #define LOCK_EINT_MSK       rEINTG2MASK
    #define LOCK_EINT_SHIFT     0
    #define LOCK_KEY            IMAPX200_GPB(0)

    #define K_HDMI              K_MENU
#endif

#if defined(CONFIG_BOARD_K0)
    #define BACK_EINT_GRP       rEINTG2PEND
    #define BACK_EINT_MSK       rEINTG2MASK
    #define BACK_EINT_SHIFT     0
    #define BACK_KEY            IMAPX200_GPB(0)

    #define MENU_EINT_GRP       rEINTG1PEND
    #define MENU_EINT_MSK       rEINTG1MASK
    #define MENU_EINT_SHIFT     2
    #define MENU_KEY            IMAPX200_GPA(2)

    #define HOME_EINT_GRP       rEINTG1PEND
    #define HOME_EINT_MSK       rEINTG1MASK
    #define HOME_EINT_SHIFT     3
    #define HOME_KEY            IMAPX200_GPA(3)

    #define VOL0_EINT_GRP       rEINTG2PEND
    #define VOL0_EINT_MSK       rEINTG2MASK
    #define VOL0_EINT_SHIFT     1
    #define VOL0_KEY            IMAPX200_GPB(1)

    #define VOL1_EINT_GRP       rEINTG2PEND
    #define VOL1_EINT_MSK       rEINTG2MASK
    #define VOL1_EINT_SHIFT     2
    #define VOL1_KEY            IMAPX200_GPB(2)
                
    #define LOCK_EINT_GRP       rEINTG4PEND
    #define LOCK_EINT_MSK       rEINTG4MASK
    #define LOCK_EINT_SHIFT     8
    #define LOCK_KEY            IMAPX200_GPE(8)

    #define K_HDMI              K_MENU
#endif

struct key_property_list{
    uint32_t name;
    uint32_t order;
    uint32_t short_press;
    uint32_t short_release;
    int  init_val;

    int x0;
    int y0;
    int x1;
    int y1;
};

enum{
    K_POWER=0,
#if defined(BACK_EINT_SHIFT)    
    K_BACK,
#endif    
#if defined(HOME_EINT_SHIFT)    
    K_HOME,
#endif    
#if defined(MENU_EINT_SHIFT)    
    K_MENU,
#endif    
#if defined(VOL0_EINT_SHIFT)
    K_VOL0,
#endif
#if defined(VOL1_EINT_SHIFT)
    K_VOL1,
#endif   
#if defined(LOCK_EINT_SHIFT)
    K_LOCK,
#endif     
    K_MAX,
};

enum{
#if defined(VIRTUAL_BACK_X0)    
    V_BACK,
#endif    
#if defined(VIRTUAL_HOME_X0)    
    V_HOME,
#endif    
#if defined(VIRTUAL_MENU_X0)    
    V_MENU,
#endif
    V_MAX,
};


static struct key_property_list gpio_key_list[K_MAX + V_MAX]={
//power key
    {(uint32_t)-1,  K_POWER,    POWER_CODE,      POWER_CODE,    1},

//actual key
#if defined(BACK_EINT_SHIFT)        
    {BACK_KEY,      K_BACK,     BACK_CODE,       (uint32_t)-1,  -1},
#endif
#if defined(HOME_EINT_SHIFT)        
    {HOME_KEY,      K_HOME,     HOME_CODE,       (uint32_t)-1,  -1},
#endif
#if defined(MENU_EINT_SHIFT)
    {MENU_KEY,      K_MENU,     MENU_CODE,       (uint32_t)-1,  -1},
#endif
#if defined(SWITCH_EINT_SHIFT)  
    {SWITCH_KEY,    K_SWITCH,   SWITCH_CODE,     (uint32_t)-1,  -1},
#endif		
#if defined(VOL0_EINT_SHIFT)
    {VOL0_KEY,      K_VOL0,     VOL0_CODE,       (uint32_t)-1,  -1},
#endif
#if defined(VOL1_EINT_SHIFT)
    {VOL1_KEY,      K_VOL1,     VOL1_CODE,       (uint32_t)-1,  -1},
#endif
#if defined(LOCK_EINT_SHIFT)
    {LOCK_KEY,      K_LOCK,     LOCK_CODE,       UNLOCK_CODE,   -2/*report status whether value*/},
#endif

//virtual key
#if defined(VIRTUAL_BACK_X0)        
    {0,      V_BACK + K_MAX,     BACK_CODE,       (uint32_t)-1,  -1,VIRTUAL_BACK_X0,VIRTUAL_BACK_Y0,VIRTUAL_BACK_X1,VIRTUAL_BACK_Y1},
#endif
#if defined(VIRTUAL_HOME_X0)        
    {0,      V_HOME + K_MAX,     HOME_CODE,       (uint32_t)-1,  -1,VIRTUAL_HOME_X0,VIRTUAL_HOME_Y0,VIRTUAL_HOME_X1,VIRTUAL_HOME_Y1},
#endif
#if defined(VIRTUAL_MENU_X0)
    {0,      V_MENU + K_MAX,     MENU_CODE,       (uint32_t)-1,  -1,VIRTUAL_MENU_X0,VIRTUAL_MENU_Y0,VIRTUAL_MENU_X1,VIRTUAL_MENU_Y1},
#endif
};

#define HOME_BACK
#if defined(CONFIG_HDMI_OUTPUT_SUPPORT)
extern struct completion Menu_Button;
#endif
static volatile int report_as_sleep = 1;
static struct gpio_keys_drvdata *ddata;

struct gpio_button_data {
	struct timer_list timer;
	struct delayed_work work;
	int value[K_MAX + V_MAX];
	int changed[K_MAX + V_MAX];

	int x;
	int y;
	int p;
};

struct gpio_keys_drvdata {
	struct input_dev *input;
	struct gpio_button_data data;
};

void zt_lock_screen(void);
void zt_unlock_screen(void);


int imapx200_gpio_to_irq(unsigned gpio)
{
	return gpio;
}

extern void kernel_halt(void);
// 组合键关机
void zt_shutdown(void)
{
	sys_sync(); 
	kernel_halt();
	return;	
}

static int get_key_value(struct key_property_list *key)
{
    struct gpio_button_data *bdata = &ddata->data;

    int val = 0;
    if(key->name){
        val = !gpio_get_value(key->name);
    }else{
        printk("check v key %d (%d,%d,%d,%d) current(%d,%d,%d)\n",
            key->order,
            key->x0,key->x1,key->y0,key->y1,
            bdata->x,
            bdata->y,
            bdata->p);
        if((bdata->p) &&
            (bdata->x > key->x0 && bdata->x <key->x1)&&
            (bdata->y > key->y0 && bdata->y <key->y1)){
            printk("get v key %d\n",key->order - K_MAX);
            val = 1;
        }
    }

    return val;
}


static void init_key_value(struct gpio_button_data *bdata)
{
    struct key_property_list * const key_list = gpio_key_list,*ptr;
    int i;

    //check key
    for(i=0; i<K_MAX + V_MAX; i++){
        ptr = &key_list[i];

        if(ptr->init_val != -1)
            bdata->value[ptr->order] = ptr->init_val;
        else
            bdata->value[ptr->order] = get_key_value(ptr)/*!gpio_get_value(ptr->name)*/;

        bdata->changed[ptr->order] = 0;
    }
}



static void gpio_keys_report_work(struct work_struct *work)
{
	struct gpio_button_data *bdata = &ddata->data;
	struct input_dev *input = ddata->input;
	unsigned int type = EV_KEY;
	unsigned int pressed,pulse_event,code,reverse_value;

    int i,value;

    struct key_property_list * const key_list = gpio_key_list,*ptr;

    //check key
    for(i=0; i<K_MAX + V_MAX; i++){
        ptr = &key_list[i];

        if(ptr->name == (uint32_t)-1)
            continue;
        
        pressed = get_key_value(ptr)/*!gpio_get_value(ptr->name)*/;    
        //printk("key %d = %d\n",ptr->order,pressed);
        if(pressed != bdata->value[ptr->order]){
            bdata->value[ptr->order] = pressed;
            bdata->changed[ptr->order] = 1;
        }
    }

    //report key
    for(i=0; i<K_MAX + V_MAX; i++){
        ptr = &key_list[i];
        
        if(bdata->changed[ptr->order]){
            pulse_event = 0; 
            reverse_value = 0;
          
            if(ptr->order == K_POWER      //power key only has release action
#if defined(LOCK_EINT_SHIFT)
                ||ptr->order == K_LOCK
#endif
            ){ 
                pulse_event = 1;
            }
            if(ptr->order == K_POWER){      //power button 
                reverse_value = 1;
            }

          #if defined(CONFIG_HDMI_OUTPUT_SUPPORT)
            if(ptr->order == K_HDMI && bdata->value[ptr->order]){ //hdmi function
                //printk("hdmi key pressed\n");
                complete(&Menu_Button);
            }
          #endif
            
            if(pulse_event){
                code = bdata->value[ptr->order]?ptr->short_press:ptr->short_release;
                value = reverse_value?0:1;
            }else{
                code = ptr->short_press;
                value = bdata->value[ptr->order];
            }
            
            //printk("report key[%d] code %d press %d\n",ptr->order,code,value);
            input_event(input, type, code, value);
            if(pulse_event){
                input_event(input, type, code, reverse_value?1:0);
            }

            bdata->changed[ptr->order] = 0;
        }
    }
}



static void gpio_keys_timer(unsigned long data)
{
	struct gpio_button_data *bdata = (struct gpio_button_data *)data;
	
	schedule_work((struct work_struct *)&bdata->work);
}


static irqreturn_t gpio_keys_isr_47(int irq, void *dev_id)
{
    //struct gpio_keys_button *button = (struct gpio_keys_button *)dev_id;
    struct gpio_button_data *bdata = &ddata->data;

#if defined(BACK_EINT_SHIFT)    
	unsigned int back;
#endif	
#if defined(HOME_EINT_SHIFT)    
    unsigned int home;
#endif
#if defined(MENU_EINT_SHIFT)
    unsigned int menu;
#endif
#if defined(SWITCH_EINT_SHIFT)
    unsigned int swh;
#endif    
#if defined(VOL0_EINT_SHIFT)
    unsigned int vol0;
#endif
#if defined(VOL1_EINT_SHIFT)
        unsigned int vol1;
#endif
#if defined(LOCK_EINT_SHIFT)
        unsigned int lock;
#endif


	unsigned long flags;

    irqreturn_t ret = IRQ_HANDLED;

	local_irq_save(flags);
#if defined(BACK_EINT_SHIFT) 		
	back = __raw_readl(BACK_EINT_GRP);
#endif
#if defined(HOME_EINT_SHIFT) 	
	home = __raw_readl(HOME_EINT_GRP);
#endif
#if defined(MENU_EINT_SHIFT)		
	menu = __raw_readl(MENU_EINT_GRP);
#endif	
#if defined(SWITCH_EINT_SHIFT)
    swh = __raw_readl(SWITCH_EINT_GRP);
#endif
#if defined(VOL0_EINT_SHIFT)
    vol0 = __raw_readl(VOL0_EINT_GRP);
#endif
#if defined(VOL1_EINT_SHIFT)
    vol1 = __raw_readl(VOL1_EINT_GRP);
#endif
#if defined(LOCK_EINT_SHIFT)
    lock = __raw_readl(LOCK_EINT_GRP);
#endif    

    if(0){
        ;
#if defined(BACK_EINT_SHIFT)
    }else if( back & (1 << BACK_EINT_SHIFT)) {
            DBG("back irq\n");
            __raw_writel((1 << BACK_EINT_SHIFT), BACK_EINT_GRP);
#endif
#if defined(HOME_EINT_SHIFT) 
    }else if(home & (1<<HOME_EINT_SHIFT) ) {
        DBG("home irq\n");
        __raw_writel((1 << HOME_EINT_SHIFT), HOME_EINT_GRP);
#endif    
#if defined(MENU_EINT_SHIFT)
    }else if( menu & (1 << MENU_EINT_SHIFT)) {
        DBG("menu irq\n");
        __raw_writel((1 << MENU_EINT_SHIFT), MENU_EINT_GRP);
#endif        
#if defined(SWTICH_EINT_SHIFT)
    }else if( swh & (1 << SWITCH_EINT_SHIFT)) {
        __raw_writel((1 << SWITCH_EINT_SHIFT), SWITCH_EINT_GRP);
#endif
#if defined(VOL0_EINT_SHIFT)
    }else if( vol0 & (1 << VOL0_EINT_SHIFT)) {
        DBG("vol0 irq\n");
        __raw_writel((1 << VOL0_EINT_SHIFT), VOL0_EINT_GRP);
#endif
#if defined(VOL1_EINT_SHIFT)
    }else if( vol1 & (1 << VOL1_EINT_SHIFT)) {
        DBG("vol1 irq\n");
        __raw_writel((1 << VOL1_EINT_SHIFT), VOL1_EINT_GRP);
#endif
#if defined(LOCK_EINT_SHIFT)
    }else if( lock & (1 << LOCK_EINT_SHIFT)) {
        DBG("lock irq\n");
        __raw_writel((1 << LOCK_EINT_SHIFT), LOCK_EINT_GRP);
#endif        
    }else{
        ret = IRQ_NONE;
    }

    if(ret!=IRQ_NONE){
    	schedule_delayed_work(&bdata->work,HZ/10);
    }
    
	local_irq_restore(flags);
	return ret;
}

void zt_lock_screen()
{
	struct input_dev *input = ddata->input;
	unsigned int type = EV_KEY;
	
	msleep(100);
	//input_sync(input);
	input_event(input, type, POWER_CODE, 0);
	input_event(input, type, POWER_CODE, 1);
	input_event(input, type, POWER_CODE, 0);
	input_sync(input);
	msleep(100);
}

void zt_unlock_screen(void)
{
	struct input_dev *input = ddata->input;
	unsigned int type = EV_KEY;
	
	msleep(100);
    input_event(input, type, HOME_CODE, 1);
    input_event(input, type, HOME_CODE, 0);
    
    msleep(100);
    input_event(input, type, MENU_CODE, 1);
    input_event(input, type, MENU_CODE, 0);   

	msleep(100);
}



void zt_shut_down(void)
{
	struct input_dev *input = ddata->input;
	unsigned int type = EV_KEY;
	
	msleep(100);
	//input_sync(input);
	input_event(input, type, POWER_CODE, 0);
	input_event(input, type, POWER_CODE, 1);
	input_sync(input);
	msleep(100);
}

static irqreturn_t gpio_keys_isr(int irq, void *dev_id)
{
    struct gpio_keys_button *button = (struct gpio_keys_button *)dev_id;
    struct gpio_button_data *bdata = &ddata->data;
	volatile unsigned int tmp;
		
	//spin_lock_irqsave(&imapx200_gpio_lock,flags);	
	
	tmp = __raw_readl(rPOW_STB);
	
	//printk("gpio_keys_isr : tmp = 0x%x\r\n",tmp);
	if (tmp & 0x2 || tmp & 0x1){
		while(tmp != 0){
			if(tmp& 0x2)
				__raw_writel(0x2, rPOW_STB);
			else 
				__raw_writel(0x1, rPOW_STB);
			
			udelay(300);
			tmp = __raw_readl(rPOW_STB);
		}
		bdata->value[K_POWER] = 0/*POWER_CODE*/;
		bdata->changed[K_POWER] = 1/*POWER_CODE*/;
		//printk(KERN_INFO "State 0x2\n");
		
		
		report_as_sleep = 0;
		goto isr_sc_work;
	}
	else if (tmp & 0x4)
	{
		__raw_writel(0x4, rPOW_STB);
	}
	else{
		printk(KERN_ERR "this is the wron value, %d!\n", tmp);
	}	

	printk("State 0x%x\n",tmp);
	report_as_sleep = 1;
isr_sc_work:
#if 1
	if (button->debounce_interval)
		mod_timer(&bdata->timer,
			jiffies + msecs_to_jiffies(button->debounce_interval));
	else
		schedule_delayed_work(&bdata->work,HZ/10);
#endif
	//spin_unlock_irqrestore(&imapx200_gpio_lock,flags);
	return IRQ_HANDLED;
}

void virtual_key_isr(int x,int y,int p)
{
    struct gpio_button_data *bdata = &ddata->data;

    printk("virtual (%d %d %d)\n",x,y,p);

    bdata->x = x;
    bdata->y = y;
    bdata->p = p;

    schedule_delayed_work(&bdata->work,HZ/10);
}

static int config_gpio_key(void)
{
	if (
	  #if defined(BACK_EINT_SHIFT)
	    gpio_request(BACK_KEY, "back key")||
	  #endif  
      #if defined(HOME_EINT_SHIFT)	
	    gpio_request(HOME_KEY, "home key")||
      #endif
	  #if defined(MENU_EINT_SHIFT)  
	    gpio_request(MENU_KEY, "menu key")||
	  #endif  
	  #if defined(SWITCH_EINT_SHIFT)
	    gpio_request(SWITCH_KEY, "switch key")||
	  #endif  
	  #if defined(VOL0_EINT_SHIFT)
        gpio_request(VOL0_KEY, "vol0 key")||
      #endif
      #if defined(VOL1_EINT_SHIFT)
        gpio_request(VOL1_KEY, "vol1 key")||
      #endif
      #if defined(LOCK_EINT_SHIFT)
        gpio_request(LOCK_KEY, "lock key")||
      #endif  
        0
	  ) {
		printk("config_gpio_key : request key is error\r\n");
		return -1;	
	}
#if defined(BACK_EINT_SHIFT)		
	gpio_direction_input(BACK_KEY);
	imapx200_gpio_setpull(BACK_KEY, 0);
#endif	
#if defined(HOME_EINT_SHIFT)	
    gpio_direction_input(HOME_KEY);
    imapx200_gpio_setpull(HOME_KEY, 0);
#endif
#if defined(MENU_EINT_SHIFT)
	gpio_direction_input(MENU_KEY);
	imapx200_gpio_setpull(MENU_KEY, 0);
#endif
#if defined(SWITCH_EINT_SHIFT)
    gpio_direction_input(SWITCH_KEY);
    imapx200_gpio_setpull(SWITCH_KEY, 0);
#endif    
#if defined(VOL0_EINT_SHIFT)
	gpio_direction_input(VOL0_KEY);
	imapx200_gpio_setpull(VOL0_KEY, 0);
#endif	
#if defined(VOL1_EINT_SHIFT)
	gpio_direction_input(VOL1_KEY);
	imapx200_gpio_setpull(VOL1_KEY, 0);
#endif
#if defined(LOCK_EINT_SHIFT)
	gpio_direction_input(LOCK_KEY);
	imapx200_gpio_setpull(LOCK_KEY, 0);
#endif	

    local_irq_disable();

    //config both edge
    __raw_writel(0x666666,rEINTGCON);  //all both edge
    __raw_writel(0xffffffff,rEINTGFLTCON0);
    __raw_writel(0xffff,rEINTGFLTCON0);

#if defined(BACK_EINT_SHIFT)    
    __raw_writel(__raw_readl(BACK_EINT_MSK) & (~(0x1<<BACK_EINT_SHIFT)),
        BACK_EINT_MSK);
#endif
#if defined(HOME_EINT_SHIFT)
    __raw_writel(__raw_readl(HOME_EINT_MSK) & (~(0x1<<HOME_EINT_SHIFT)),
        HOME_EINT_MSK);
#endif
#if defined(MENU_EINT_SHIFT)        
    __raw_writel(__raw_readl(MENU_EINT_MSK) & (~(0x1<<MENU_EINT_SHIFT)),
        MENU_EINT_MSK);
#endif        

#if defined(SWITCH_EINT_SHIFT)
    __raw_writel(__raw_readl(SWITCH_EINT_MSK) & (~(0x1<<SWITCH_EINT_SHIFT)),
        SWITCH_EINT_MSK);
#endif        
#if defined(VOL0_EINT_SHIFT)
    __raw_writel(__raw_readl(VOL0_EINT_MSK) & (~(0x1<<VOL0_EINT_SHIFT)),
        VOL0_EINT_MSK);
#endif
#if defined(VOL1_EINT_SHIFT)
    __raw_writel(__raw_readl(VOL1_EINT_MSK) & (~(0x1<<VOL1_EINT_SHIFT)),
        VOL1_EINT_MSK);
#endif
#if defined(LOCK_EINT_SHIFT)
    __raw_writel(__raw_readl(LOCK_EINT_MSK) & (~(0x1<<LOCK_EINT_SHIFT)),
        LOCK_EINT_MSK);       
#endif

    local_irq_enable();

    return 0;
}

static int __devinit gpio_keys_probe(struct platform_device *pdev)
{
	struct gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_button_data *bdata;
	struct input_dev *input;
	int i, error;
	int wakeup = 0;

/*****************************************************************/
	printk("gpio_keys_probe : pdata->nbuttons = 0x%x\r\n",pdata->nbuttons);
	config_gpio_key();
	ddata = kzalloc(sizeof(struct gpio_keys_drvdata) /*+
			pdata->nbuttons * sizeof(struct gpio_button_data)*/,
			GFP_KERNEL);
	input = input_allocate_device();
	if (!ddata || !input) {
		error = -ENOMEM;
		goto fail1;
	}

	platform_set_drvdata(pdev, ddata);

	input->name = pdev->name;
	input->phys = "gpio-keys/input0";
	input->dev.parent = &pdev->dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;
	//spin_lock_init(&imapx200_gpio_lock);
	/* Enable auto repeat feature of Linux input subsystem */
//	if (pdata->rep)
//		__set_bit(EV_REP, input->evbit);

	ddata->input = input;
	bdata = &ddata->data;
    init_key_value(bdata);
    
    setup_timer(&bdata->timer,
                    gpio_keys_timer, (unsigned long)bdata);
    INIT_DELAYED_WORK(&bdata->work, gpio_keys_report_work);


	disable_irq(47);
	for (i = 0; i < pdata->nbuttons; i++) {
		struct gpio_keys_button *button = &pdata->buttons[i];
		int irq;
		unsigned int type = button->type ?: EV_KEY;

		irq = imapx200_gpio_to_irq(button->gpio);
		//printk("gpio_keys_probe : irq = 0x%x\r\n",irq);
		if (irq < 0) {
			error = irq;
			pr_err("gpio-keys: Unable to get irq number"" for GPIO %d, error %d\n",button->gpio, error);
			goto fail2;
		}
		if(47 == irq){
			error = request_irq(irq, gpio_keys_isr_47,IRQF_SHARED, button->desc ? button->desc : "gpio_keys",button);
			if (error) {
				pr_err("gpio-keys: Unable to claim irq %d; error %d\n",irq, error);
				goto fail2;
			}			
		} else {
			error = request_irq(irq, gpio_keys_isr,IRQF_SHARED, button->desc ? button->desc : "gpio_keys ",button);
			if (error) {
				pr_err("gpio-keys: Unable to claim irq %d; error %d\n",irq, error);
				goto fail2;
			}
		}

		if (button->wakeup)
			wakeup = 1;

		input_set_capability(input, type, button->code);
	}

	//enable_irq(47);
	/* Add 116 to keybit, by warits */
	set_bit(POWER_CODE, input->keybit);
	set_bit(HOME_CODE, input->keybit);
	set_bit(MENU_CODE, input->keybit);
	set_bit(BACK_CODE, input->keybit);
	set_bit(VOL0_CODE, input->keybit);
	set_bit(VOL1_CODE, input->keybit);
	set_bit(LOCK_CODE, input->keybit);
	set_bit(UNLOCK_CODE, input->keybit);

	error = input_register_device(input);
	if (error) {
		pr_err("gpio-keys: Unable to register input device, "
			"error: %d\n", error);
		goto fail2;
	}
	
	printk("gpio_keys_probe is OK\r\n");
	
	device_init_wakeup(&pdev->dev, wakeup);

    schedule_delayed_work(&bdata->work,HZ*40);

	return 0;

 fail2:
	while (--i >= 0) {
		free_irq(imapx200_gpio_to_irq(pdata->buttons[i].gpio), &pdata->buttons[i]);
	}

    del_timer_sync(&ddata->data.timer);
    cancel_work_sync((struct work_struct *)&ddata->data.work);

	platform_set_drvdata(pdev, NULL);
 fail1:
	input_free_device(input);
	kfree(ddata);

	return error;
}

static int __devexit gpio_keys_remove(struct platform_device *pdev)
{
	struct gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_keys_drvdata *ddata = platform_get_drvdata(pdev);
	struct input_dev *input = ddata->input;
	int i;

	device_init_wakeup(&pdev->dev, 0);

	for (i = 0; i < pdata->nbuttons; i++) {
		int irq = imapx200_gpio_to_irq(pdata->buttons[i].gpio);
		free_irq(irq, &pdata->buttons[i]);
	}
    del_timer_sync(&ddata->data.timer);
    cancel_work_sync((struct work_struct *)&ddata->data.work);
	
	input_unregister_device(input);

	return 0;
}


#ifdef CONFIG_PM
static int gpio_keys_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	int i;

	if (device_may_wakeup(&pdev->dev)) {
		for (i = 0; i < pdata->nbuttons; i++) {
			struct gpio_keys_button *button = &pdata->buttons[i];
			if (button->wakeup) {
				int irq = imapx200_gpio_to_irq(button->gpio);
				enable_irq_wake(irq);
			}
		}
	}

	return 0;
}

static int gpio_keys_resume(struct platform_device *pdev)
{
	struct gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	int i;

	if (device_may_wakeup(&pdev->dev)) {
		for (i = 0; i < pdata->nbuttons; i++) {
			struct gpio_keys_button *button = &pdata->buttons[i];
			if (button->wakeup) {
				int irq = imapx200_gpio_to_irq(button->gpio);
				disable_irq_wake(irq);
			}
		}
	}

	return 0;
}
#else
#define gpio_keys_suspend	NULL
#define gpio_keys_resume	NULL
#endif

static struct platform_driver gpio_keys_device_driver = {
	.probe		= gpio_keys_probe,
	.remove		= __devexit_p(gpio_keys_remove),
	.suspend	= gpio_keys_suspend,
	.resume		= gpio_keys_resume,
	.driver		= {
		.name	= "gpio-keys",
		.owner	= THIS_MODULE,
	}
};

static int __init gpio_keys_init(void)
{
	return platform_driver_register(&gpio_keys_device_driver);
}

static void __exit gpio_keys_exit(void)
{
	platform_driver_unregister(&gpio_keys_device_driver);
}

module_init(gpio_keys_init);
module_exit(gpio_keys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phil Blundell <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:gpio-keys");
