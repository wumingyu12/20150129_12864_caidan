#include "Finger_Modle.h"
#include <reg52.h>
#include "LCD12864.h"

#define FALSE 0
#define TURE  1
#define MAX_NUMBER    30 //缓存数组的最大值
#define _Nop()  _nop_()

unsigned char 		g_FifoNumber=0; //返回的fifo的字节数，如果你看到手册的返回包是12个字节那么这里的值就是11
unsigned char     g_FIFO[MAX_NUMBER+1]={0};//返回数据的缓存器
unsigned int      g_SearchNumber=0;//搜索匹配得到的匹配页码

//默认为识别模式 
//bit g_modeflag= 0 ,  g_clearallflag=0, g_changeflag=0;

//////////////////////////////////////常用指令定义/////////////////////////////

//Verify  Password   ：验证设备握手口令
unsigned char code VPWD[16]={16,0x01,0Xff,0xff,0xff,0xff, 0x01,0,7,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};	 //回送12个
//设置设备握手口令
unsigned char code STWD[16]={16,0X01 ,0Xff,0xff,0xff,0xff, 0x01,0,7,0x12,0x00,0x00,0x00,0x00,0x00,0x1a};	 //回送12个
//GetImage           ：探测手指并从传感器上读入图像
unsigned char code PS_GetImage[12]={12, 0X01 ,0Xff,0xff,0xff,0xff, 0x01, 0,3,1,0x00,0x05};	//回送12个
//Gen Templet1        ：根据原始图像生成指纹特征1，放在buffer1中
unsigned char code PS_GenChar1[14]={13,0X01 ,0Xff,0xff,0xff,0xff,0x01,0,4,2,1,0x00,0x08};	//回送12个
//Gen Templet2        ：根据原始图像buffer生成指纹特征2
unsigned char code PS_GenChar2[13]={13,0X01 ,0Xff,0xff,0xff,0xff,0x01,0,4,2,2,0x00,0x09}; //回送12个		
//Search Finger      ：以CharBufferA或CharBufferB中的特征文件搜索整个或部分指纹库,这里用了buffer1
unsigned char code PS_Search1[18]={17,  0X01 ,0Xff,0xff,0xff,0xff, 0x01,   0,8, 4,1,0,0,    0,0x65,  0x00,0x73};	//回送16个
//Merge Templet      ;将CharBufferA与CharBufferB中的特征文件合并生成模板，结果存于ModelBuffer。
unsigned char code PS_RegModel[14]={12,  0X01 ,0Xff,0xff,0xff,0xff, 0x01,  0,3,5 , 0x00,0x09};//回送12个	
//Store Templet      ：将ModelBuffer中的文件储存到flash指纹库中
unsigned char code PS_StoreChar[16]={15,  0X01 ,0Xff,0xff,0xff,0xff, 0x01,  0,6,6,2,     0x00,0x00,     0x00,0x0f}; //回送12个,实际发命令时要指定存储的指纹库号
//Read Note
unsigned char code RDNT[14]={13,0X01 ,0Xff,0xff,0xff,0xff, 0x01, 0,4,0x19,  0, 0x00,0x1e};
//Clear Note
unsigned char code DENT[46]={45,0X01 ,0Xff,0xff,0xff,0xff, 0x01, 0,36,0x18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x00,0x3d};
//DEL one templet
unsigned char code DELE_one[16]={16, 0X01 ,0Xff,0xff,0xff,0xff, 0x01,   0,7,  0x0c,    0x00,0x00, 0,1, 0x00,0x15};
//DEL templet      ;清空指纹库
unsigned char code PS_Empty[12]={12,0X01 ,0Xff,0xff,0xff,0xff, 0x01, 0,3,  0x0d,0x00,0x11};
//读有效模版个数,第一个数是长度，第二个是空的，不知用来干嘛也不发送，但没有好像就是不行
unsigned char code PS_ValidTempleteNum[12]={12,0X01 ,0Xff,0xff,0xff,0xff, 0x01,0,3,0x1d,0x00,0x21};

void delay1ms(unsigned char count){
	unsigned char i=0;
	unsigned int  k=0; 
	for(k=0;k<count;k++) 
	{
		for(i=0;i<160;i++)
		{
		   ;
		} 
	} 
}

void FM_Init(){//初始化话主要是串口
  //串口初始化 
    SCON=0x50;   //UART方式1:8位UART;   REN=1:允许接收 
    PCON=0x00;   //SMOD=0:波特率不加倍 
    TMOD=0x21;   //T1方式2,用于UART波特率，定时器1用来波特率
    TH1=0xFD; //波特率高位
    TL1=0xFD;   //UART波特率设置:9600
		TR1=1; //开定时器1

	  TI=0;//清发送中断标志
	  RI=0;//清接收中断标志
}

void TxdByte(unsigned char dat){//串口发送信息,通过查询方式发送一个字符
    TI = 0;		 //让TI＝0，发送完成中断设置为0
    SBUF = dat;	 //读入数据
    while(!TI){};	 //等待发送完毕，当中断为1（完成中断）就不循环
    TI = 0;		 //清零
}

//command的延时也很重要要不你事实上发送的命令是有效响应了但你延时太短导致你以为没有正常返回,用oxef
bit Command(unsigned char *p,unsigned char COM_TIMEOUT){//命令解析,给模块发送一个命令,后面的是最多循环次数,返回的值会保留在g_FIFO中
	unsigned char clk_high=0,clk_low=0;//在do循环中会改变，高位为低位的进位，clk_high高于宏定义TIMEOUT就退出do，就是超时
	unsigned char count=0,tmpdat=0,temp=0,i=0,package=0,flag=0,checksum=0; 		
	bit result=0, start=0,stop=0;
	TxdByte(0xef);//数据包包头识别码
	TxdByte(0x01);//数据包包头识别码
	i=*p;         //数组的第“0”个元素、里面存放了本数组的长度，把这个长度给变量i，方便进行操作
	p++; 
	p++;//跳过第一个0x01,不知到这个有什么用但去掉又出问题了
	for (count=i-1; count!=1;count--)  //Sent command String
	{
		temp=*p++;//取第个“1”个元素的内容，然后发送 
		TxdByte(temp);//将数据发送出去
	} 
	//dprintf(1,0," aa ");	
	result=TURE;//发送完成,结果为真 (真为1)，先假设为真   	
	g_FifoNumber=0;
	for (count=MAX_NUMBER+1; count!=0; count--)//清空所有FIFO[]数组里面的内容，写入0X00
	{
	  g_FIFO[count-1]=0x00; 
	}	  
	if (result)   //假设为真那第一次一定会进入这个if
	{
			//dprintf(1,0," aa ");
		result=FALSE;
		start =FALSE;
		stop  =FALSE;
		count=0;
		clk_low=0;
		clk_high=0;	//清零CL0计数
		
		do /////////////////////////////do的内容////////////////////////////////
		{	//后面有break跳出
			restart0:
			//dprintf(1,0," aa ");
			clk_low++;//每循环一次就加1
			if(clk_low==0xff)//低位计数满了
			{
				clk_high++;//进位
				clk_low=0x00;//清零
			}
			if (RI==1)//如果接收到数据，如果发生了串口接收完成中断，就执行下面的，否则就do空循环,接收到1个字节
			{ 				
				tmpdat=SBUF;//先把接收到的数据放到tmpdat中
				RI=0;//置位，等下一个字节的接收
				if ((tmpdat==0xef)&&(start==FALSE))//这个数据为第一个传回来的数据，也就是“指令应答”的第一个字节
				{ 
					count=0;
					g_FIFO[0]=tmpdat;//读入第一个应答字节(0XEF)，存在第“0”个元素中    
					flag=1;	
					goto 
					restart0;//可以用中断方式进行			
				}

				if(flag==1)//第一个字节已经回来，所以flag==1成立
				{  
					if(tmpdat!=0x01)  //接收数据错误，将重新从缓冲区接收数据，如果接收到不是0xef01
					{  	
						flag=0;//接收应答失败
						result=FALSE;
						start =FALSE;
						stop=FALSE;
						count=0;
						goto 
						restart0;					
					}
					//如果成功接收到0xef01，可以开始接收数据
					flag=2;//flag=2;表示应答成功，可以开始接收数据了
					count++;//现在count=1;
					g_FIFO[count]=tmpdat;//读入第二个应答字节（0X01），存在第“1”个元素中    
					start=TURE;	//应答成功可以开始接收数据
					goto 
					restart0;	
				}
										  
				if((flag==2)&&(start==TURE))//flag=2;表示应答成功，可以开始接收数据了
				{	   	  					 
					count++; //数据元素下标＋＋
					g_FIFO[count]=tmpdat;//存入数据
					if(count>=6)
					{
						checksum=g_FIFO[count]+checksum; //计算校验和
					}

					if(count==8)
					{ 
						package = g_FIFO[7]*256 + g_FIFO[8];	//计算包长度							
						stop= TURE;
					}

					if(stop)
					{						
						if(count == package+8)
						{
							checksum=checksum-g_FIFO[count-1] - g_FIFO[count];
							if(checksum != (g_FIFO[count]&0xff)) 
							result=FALSE; //校验失败,置结果标志为0							
							else 
							result=TURE;
							flag=0;
							break;//退出循环
						} 
					}
				}
			}
		}/////////////////////////////do-while的内容----------------结束////////////////////////////////
		 //如果在超时范围内，接收的数据在最大数组数内，就一直do循环  
		while ((clk_high<COM_TIMEOUT)&&(count<=MAX_NUMBER)); //由定时器以及最大接收数据来控制，保证不会在此一直循环
		  
		g_FifoNumber=count;	//保存接收到的数据个数
	}
	return (result);
}


bit FM_VefPSW(void){//验证设备握手口令,成功返回1     
 	unsigned char  count=0;
	//dprintf(1,0," aa ");
	while (1)
   	{
     	if(Command(VPWD,0xef) && (g_FifoNumber==11) && (g_FIFO[9]==0x00))  //注意后面的command()参数COM_TIMEOUT的值不要过大?
		{
		  return (1) ;
			
		}	
     	count++;
   	  	if (count>=2)//如果不成功，再验证一次，如果两次不成功，返回失败
	    {  
	        return(0);   
	 	}
		
	}
}
bit FM_Empty(void) //清空指纹库   
{				
     delay1ms(200);
	 if(Command(PS_Empty,0xef)&&(g_FifoNumber==11)&&(g_FIFO[9]==0x00)){
			return 1;
	 }else{ //清空指纹库  
			return 0;
		}
}

bit FM_CreatChar_buffer(unsigned char Bufid){//根据图像生成特征码，保存到指定的charbufid中，可以为1，2
	  unsigned char i=0,j=0;
	  delay1ms(20);//这是为了上一条命令，保存图像预留一些执行时间，这个值也十分重要不要乱改
	  for(i=0;i<0x2f;i++){//一定要多次，因为你运行时可能模块还不会响应你，循环256次,这个循环值非常重要，太大你会有死循环的感觉太小有不能保证执行好命令
			for(j=0;j<0xff;j++){
		if(Bufid==1){//如果录入的图像要保存到buf1	   			
      if(Command(PS_GenChar1,0xef) && (g_FifoNumber==11) && (g_FIFO[9]==0x00)){
				return 1;
			}  
		}
		else if(Bufid==2){//如果要保存到buf2
		  	if(Command(PS_GenChar2,0xef) && (g_FifoNumber==11) && (g_FIFO[9]==0x00))  {				
				return 1;
			}  			
		}
		else//输入了其他的bufid
		{
		  return 0;//失败
		}
	}
	}
	return 0;
}

bit FM_Save_model(unsigned char ID)//保存指纹模版buffer里面的东西到指定的id号里面
{
	 unsigned char i=0;

	 //现在开始进行存储指纹模板的操作
     for (i=0;i<16;i++)	//保存指纹信息
 	 {
		g_FIFO[i]=PS_StoreChar[i];
	 }  
     g_FIFO[12]=ID;           //把指纹模板存放的PAGE_ID也就是FLASH的位置
     g_FIFO[14]=g_FIFO[14]+ID;	//校验和
     if (Command(g_FIFO,0xef)==1)//不成功返回0	//此处进行存放指纹模板的命
	 {
	   return(1);
	 }

	 return (0) ; //不成功返回0
}

unsigned char FM_ValidTempleteNum(bit addr){//返回可以用的模版数，如果addr=0,就返回低位的字节，如果为1，就返回高位字节，一般低位就可以了，高位都为0
	if(Command(PS_ValidTempleteNum,0xef) && (g_FifoNumber==13) && (g_FIFO[9]==0x00)){
		if(addr==0){return g_FIFO[11];}//返回低位值
		else{return g_FIFO[10];}//返回高位值
	}
	else{
		return 0xff;//错误
	}
}

bit FM_RegModel_Charbuffer(){//根据两个特征码buffer生成一个指纹模版
	if (  Command(PS_RegModel,0xef)&& (g_FifoNumber==11) && (g_FIFO[9]==0x00) )
	{
		return 1;//成功
	}else{
		return 0;
	}
}

bit FM_GetImage(void){//探测手指并读出图像到图像寄存器中
	unsigned char i=0;
	for(i=0;i<40;i++){
		if(Command(PS_GetImage,0xef)&&(g_FifoNumber==11)&&(g_FIFO[9]==0x00)){//读到图像到imgbuffer成功
			return 1;
		}else{
			;
		}
	}
	return 0;//40次后还是不成功
}

unsigned char FM_Search(void)//搜索用户,并返回匹配的指纹号，如果是0xff就是错误的 
{
 	unsigned char matchnum=0,i=0;
	bit ok1=0,ok2=0;
  	while (i<2)//别太多会很长
    {
			ok1=FM_GetImage();//获取图像
			ok2=FM_CreatChar_buffer(1);//将图像1生成特征码放到buffer1中
     	if ( ok1&&ok2 ) //
      {
       		matchnum=FM_Searchfinger1();//进行指纹比对，如果搜索到，返回搜索到的指纹序号
       		if(!(matchnum==0xff))//如果返回不是0xff，就是正常搜索到了指纹号
       		{
       			return matchnum; 
       		}else {
						; //如果搜索失败什么都不干进入下次循环
					}     
      }
			i++;	
    }
   return 255;//最后还是255
}

unsigned char FM_Searchfinger1(void)//搜索指纹(发送搜索命令、以及根据返回值确定是否存在),返回匹配号,用buffer1        
{		
   	if(Command(PS_Search1,0xef) && (g_FifoNumber==15) && (g_FIFO[9]==0x00) )  
    {
		//SearchNumber=FIFO[10]*0x100+FIFO[11];//搜索到的页码
		//MatchScore=FIFO[12]*0x100+FIFO[13]   可以在此计算得分，从而进行安全级别设定，本程序忽略
	   	return g_FIFO[11];//只返回低位的
		}else{
       	return 0xff;//代表错误
    }  
}





