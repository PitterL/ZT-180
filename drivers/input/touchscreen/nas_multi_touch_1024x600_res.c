#include <linux/errno.h>

#include <linux/kernel.h>

#include <linux/module.h>

#include <linux/slab.h>

#include <linux/input.h>

#include <linux/init.h>

#include <linux/serio.h>

#include <linux/delay.h>

#include <linux/platform_device.h>

#include <linux/clk.h>

#include <linux/irq.h>

#include <linux/interrupt.h>

#include <linux/i2c.h>

#include <linux/timer.h>

#include <linux/workqueue.h>

#include <linux/poll.h>

#include <linux/spinlock.h>



#include <asm/io.h>

#include <asm/irq.h>

#include <asm/gpio.h>

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <asm/io.h>

//#include "touchp.h"



//#define GPIO_00_PIN     85



//#define TRUE	1

//#define FALSE 	0



#define UORxL_AddW 0x96//0x90          //UOR AddresS for Writing"1001 1010"

#define UORxL_AddR 0x97//0x91          //UOR Address for Reading"1001 1011"

#define InitX2                0x00

#define InitY2                0x10

#define CalibX                0x20

#define CalibY                0x30

#define MSRX_2T                0x40

#define MSRY_2T                0x50

#define MSRX_1T                0xC0

#define MSRY_1T                0xD0

#define MSRZ1_1T               0xE0

#define MSRZ2_1T               0xF0



#define ReadDx		0x40

#define ReadDy		0x50

#define InitX			0x00

#define InitY			0x10

#define CalibX		0x20

#define CalibY		0x30

#define ReadX		  0xC0

#define ReadY		  0xD0

#define ReadZ1		0xE0

#define ReadZ2		0xF0

#define GPIOPortE_Pin3 IRQ_EINT4
#define UOR6153_I2C_NAME   "uor6153_ts"
#define FALSE 0
#define TRUE 1
//#define	                    GESTURE_IN_DRIVER//!!! Turn on Gesture Detection in Driver !!!
#define Tap				    1	//Tap

#define RHorizontal		            3	//Right horizontal

#define LHorizontal		            4	//Left horizontal

#define UVertical			5	//Up vertical

#define DVertical			6	//Down vertical

#define RArc				7	//Right arc

#define LArc				8	//Left arc

#define CWCircle			9	//Clockwise circle

#define CCWCircle		        10	//Counter-clockwise circle

#define RPan				11	//Right pan

#define LPan				12	//Left pan

#define DPan				13	//Right pan

#define UPan				14	//Left pan

#define PressTap			15	//Press and tap

#define PinchIn			        16	//Pinch in

#define PinchOut			17	//Pinch out

enum ts_stat{

GESVALUE_HAVE,

GESVALUE_NO,

GESVALUE_CLOSE,

};



#define R_xplate	 1024

//z value top
#define R_Threshold 	 10000

//z value bottom
#define R_Threshold2     500 //mmmmmm

#define DeltaX 100

#define DeltaY 100

#define XMax 3500

#define XMin	300

#define YMax 3000

#define YMin	300



#define ZERO_TOUCH	0	

#define ONE_TOUCH	1

#define TWO_TOUCH	2


//distance to multi-touch
#define DX_T			65

#define DY_T			65

#define DXY_SKIP		0x80	//Ÿ×±ŒžõÅD¶W¹L¶ZÂ÷DXY_SKIPªºšâÂI



#define NumberFilter		4

#define NumberDrop			2	//This value must not bigger than (NumberFilter-1)


//drop point number at begin
#define FIRSTTOUCHCOUNT		0//ÂoÂIX: «eX­Ó³æÂI

//drop point number after multi-touch end
#define ONETOUCHCountAfter2	20//ÂoÂIY: šâÂI«áY­Ó³æÂI

#define JITTER_THRESHOLD	1000//ÂoÂI: «e«ášâ­Ó³æÂI­Y¶W¹LŠ¹­È

#define	MAX_READ_PERIOD	     25//³Ì€jÅªÂI¶¡¹j(ms)

#define FIRST_TWO_TOUCH_FILTER		1//Âo³Ì«e­±Z­ÓšâÂI

#define JITTER_THRESHOLD_DXDY	48//ÂoÂI: «e«ášâ­Óšâ«ü­Y¶W¹LŠ¹­È

#define PERIOD_PER_FILTER	0//filterÅªÂI¶¡¹j(us)

#define	DROP_POINT_DELAY_MS			10//oI®ɪºdelay time(ms)
#define	READ_DATA_DELAY_MS			20//ŪI®ɪºdelay time(ms)
//#define	DROP_POINT_DELAY_J			msecs_to_jiffies(DROP_POINT_DELAY_MS)//oI®ɪºdelay jiffies(related to your HZ)
//#define	READ_DATA_DELAY_J			msecs_to_jiffies(READ_DATA_DELAY_MS) //ŪI®ɪºdelay jiffies(related to your HZ)

//jiffies time to report
#define	DROP_POINT_DELAY_J	  1		
#define	READ_DATA_DELAY_J	  3    // 3

//#define FILTER_FUNC
#define NFilt NumberFilter

#define NDrop NumberDrop





typedef signed char VINT8;

typedef unsigned char VUINT8;

typedef signed short VINT16;

typedef unsigned short VUINT16;

typedef unsigned long VUINT32;

typedef signed long VINT32;



struct uor_touch_screen_struct {

	struct i2c_client *client;

	struct input_dev *dev;

	long xp;

	long yp;

	long pX;

	long pY;

	long pDX;

	long pDY;

	int count;

	int shift;

	unsigned char n_touch;

	

	wait_queue_head_t wq;

	spinlock_t lock;

	struct timer_list	ts_timer;

	unsigned char ges_status;

	unsigned char GesNo;

};



static struct uor_touch_screen_struct ts;



static int uor_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id);

static int uor_detach_adapter(struct i2c_client *client);

static int uor_i2c_detect(struct i2c_adapter *adapter, int address, int kind);

VINT8 Init_UOR_HW(void);



void SendGestureKey(VUINT8);

VUINT8 gesture_decision(VUINT8,VUINT16,VUINT16,VUINT16,VUINT16);

//volatile struct adc_point gADPoint;

#if 1
static int reg_read_n(u8 reg,u8 * data,int n)
{
	int ret;
	
	/* Set address */
	ret = i2c_master_send(ts.client, &reg, 1);
	if (ret != 1){
		printk("%s reg 0x%x failed #1 ret %d\n",__func__,reg ,ret);
		ret = -1;
		goto error;
	}

	ret = i2c_master_recv(ts.client, data, n);
	if (ret != n){
		printk("%s reg 0x%x failed #2 ret %d\n",__func__,reg ,ret);
		ret = -1;
		goto error;
	}

    //printk("%s reg 0x%x : 0x%x %d\n",__func__,reg,*data,ret);

error:
	return ret;
}
#endif


#ifdef CONFIG_PM

int uor_suspend(struct i2c_client *client, pm_message_t mesg)

{

	//disable your interrupt !!!

//	disable_irq(TOUCH_INT_IOPIN);

//	gpio_irq_disable(GPIOPortE_Pin3);

	return 0;

}



int uor_resume(struct i2c_client *client)

{

	//enable your interrupt !!!

	//enable_irq(TOUCH_INT_IOPIN);

//	gpio_irq_enable(GPIOPortE_Pin3);

	return 0;

}

#else

#define uor_suspend NULL

#define uor_resume  NULL

#endif


static const struct i2c_device_id uor6153_id[] = {
	{ UOR6153_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver uor_i2c_driver = {
	.probe = uor_i2c_probe,
	.remove = uor_detach_adapter,
	.id_table = uor6153_id,
	.driver = {
	    .owner	= THIS_MODULE,
		.name = UOR6153_I2C_NAME,
	},
};


static unsigned short normal_i2c[]      = {UORxL_AddW>>1, I2C_CLIENT_END};

static unsigned short probe[2]          = {I2C_CLIENT_END, I2C_CLIENT_END};

static unsigned short ignore[2]         = {I2C_CLIENT_END, I2C_CLIENT_END};



static struct i2c_client_address_data addr_data = {

        normal_i2c,

        probe,

        ignore,

};


#if 0
static int uor_i2c_detect(struct i2c_adapter *adapter, int address, int kind)

{

	struct i2c_client *client;



//	printk("uor.c:uor_i2c_detect() --oql\n");

	

    client = kzalloc(sizeof(*client), GFP_KERNEL);

    if (client == NULL)

	{

		printk("uor.c:uor_i2c_detect() client ==null--oql\n");	

    	return -ENOMEM;

	}

    client->adapter = adapter;

    client->addr = address;

	client->driver = &uor_i2c_driver;

	client->Channel = I2C_CH0;

	client->mode = NORMALMODE;

	client->speed = 100;// 80;// 400;
	client->addressBit = I2C_7BIT_ADDRESS_8BIT_REG;

	i2c_set_clientdata(client, &ts);	

	ts.client = client; // save the client we get

    i2c_attach_client(client);

	 if(Init_UOR_HW() < 0)

		 printk(KERN_ERR "uor.c: Init_UOR_HW() fail in uor_init()!\n");

	

	return 0;

}
#endif



static int uor_detach_adapter(struct i2c_client *client){

//	printk("uor.c:uor_detach_adapter() \n");

	//i2c_detach_client(client);

	//kfree(client);

	return 0;

}






static int UOR_IICRead(char command, char *rxdata, int length)
{

    return reg_read_n(command,rxdata,length);

    /*
	int retry;

	struct i2c_msg msgs[] = {
		{
		.addr = ts.client->addr,
		.flags = 0,
		.len = 1,
		.buf = &command,
		},
		{
		.addr = ts.client->addr,
		.flags = I2C_M_RD,
		.len = length,
		.buf = rxdata,
		},
	};

	for (retry = 0; retry < 3; retry++) {
		if (i2c_transfer(ts.client->adapter, msgs, 2) > 0)
			break;
		else
			udelay(100);
	}
	if (retry >= 3) {
		printk(KERN_ERR"%s: retry over 10!\n", __func__);
		return -EIO;
	}
	return 0;
	*/
}



#if 0

VINT8 UOR_IICRead(VUINT8 Command,VUINT8 *readdata,VINT8 nread)
{//Read bytes from UOR via I2C



	 *readdata = Command;

	i2c_master_recv(ts.client,readdata,nread);

	return 0;

}
#endif


int xFilter[NFilt], yFilter[NFilt],DxFilter[NFilt], DyFilter[NFilt];

unsigned int XYIndex = 0;



int  XYFilter(int *xFilter, int *yFilter, int Num,int Drop){

	unsigned int i,SumTempX=0,SumTempY=0;

	int Dp,checkSmx,checkSmy,checkBgx,checkBgy;

	int SmX =0, SmY = 0;

	int LaX = 0, LaY = 0;

	int SmInX = 0, SmInY = 0;

	int LaInX = 0, LaInY =0;

// printk("in filter 0 :SmInX %d,LaInX %d, SmInY %d , LaInY %d!!!\n", SmInX,LaInX, SmInY, LaInY);

	if( (Num <=2) && (Drop > (Num-1)) )

		return FALSE; // not enough to sample

		

	for(i=0;i<Num;i++){

		SumTempX += xFilter[i];

		SumTempY += yFilter[i];

	}

	

	Dp = Drop;



	checkSmx = 0;

	checkSmy = 0;

	checkBgx = 0;

	checkBgy = 0;

	while(Dp>0){

		SmX = 0x0FFF;SmY = 0x0FFF;

		LaX = 0x0;LaY = 0x0;

		SmInX = 0;SmInY = 0;

		LaInX = 0;LaInY =0;

		for(i =  0; i < Num; i++){

			if(checkSmx&(1<<i)){

			}else if(SmX > xFilter[i]){

				SmX = xFilter[i];

				SmInX= i;

			}

			if(checkSmy&(1<<i)){

			}else if(SmY > yFilter[i]){

				SmY = yFilter[i];

				SmInY = i;

			}

			

			if(checkBgx&(1<<i)){

			}else if(LaX < xFilter[i]){

				LaX = xFilter[i];

				LaInX = i;

			}

			

			if(checkBgy&(1<<i)){

			}else if(LaY < yFilter[i]){

				LaY = yFilter[i];

				LaInY = i;

			}

		}

		if(Dp){

			SumTempX-= xFilter[SmInX];

			SumTempX-= xFilter[LaInX];

			SumTempY-= yFilter[SmInY];

			SumTempY-= yFilter[LaInY];

			Dp -= 2;

		//	printk("in filter 1 :SmInX %d,LaInX %d, SmInY %d , LaInY %d!!!\n", SmInX,LaInX, SmInY, LaInY);

		}

		checkSmx |= 1<<SmInX;

		checkSmy |= 1<<SmInY;

		checkBgx |= 1<<LaInX;

		checkBgy |= 1<<LaInY;

	}

	

	xFilter[0] = SumTempX/(Num-Drop);

	yFilter[0] = SumTempY/(Num-Drop);

//	printk("in filter 2 :SmInX %d,LaInX %d, SmInY %d , LaInY %d!!!\n", SmInX,LaInX, SmInY, LaInY);

	return TRUE;

}





VINT8 Init_UOR_HW(void){



	VUINT8   i,icdata[2];

	VUINT32   Dx_REF,Dy_REF,Dx_Check,Dy_Check;

	int		  TempDx[NumberFilter],TempDy[NumberFilter];



	for(i=0;i<NumberFilter;i++){

		UOR_IICRead(InitX,icdata,2);

		TempDx[i] = (icdata[0]<<4 | icdata[1]>>4);

        UOR_IICRead(InitY,icdata,2);

		TempDy[i] = (icdata[0]<<4 | icdata[1]>>4);

		//printk(KERN_ERR "filter test:#%d (x,y)=(%d,%d) !!!\n", i,TempDx[i], TempDy[i]);

	}

	XYFilter(TempDx,TempDy,NumberFilter,2);

    Dx_REF = TempDx[0];

    Dy_REF = TempDy[0];

	//printk(KERN_ERR "filter result:(x,y)=(%d,%d) !!!\n", Dx_REF, Dy_REF);

	

	i = 0;

	do{



		UOR_IICRead(InitX,icdata,2);

		Dx_Check = abs((icdata[0]<<4 | icdata[1]>>4) - Dx_REF);

		UOR_IICRead(InitY,icdata,2);

		Dy_Check = abs((icdata[0]<<4 | icdata[1]>>4) - Dy_REF);



		i++;



		if(i>NumberFilter)

			return -1;

//	printk("%s:%d,%d,%d\n",__FUNCTION__,__LINE__,Dx_Check,Dy_Check);



	}while(Dx_Check > 4 || Dy_Check > 4);



	return 0;

}



#ifdef  GESTURE_IN_DRIVER

VINT8 PressAndTap = 0,n_touch=0,circle_flag = 0;

VUINT8 touch_flag[2]={0,0},direction_flag1[4],flag_pinch = 0,flag_pan = 0,flag_count = 0;

VUINT16 Dx1,Dx2,Dy1,Dy2,X1,X2,Y1,Y2,XX1,XX2,YY1,YY2;

VUINT16 DIS_1T,DIS_PAN,DIS_PINCH;

VUINT8 RightPan = 0, LeftPan = 0, DownPan = 0, UpPan = 0,circle_direction_flag=0;



void SendGestureKey(VUINT8 Gesture ){

		switch(Gesture){

		case Tap:

			ts.GesNo = 'T';

			printk(KERN_INFO "Gesture is TAP\r\n");

			break;

		case RHorizontal:

			ts.GesNo = 'R';

			printk(KERN_INFO "Gesture is RHorizontal\r\n");

			break;

		case LHorizontal:

			ts.GesNo = 'L';

			printk(KERN_INFO "Gesture is LHorizontal\r\n");

			break;

		case UVertical:

			ts.GesNo = 'U';

			printk(KERN_INFO "Gesture is UVertical\r\n");

			break;

		case DVertical:

			ts.GesNo = 'D';

			printk(KERN_INFO "Gesture is DVertical\r\n");

			break;

		case RArc:

			ts.GesNo = 'A';

			printk(KERN_INFO "Gesture is RArc\r\n");

			break;

		case LArc:

			ts.GesNo = 'A';

			printk(KERN_INFO "Gesture is LArc\r\n");

			break;

		case CWCircle:

			ts.GesNo = 'C';

			printk(KERN_INFO "Gesture is CWCircle\r\n");



			break;

		case CCWCircle:

			ts.GesNo = 'c';

			printk(KERN_INFO "Gesture is CCWCircle\r\n");



			break;

		case RPan:

			ts.GesNo = 'r';

			printk(KERN_INFO "Gesture is RPan\r\n");



			break;

		case LPan:

			ts.GesNo = 'l';

			printk(KERN_INFO "Gesture is LPan\r\n");



			break;

		case DPan:

			ts.GesNo = 'd';			

			printk(KERN_INFO "Gesture is DPan\r\n");



			break;

		case UPan:

			ts.GesNo = 'u';

			printk(KERN_INFO "Gesture is UPan\r\n");



			break;

		case PressTap:

			ts.GesNo = 'p';

			

			printk(KERN_INFO "Gesture is PressTap\r\n");

		

			break;



		case PinchIn:

			ts.GesNo = 'I';

			printk(KERN_INFO "Gesture is PinchIn\r\n");



			break;

		case PinchOut:

			ts.GesNo = 'O';		

			printk(KERN_INFO "Gesture is PinchOut\r\n");



			break;

		default:

//			printk(KERN_INFO "Gesture key is %d \n", Gesture);

			break;

		}

/*		

	if(Gesture != 0 && ts.ges_status != GESVALUE_CLOSE){

//		ts.GesNo = gesture;

		ts.ges_status = GESVALUE_HAVE;

		wake_up_interruptible(&(ts.wq));

	}

*/

}





VUINT8 check_event(VUINT16 x1,VUINT16 x2,VUINT16 y1,VUINT16 y2,VUINT16 dis){

	

	VUINT8 direction = 0x00;

	VUINT16 absX = 0, absY = 0;

 	//Decide what the direction is from (x1,y1) to (x2,y2)

 	absX = (x1>x2)?(x1-x2):(x2-x1);

	absY = (y1>y2)?(y1-y2):(y2-y1);

	if(absX>dis)

		direction |= (x1 > x2 ? 0x02:0x01);		//0x01:Right	0x02:Left

	else if(absY>dis)

		direction |= (y1 > y2 ? 0x08:0x04);		//0x08:Up		0x04:Down



	return direction;

}



//Gesture decision function

VUINT8 gesture_decision(VUINT8 n_touch,VUINT16 x,VUINT16 y, VUINT16 Dx, VUINT16 Dy){



	VUINT8 gesture =0,flag_d=0,flag_v = 0;

	VUINT16 event_flag = 0;



	DIS_PINCH = 0x18;

//	DIS_PAN = 2500;

	DIS_PAN = 800;

//	DIS_1T = 500;

	DIS_1T = 350;

	



		if(n_touch == 2){

		//Number of touch is 2,

		//clear flags for 1-touch gesture decision.

		touch_flag[0] = 0;

		memset(direction_flag1,0,4);



		if(!touch_flag[1]){ 

			//start a new 2Touch gesture decision flow

			touch_flag[1] = 1;

			PressAndTap = 1;

			XX1 = x;

			YY1 = y;

			Dx1 = Dx;

			Dy1 = Dy;

		}else{

			//Check to see if any 2-Touch event happens.



			XX2 = x;

			YY2 = y;



			//If the variation in Dx or Dy is bigger than DIS_PINCH or the variation in 

			//the coordinate of central point between the two touch is bigger than DIS_PAN,

			//clear the flag PressAndTap.

			if(check_event(XX1,XX2,YY1,YY2,50))

				PressAndTap = 0;



			Dx2 = Dx;

			Dy2 = Dy;

/***********************Ÿ×±ŒžõÅD¶W¹L¶ZÂ÷DXY_SKIPªºšâÂI*****************************************/

			if(check_event(Dx1,Dx2,Dy1,Dy2,DXY_SKIP)){

				Dx1 = Dx2;

				Dy1 = Dy2;

				return 0;

			}

/****************************************************************************************/

			if(flag_v = check_event(Dx1,Dx2,Dy1,Dy2,DIS_PINCH)){

				//If the variation in Dx or Dy is bigger than DIS_PINCH,

				//return gesture PinchIn/PinchOut.

				PressAndTap = 0;

				Dx1 = Dx2;

				Dy1 = Dy2;

				XX1 = XX2;

				YY1 = YY2;

				switch(flag_v){

				case 0x1:

				case 0x4:

					gesture = PinchOut;

				break;

				case 0x2:

				case 0x8:

					gesture = PinchIn;

				break;



				default:

					gesture = 0x2f;

				break;

				}

			}else if(flag_d = check_event(XX1,XX2,YY1,YY2,DIS_PAN)){

				//If the variation in Dx or Dy is smaller than DIS_PINCH and the variation in 

				//the coordinate of central point between the two touch is bigger than DIS_PAN,

				//set the flag RightPan/LeftPan

				PressAndTap = 0;

				Dx1 = Dx2;

				Dy1 = Dy2;

				XX1 = XX2;

				YY1 = YY2;

				switch(flag_d){

				case 0x1:

					RightPan = 1;

				break;

				case 0x2:

					LeftPan = 1;

				break;

				case 0x4:

					DownPan = 1;

				break;

				case 0x8:

					UpPan = 1;

				break;


				default:

					gesture = 0x20;

				break;

				}

			}

		}



	}else if(PressAndTap){

	  	//n_touch is changed from 2 to 1 or from 2 to 0, 

		//and the flag PressAndTap is set.

		gesture = PressTap;

		PressAndTap = 0;

		touch_flag[0] = 0;

		touch_flag[1] = 0;

		memset(direction_flag1,0,4);

	}else if(n_touch == 1){

		//Number of touch is 1,

		//clear flags for 2-touch gesture decision.

		touch_flag[1] = 0;

		if(!touch_flag[0]){ 

			//start a new gesture decision flow

			memset(direction_flag1,0,4);



			touch_flag[0] = 1;

			X1 = x;

			Y1 = y;

		}else{

			//Check to see if any 1-Touch event happens(Up/Down/Right/Left).

			X2 = x;

			Y2 = y;



			if(flag_d = check_event(X1,X2,Y1,Y2,DIS_1T)){

				X1 = X2;

				Y1 = Y2;

				

				direction_flag1[flag_count] = flag_d;



				if(circle_flag){

					//If the previous recognized gesture is circle.	

					event_flag = circle_direction_flag<<4 | direction_flag1[0];

					switch(event_flag){

					case 0x14:

					case 0x42:

					case 0x28:

					case 0x81:

						gesture = CWCircle;

					break;

					case 0x24:

					case 0x41:

					case 0x18:

					case 0x82:

						gesture = CCWCircle;

					break;

					default:

						gesture = 0;

					break;

					}

					circle_direction_flag = direction_flag1[0];

					flag_count=0;

					circle_flag = 1;

				}else if(flag_count == 0){

					flag_count++;

					

				}else if(flag_count <3){

				

					if(direction_flag1[flag_count] != direction_flag1[flag_count-1]){

						flag_count++;

					}

						

				}else if(direction_flag1[flag_count] != direction_flag1[flag_count-1]){

					event_flag = direction_flag1[3] | direction_flag1[2]<<4 | direction_flag1[1] << 8 | direction_flag1[0]<<12;

					switch(event_flag){

					case 0x1428:

					case 0x4281:

					case 0x2814:

					case 0x8142:

						circle_direction_flag = direction_flag1[3];

						gesture = CWCircle;



						flag_count=0;

						circle_flag = 1;

						memset(direction_flag1,0,4);



					break;



					case 0x2418:

					case 0x4182:

					case 0x1824:

					case 0x8241:

						circle_direction_flag = direction_flag1[3];

						gesture = CCWCircle;



						flag_count=0;

						circle_flag = 1;

						memset(direction_flag1,0,4);



					break;

					default:

						gesture = 0;

					break;

					}//switch



				}//if(direction_flag1[3]!=direction_flag1[2])

				

			}

		}

	}else if(n_touch == 0){

		

		if(circle_flag){

			circle_flag=0;

			touch_flag[0] = 0;

			flag_count=0;

			memset(direction_flag1,0,4);

		}

		

		if(direction_flag1[2]){

   			event_flag = direction_flag1[2] | direction_flag1[1] << 4 | direction_flag1[0]<<8;

			switch(event_flag){

			

			case 0x144:

			case 0x142:

			case 0x422:

			case 0x428:

			case 0x288:

			case 0x281:

			case 0x811:

			case 0x814:

				gesture = RArc;

			break;

			

			case 0x244:

			case 0x241:

			case 0x411:

			case 0x418:

			case 0x188:

			case 0x182:

			case 0x822:

			case 0x824:

				gesture = LArc;

			break;



			default:

				gesture = 0;

			break;

			}

		}else if(direction_flag1[1]){

			event_flag = direction_flag1[1] | direction_flag1[0] << 4;

			switch(event_flag){

			case 0x11:

				gesture = RHorizontal;

			break;



			case 0x22:

				gesture = LHorizontal;

			break;

						

			case 0x88:

				gesture = UVertical;

			break;

		

			case 0x44:

				gesture = DVertical;

			break;

		

			case 0x14:

			case 0x42:

			case 0x28:

			case 0x81:

				gesture = RArc;

			break;

		

			case 0x24:

			case 0x41:

			case 0x18:

			case 0x82:

				gesture = LArc;

			break;

			

			default:

				gesture = 0;

			break;

			}

			

		}else if(direction_flag1[0]){

			switch(direction_flag1[0]){

			case 1:

				gesture = RHorizontal;

			break;

			case 2:

				gesture = LHorizontal;

			break;

			case 4:

				gesture = DVertical;

			break;

			case 8:

				gesture = UVertical;

			break;

			}

		}else if(RightPan){

			gesture = RPan;

			RightPan = 0;

		}else if(LeftPan){

			gesture = LPan;

			LeftPan = 0;



		}else if(UpPan){

			gesture = UPan;

			UpPan = 0;

		}else if(DownPan){

			gesture = DPan;

			DownPan = 0;



		}else if(touch_flag[0] && !check_event(X1,X2,Y1,Y2,50)){

			gesture = Tap;

		}

	

		flag_count=0;

		touch_flag[0] = 0;

		touch_flag[1] = 0;

		memset(direction_flag1,0,4);

	}



	// if there is no gesture, we dont clear old one

/*

	if(gesture != 0 && ts.ges_status != GESVALUE_CLOSE){

		ts.GesNo = gesture;

		ts.ges_status = GESVALUE_HAVE;

		wake_up_interruptible(&(ts.wq));		

	}

*/

	return gesture;

}

#endif



static struct workqueue_struct *queue = NULL;

//static struct work_struct work;

static struct delayed_work work;



static int FirstTC = 0,OneTCountAfter2 = 0,TWOTouchFlag = 0;

static int two_touch_count = 0, pre_dx = 0, pre_dy = 0;



static void uor_read_data(unsigned short *X, unsigned short *Y, 

	unsigned short *DX, unsigned short *DY)

{



	VUINT8 EpBuf[16];

	unsigned short	x, y;

	unsigned short	Dx, Dy;



	memset(EpBuf, 0, sizeof(EpBuf));

	UOR_IICRead(MSRX_1T, EpBuf, 2);

	x= EpBuf[0]; 

	x <<=4;

	x |= (EpBuf[1]>>4);

	

	UOR_IICRead(MSRY_1T,  EpBuf, 2);

	y = EpBuf[0]; 

	y <<=4;

	y |= (EpBuf[1]>>4);

	

	UOR_IICRead(MSRX_2T,  (EpBuf), 3);

	Dx = EpBuf[2];

	

	UOR_IICRead(MSRY_2T,  (EpBuf), 3);

	Dy = EpBuf[2];	

	

	

	*X = x;

	*Y = y;

	*DX = Dx;

	*DY = Dy;
}



static void uor_read_loop(struct work_struct *data)

{

	unsigned short  x, y;

	unsigned short x1,x2,y1,y2;
	unsigned short  Dx, Dy, z1, z2;

	unsigned short Dx1,Dx2,Dy1,Dy2;
	unsigned int	R_touch;

	unsigned int Rt;

	unsigned int nTouch = 0;	

    VUINT8 EpBuf[16];

		int irqno = GPIOPortE_Pin3;
	int mask;


//	printk("%s:%d\n",__FUNCTION__,__LINE__);	


	//printk("uor_read_loop %s:%d\n",__FUNCTION__,__LINE__);	

	while(1){

			


			

	//		printk(KERN_ERR "%s:before filter (x,y)=(%d,%d) (dx,dy)=(%d,%d) n_touch %d, R_touch %d, (z1,z2)=(%d,%d) !!!\n",__FUNCTION__, x, y, Dx, Dy, nTouch, R_touch, z1, z2);







#ifdef FILTER_FUNC

			    uor_read_data(&x, &y, &Dx, &Dy);
					//first point

					xFilter[ts.count] = x;

					yFilter[ts.count] = y;	

					DxFilter[ts.count] = Dx;

					DyFilter[ts.count] = Dy;

					//printk(KERN_ERR "Data before filter:#%d (x,y)=(%d,%d) (dx,dy)=(%d,%d) !!!\n",ts.count , x, y, Dx, Dy);

					ts.count ++;

					//udelay(PERIOD_PER_FILTER);//Per Read Point Delay
					



					while(ts.count < NFilt)//collect other point

					{

						uor_read_data(&x, &y, &Dx, &Dy);

						xFilter[ts.count] = x;

						yFilter[ts.count] = y;	

						DxFilter[ts.count] = Dx;

						DyFilter[ts.count] = Dy;

						//printk(KERN_ERR "Data before filter:#%d (x,y)=(%d,%d) (dx,dy)=(%d,%d) !!!\n",ts.count , x, y, Dx, Dy);

						ts.count ++;

						//udelay(PERIOD_PER_FILTER);//Per Read Point Delay
						

					}

				

					if(!XYFilter(xFilter, yFilter, NFilt,NDrop)){ // no correct point	

						printk(KERN_ERR "%s: X Y filter error !!!\n",__FUNCTION__);

					}

					ts.xp =xFilter[0];

					ts.yp =yFilter[0];

					

					

					if(!XYFilter(DxFilter, DyFilter, NFilt,NDrop)){ // no correct point

					    printk(KERN_ERR "%s: DX DY filter error !!!\n",__FUNCTION__);

					}

					Dx = DxFilter[0];

					Dy = DyFilter[0];

					ts.count = 0;



					//printk(KERN_ERR "Data after filter: (x,y)=(%d,%d) (dx,dy)=(%d,%d) !!!\n", ts.xp, ts.yp, Dx, Dy);

				

#else // no filter

                            	memset(EpBuf, 0, sizeof(EpBuf));

	                            UOR_IICRead(MSRX_1T, EpBuf, 2);
	                            x= EpBuf[0]; 
	                            x<<=4;
	                            x|= (EpBuf[1]>>4);
					UOR_IICRead(MSRX_1T, EpBuf, 2);
	                            x2= EpBuf[0]; 
	                            x2<<=4;
	                            x2|= (EpBuf[1]>>4);
	
	                            UOR_IICRead(MSRY_1T,  EpBuf, 2);
	                            y1= EpBuf[0]; 
	                            y1<<=4;
	                            y1 |= (EpBuf[1]>>4);

				       UOR_IICRead(MSRY_1T,  EpBuf, 2);
	                            y2= EpBuf[0]; 
	                            y2<<=4;
	                            y2 |= (EpBuf[1]>>4);
	

					if ((x-x2)>100 ||(x2-x)>100)
					   ts.xp = 4095;
					else

				          ts.xp = x;

					if ((y1-y2)>100 ||(y2-y1)>100)
					   ts.yp = 4095;
					else

				          ts.yp = y1;

//printk(KERN_ERR "no filter: (x,y)=(%d,%d)!!!\n", ts.xp, ts.yp);
#endif

	memset(EpBuf, 0, sizeof(EpBuf));				

	//check nTouch again after AVG

    UOR_IICRead(MSRZ1_1T,  EpBuf, 2);

	z1 = EpBuf[0]; 

	z1 <<=4;

	z1 |= (EpBuf[1]>>4);

	

	UOR_IICRead(MSRZ2_1T, EpBuf, 2);

	z2 = EpBuf[0]; 

	z2 <<=4;

	z2 |= (EpBuf[1]>>4);

//printk(KERN_ERR "no filter: (Z1,Z2)=(%d,%d)!!!\n", z1, z2);

			if(z1 ==0)

				z1 =1;//avoid divde by zero

			R_touch =(abs(((z2*x)/z1-x)))/4; //(float)((((float) z2)/((float) z1) -1)*(float)x)/4096;

			Rt =R_touch;



			if(readl(rGPGDAT) & (1 << 4))
//			if(x<100)
				{
			nTouch =  ZERO_TOUCH;
				}
			//printk(KERN_ERR "GPIO zero");}
			else if(  (Rt < R_Threshold2) )
				{
				nTouch =  TWO_TOUCH;
			//printk(KERN_ERR "GPIO two");
				}
			else
				{
				nTouch = ONE_TOUCH;
						//printk(KERN_ERR "GPIO one");
				}



				x = ts.xp;

				y = ts.yp;

//			printk(KERN_ERR "%s:after Avg Filter (x,y)=(%d,%d)  n_touch %d, R_touch %d !!!\n",__FUNCTION__, ts.xp, ts.yp,  nTouch, R_touch);



				//invert for android x-axis

				//ts.xp = 4095 - ts.xp;

			if(nTouch == ONE_TOUCH || nTouch == TWO_TOUCH){					

				if(nTouch == TWO_TOUCH){

                                   UOR_IICRead(MSRX_2T,  (EpBuf), 3);
	                            Dx1 = EpBuf[2];
		                     UOR_IICRead(MSRX_2T,  (EpBuf), 3);
	                            Dx2 = EpBuf[2];
	
	                            UOR_IICRead(MSRY_2T,  (EpBuf), 3);
	                            Dy1 = EpBuf[2];	
				       UOR_IICRead(MSRY_2T,  (EpBuf), 3);
	                            Dy2 = EpBuf[2];	

					
					if ((Dx1-Dx2)>16 ||(Dx2-Dx1)>16)
					   Dx = 256;
					else
				          Dx = (Dx1+Dx2)>>1;
					if ((Dy1-Dy2)>16 ||(Dy2-Dy1)>16)
					   Dy = 256;
					else
				          Dy = (Dy1+Dy2)>>1;

					if((Dx < DX_T && Dy < DY_T) || (Dx>252||Dy>252) || two_touch_count < FIRST_TWO_TOUCH_FILTER){
							//printk(KERN_ERR "%s:filter for first two touch -(x,y)=(%d,%d) (dx,dy)=(%d,%d),count = %d, FIRST_TWO_TOUCH_FILTER = %d  !!!\n",__FUNCTION__, x, y, Dx, Dy,two_touch_count, FIRST_TWO_TOUCH_FILTER);

							two_touch_count++;

							queue_delayed_work(queue, &work, DROP_POINT_DELAY_J);
							goto READ_LOOP_OUT;

					}

					else if( (pre_dx!=0) && (pre_dy!=0) && (Dx - pre_dx > JITTER_THRESHOLD_DXDY || pre_dx - Dx > JITTER_THRESHOLD_DXDY || pre_dy - Dy > JITTER_THRESHOLD_DXDY || Dy - pre_dy > JITTER_THRESHOLD_DXDY)){//single touch point «e«á®t¶ZJITTER_THRESHOLD «hÂoÂI 

							//printk(KERN_ERR "%s:filter for jitter(dual) --(pre_dx,pre_dy)=(%d,%d) ,(dx,dy)=(%d,%d) , JITTER_THRESHOLD_DXDY = %d !!!\n",__FUNCTION__, pre_dx, pre_dy , Dx, Dy, JITTER_THRESHOLD_DXDY);

							queue_delayed_work(queue, &work, DROP_POINT_DELAY_J);
							goto READ_LOOP_OUT;

					}

					else{

							//report x,y,pressure,dx,dy to Linux/Android

							//printk(KERN_ERR "%s:raw data for dual touch-- (x,y)=(%d,%d) (dx,dy)=(%d,%d)  !!!\n",__FUNCTION__, x, y, Dx, Dy);

														

					 if ( (pre_dx!=0) && (pre_dy!=0) && ((Dx - pre_dx) <12 && (pre_dx-Dx) <12 && (Dy - pre_dy )<12 && (pre_dy-Dy )<12)){
                		        	Dx = pre_dx;
                		        	Dy = pre_dy;
                		        }
							int	dx_coord = 0 ;

							int	dy_coord = 0 ;

							int x1,y1,x2,y2;

							int xp,yp;

							if((Dx - DX_T) < 0)

								dx_coord = 0 ;

							else

								dx_coord = ((Dx - 40) & 0xff8) * 4;


							if((Dy - DY_T) < 0)

								dy_coord = 0 ;

							else

								dy_coord = ((Dy - 40) & 0xff8) * 4;


							

							//compute for two touch coordinate p1=(x1,y1) p2=(x2,y2)

							

							 x1 = 2048 - dx_coord;

							 y1 = 2048 - dy_coord;

							 x2 = 2048 + dx_coord;

							 y2 = 2048 + dy_coord;

							

							//report dual touch p1 ,p2 to Linux/Android

							

							//printk(KERN_ERR "%s:report dual touch for android (x1,y1)=(%d,%d) (x2,y2)=(%d,%d)  !!!\n",__FUNCTION__, x1, y1, x2, y2);

					#if 1 		

						input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));

							//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 500+press);

							input_report_abs(ts.dev, ABS_MT_POSITION_X, 4095-x1 );

							input_report_abs(ts.dev, ABS_MT_POSITION_Y, 4095-y1 );

							input_mt_sync(ts.dev);

	

							input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));

							//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 600+press);

							input_report_abs(ts.dev, ABS_MT_POSITION_X, 4095-x2 );

							input_report_abs(ts.dev, ABS_MT_POSITION_Y, 4095-y2 );

							input_mt_sync(ts.dev);

        

        					input_sync(ts.dev);

		

					#else//this part is for Calibrate	

							TouchPanelCalibrateAPoint(x1,y1,&xp,&yp);

							xp/=4;

							yp/=4;

							input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));

							//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 500+press);

							input_report_abs(ts.dev, ABS_MT_POSITION_X, xp );

							input_report_abs(ts.dev, ABS_MT_POSITION_Y, yp );

							input_mt_sync(ts.dev);

	

							TouchPanelCalibrateAPoint(x2,y2,&xp,&yp);

							xp/=4;

							yp/=4;

//							printk(KERN_ERR "%s:report dual touch for android (x_coord,y_coord)=(%d,%d) (dx_coord,dy_coord)=(%d,%d)  !!!\n",__FUNCTION__, ts.xp, ts.yp, dx_coord, dy_coord);

							input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));

							//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 600+press);

							input_report_abs(ts.dev, ABS_MT_POSITION_X, xp );

							input_report_abs(ts.dev, ABS_MT_POSITION_Y, yp );

							input_mt_sync(ts.dev);

        

        					input_sync(ts.dev);

							#endif 

							TWOTouchFlag = 1;

							OneTCountAfter2 = 0;

							pre_dx = Dx;

							pre_dy = Dy;

							queue_delayed_work(queue, &work, READ_DATA_DELAY_J);
							goto READ_LOOP_OUT;
					}

				}

				else if(nTouch == ONE_TOUCH){

					//ÂoÂI: «eX­Ó³æÂI&& šâ«ü«áY­Ó³æÂI

					if((TWOTouchFlag == 1) && (OneTCountAfter2 < ONETOUCHCountAfter2)){//¥á±ŒÂùÂI«áªºONETOUCHCountAfter2(Y)µ§³æÂI

							//printk(KERN_ERR "%s:filter after two touch -- (x,y)=(%d,%d) ,OneTCountAfter2 = %d, ONETOUCHCountAfter2 = %d !!!\n",__FUNCTION__, x, y, OneTCountAfter2, ONETOUCHCountAfter2);

							OneTCountAfter2++;

							queue_delayed_work(queue, &work, DROP_POINT_DELAY_J);
							goto READ_LOOP_OUT;

					}		

					else if((ts.xp>4000||ts.yp>4000)||(TWOTouchFlag == 0) && (FirstTC < FIRSTTOUCHCOUNT)){//¥á±Œ«eFIRSTTOUCHCOUNT(X)µ§³æÂI
							//printk(KERN_ERR "%s:filter before single touch -- (x,y)=(%d,%d) ,FirstTC = %d, FIRSTTOUCHCOUNT = %d !!!\n",__FUNCTION__, x, y, FirstTC, FIRSTTOUCHCOUNT);

							FirstTC++;

							queue_delayed_work(queue, &work, DROP_POINT_DELAY_J);
							goto READ_LOOP_OUT;

					}

					else if( (ts.pX!=0) && (ts.pY!=0) && (ts.xp - ts.pX > JITTER_THRESHOLD || ts.pX - ts.xp > JITTER_THRESHOLD || ts.pY - ts.yp > JITTER_THRESHOLD || ts.yp - ts.pY > JITTER_THRESHOLD)){//single touch point «e«á®t¶ZJITTER_THRESHOLD «hÂoÂI 

							//printk(KERN_ERR "%s:filter for jitter -- (px,py)=(%d,%d) ,(x,y)=(%d,%d) , JITTER_THRESHOLD = %d !!!\n",__FUNCTION__, ts.pX, ts.pY ,x, y, JITTER_THRESHOLD);

							queue_delayed_work(queue, &work, DROP_POINT_DELAY_J);
							goto READ_LOOP_OUT;

					}

					else{

                                           if ( (ts.pX!=0) && (ts.pY!=0) && ((ts.xp - ts.pX) <30 && (ts.pX-ts.xp) <30 && (ts.yp - ts.pY )<30 && (ts.pY-ts.yp )<30)){
                	                	     ts.xp = ts.pX;
                	                	     ts.yp = ts.pY;
                	                          }
							printk(KERN_ERR "%s:report single touch-- (x,y)=(%d,%d) !!!\n",__FUNCTION__, ts.xp, ts.yp);

						//report x,y,pressure,size to Linux/Android

							#if 1 

							input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200) );

							//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 300);

							input_report_abs(ts.dev, ABS_MT_POSITION_X, 4095-ts.xp);

							input_report_abs(ts.dev, ABS_MT_POSITION_Y, 4095-ts.yp);

							input_mt_sync(ts.dev);

							input_sync(ts.dev);

							#else // single calibrate

							int xp,yp;

							gADPoint.x = (int)ts.xp;

							gADPoint.y = (int)ts.yp;

							TouchPanelCalibrateAPoint((int)ts.xp,(int)ts.yp,&xp,&yp);



							xp/=4;

							yp/=4;


//							printk("xp:%d,yp:%d,x:%d,y:%d\n",ts.xp,ts.yp,xp,yp);

							input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));

							//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 500+press);

							//input_report_abs(ts.dev, ABS_X, xp );

							//input_report_abs(ts.dev, ABS_Y, yp );

							input_report_abs(ts.dev, ABS_MT_POSITION_X, xp );

							input_report_abs(ts.dev, ABS_MT_POSITION_Y, yp );
							
						//printk("11111111111xp:%d,yp:%d,x:%d,y:%d\n",ts.xp,ts.yp,xp,yp);

							input_mt_sync(ts.dev);

							input_sync(ts.dev);

							#endif

							//save previous single touch point

							ts.pX = ts.xp; 

							ts.pY = ts.yp;

							queue_delayed_work(queue, &work, READ_DATA_DELAY_J);
							
							
							goto READ_LOOP_OUT;
					}

				}

		#ifdef  GESTURE_IN_DRIVER

				VUINT8 gesture = 0;

				gesture =  gesture_decision((VUINT8)nTouch, (VUINT16)(ts.xp),  (VUINT16)ts.yp,  (VUINT16)Dx, (VUINT16)Dy);	

				if(gesture){

					SendGestureKey(gesture);

					//printk(KERN_ERR "%s:single gesture %d   !!!\n",__FUNCTION__, gesture);

				}

		#endif

			}

			else if(nTouch == ZERO_TOUCH){

ZERO_TOUCH_PROCESS:

//				printk(KERN_ERR "%s:zero touch (x,y)=(%d,%d) (dx,dy)=(%d,%d) n_touch %d, R_touch %d, (z1,z2)=(%d,%d) !!!\n",__FUNCTION__, x, y, Dx, Dy, nTouch, R_touch, z1, z2);
	        	      // printk("ZERO_TOUCH_PROCESS\n");
				input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 0 );

				//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 0);

				input_mt_sync(ts.dev);

				input_sync(ts.dev);

							

				//reset filter parameters

				FirstTC = 0;

				OneTCountAfter2 = 0;

				TWOTouchFlag = 0;

				two_touch_count = 0;

				ts.xp= 0;

				ts.yp = 0;

				ts.pX = 0;

				ts.pY = 0;

				pre_dx = 0;

				pre_dy = 0;

				

		#ifdef  GESTURE_IN_DRIVER

				VUINT8 gesture = 0;

				gesture =  gesture_decision((VUINT8)nTouch, (VUINT16)(ts.xp),  (VUINT16)ts.yp,  (VUINT16)Dx, (VUINT16)Dy);	

				if(gesture){

					SendGestureKey(gesture);

					//printk(KERN_ERR "%s:single gesture %d   !!!\n",__FUNCTION__, gesture);

				}

		#endif				

				Init_UOR_HW();

				



				//set interrupt to high by software

				//gpio_direction_output(GPIO_00_PIN,1);

				//gpio_direction_input(GPIO_00_PIN);



				//enable_irq(TOUCH_INT_IOPIN);
				if (irqno < 32)
				{
					mask = readl(rSRCPND);
					mask |= 1UL << irqno;
					writel(mask, rSRCPND);
				}
				else if (irqno < 56)
				{
					mask = readl(rSRCPND2);
					mask |= 1UL << (irqno-32);
					writel(mask, rSRCPND2);
				}
				else
				{
					printk("Wrong IRQ Number %d\n", irqno);
				}  
				enable_irq(GPIOPortE_Pin3);



				//goto READ_LOOP_OUT;
				break;

			}

			else{

				printk(KERN_ERR "uor_read_loop(): n_touch state error !!!\n");

			}			

			

		}	

	
READ_LOOP_OUT:
	                 
			       return;
	//	printk("%s: exit ts while loop !!!\n",__FUNCTION__ );		

}



static int uor_isr(int irq,void *dev_id)
{

//int	ret = gpio_get_value(GPIOPortE_Pin3);

 	printk(KERN_ERR "uor.c interrupt!! value %d\n");

// printk(KERN_ERR "uor.c interrupt!! value \n");

	disable_irq_nosync(GPIOPortE_Pin3);

	queue_delayed_work(queue, &work, 0);

    return IRQ_HANDLED;
	

}



static int uor_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)

{

	//printk( "uor.c: uor_i2c_probe()\n");

  //  return i2c_probe(adapter, &addr_data, uor_i2c_detect);
     int	ret;

	//int	gpio=0;

	struct input_dev *	input_device;


//	printk("%s:==================== %d ================\n", __func__, __LINE__);

//  gADPoint.x = 0;

//	gADPoint.y = 0;

	memset(&ts, 0, sizeof(struct uor_touch_screen_struct));//init data struct ts

	i2c_set_clientdata(client, &ts);
	ts.client = client;

	input_device = input_allocate_device();

	if (!input_device) {

			printk(KERN_ERR "Unable to allocate the input device !!\n");

			return -ENOMEM;

	}

	input_device->name = "UOR-touch";



	

	ts.dev = input_device;

	__set_bit(EV_ABS, ts.dev->evbit);

	__set_bit(EV_SYN, ts.dev->evbit);



	input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 1000, 0, 0);

	//input_set_abs_params(codec_ts_input, ABS_MT_WIDTH_MAJOR, 0, 1000, 0, 0);

	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 4095, 0, 0);

	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 4095, 0, 0);

	

	ret = input_register_device(ts.dev);

	if (ret) {

		printk(KERN_ERR "%s: unabled to register input device, ret = %d\n",

			__FUNCTION__, ret);

		return ret;

	}


	printk(KERN_ERR "uor.c: before UOR init !\n");

/*	ret = Init_UOR_HW();

	 if(ret < 0)

		 printk(KERN_ERR "uor.c: Init_UOR_HW() fail in uor_init()!\n");*/



	//printk(KERN_ERR "uor.c: before GPIO setup !\n");

//       rockchip_mux_api_set(TOUCH_INT_IOMUX_PINNAME,TOUCH_INT_IOMUX_PINDIR);

//       GPIOSetPinDirection(GPIOPortE_Pin3, GPIO_IN);

//       GPIOPullUpDown(GPIOPortE_Pin3, GPIOPullUp);

	set_irq_type(GPIOPortE_Pin3, IRQF_TRIGGER_FALLING);

	if(request_irq(GPIOPortE_Pin3, uor_isr, IRQF_DISABLED, "uor6153-ts", ts.dev)){

		printk(KERN_ERR "uor.c: Could not allocate GPIO intrrupt for touch screen !!!\n");

		free_irq(GPIOPortE_Pin3,NULL);

		return -EIO;	

	}


	//printk(KERN_ERR "uor.c: before UOR workqueue !\n");

	queue = create_singlethread_workqueue("uor-touch-screen-read-loop");
	//queue = create_rt_workqueue("uor-touch-screen-read-loop");

	INIT_DELAYED_WORK(&work, uor_read_loop);

	printk("%s:==================== %d ================\n", __func__, __LINE__);

    //INIT_WORK(&work, uor_read_loop);
	//printk("[UOR]:System111111111111111111111 HZ = %d\n", HZ);


   return 0;

}


static int __init uor_init(void)

{

	//printk(KERN_INFO "ABCD\n");

     int	ret;

	//int	gpio=0;

	struct input_dev *	input_device;

	//printk(KERN_ERR "uor.c: uor_init() !\n");
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	printk(KERN_INFO"%s: UOR6153 TouchScreen driver: init\n", __func__);

	ret = i2c_add_driver(&uor_i2c_driver);

	if(ret < 0)
		printk(KERN_ERR "%s uor.c: i2c_add_driver() fail in uor_init()!\n",__func__);	

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = normal_i2c[0]/*0x48*/;  
	strlcpy(info.type, UOR6153_I2C_NAME, I2C_NAME_SIZE);



	adapter = i2c_get_adapter(2);
	if (!adapter) {
		printk(KERN_ERR "%s: get_adapter error!\n", __func__);
	}
	client = i2c_new_device(adapter, &info);
		
    i2c_put_adapter(adapter);
    if (!client) {
        printk("%s : can't add i2c device at 0x%x\n",
            __func__,(unsigned int)info.addr);
        return ret;
    }

//	printk("%s:==================== %d ================\n", __func__, __LINE__);


	return ret;

}



static void __exit uor_exit(void)

{

	free_irq(GPIOPortE_Pin3,NULL);

	i2c_del_driver(&uor_i2c_driver);  

}





module_init(uor_init);

module_exit(uor_exit);



MODULE_DESCRIPTION("UOR Touchscreen driver");

MODULE_AUTHOR("Ming-Wei Chang <mingwei@uutek.com.tw>");

MODULE_LICENSE("GPL");





