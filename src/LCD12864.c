#include " LCD12864.h "

unsigned char code AC_TABLE[32]={
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,      //第一行汉字位置
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,      //第二行汉字位置
0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,      //第三行汉字位置
0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,      //第四行汉字位置
};

void SendByte(unsigned char Dbyte)
{
     unsigned char i;
     for(i=0;i<8;i++)
     {
           SCK = 0;
           Dbyte=Dbyte<<1;      //左移一位
           SID = CY;            //移出的位给SID
           SCK = 1;
           SCK = 0;
     }
}

unsigned char ReceiveByte(void)
{
      unsigned char i,temp1,temp2;
     temp1 = 0;
     temp2 = 0;
     for(i=0;i<8;i++)
     {
           temp1=temp1<<1;
           SCK = 0;
           SCK = 1;            
           SCK = 0;
           if(SID) temp1++;
     }
     for(i=0;i<8;i++)
     {
           temp2=temp2<<1;
           SCK = 0;
           SCK = 1;
           SCK = 0;
           if(SID) temp2++;
     }
     return ((0xf0&temp1)+(0x0f&temp2));
}

void CheckBusy(void)//查询液晶是否忙
{
     do   SendByte(0xfc);            //11111,RW(1),RS(0),0
     while(0x80&ReceiveByte());      //BF(.7)=1 Busy
}


void WriteCommand(unsigned char Cbyte )//写命令
{
     CS = 1;
     CheckBusy();
     SendByte(0xf8);            //11111,RW(0),RS(0),0
     SendByte(0xf0&Cbyte);      //高四位
     SendByte(0xf0&Cbyte<<4);//低四位(先执行<<)
     CS = 0;
}

void WriteData(unsigned char Dbyte )
{
     CS = 1;
     CheckBusy();
     SendByte(0xfa);            //11111,RW(0),RS(1),0
     SendByte(0xf0&Dbyte);      //高四位
     SendByte(0xf0&Dbyte<<4);//低四位(先执行<<)
     CS = 0;
}

void LcmInit( void )
{
     WriteCommand(0x30);      //8BitMCU,基本指令集合
     //WriteCommand(0x03);      //AC归0,不改变DDRAM内容
     WriteCommand(0x0C);      //显示ON,游标OFF,游标位反白OFF
     WriteCommand(0x01);      //清屏,AC归0
     WriteCommand(0x06);      //写入时,游标右移动
}

void LcmClearTXT( void )
{
      unsigned char i;
     WriteCommand(0x30);      //8BitMCU,基本指令集合
     WriteCommand(0x80);      //AC归起始位
     for(i=0;i<64;i++)
     WriteData(0x20);
}

void LcmClearBMP( void )
{
       unsigned char i,j;
     WriteCommand(0x34);      //8Bit扩充指令集,即使是36H也要写两次
     WriteCommand(0x36);      //绘图ON,基本指令集里面36H不能开绘图
     for(i=0;i<32;i++)        
     {
           WriteCommand(0x80|i);      //行位置
           WriteCommand(0x80);      //列位置
           for(j=0;j<32;j++)            //256/8=32 byte
                WriteData(0);
     }
}

void dprintf(unsigned char row,unsigned char col,unsigned char *puts)
{
     WriteCommand(0x32);//0011-00(普通指令模式)1（图像依然开，要不会一开一关导致在闪）0
     WriteCommand(AC_TABLE[8*row+col]);      //起始位置
     while(*puts != 0)      //判断字符串是否显示完毕
     {
           if(col==8)            //判断换行
           {            //若不判断,则自动从第一行到第三行
                 col=0;
                 row++;
           }
           if(row==4) {row=0;}    //一屏显示完,回到屏左上角
           WriteCommand(AC_TABLE[8*row+col]);
           WriteData(*puts);      //一个汉字要写两次
           puts++;
           WriteData(*puts);
           puts++;
           col++;
     }
}

//这个函数是全屏写一次的实测响应太慢了。用下面的函数，只对特定行写
void fangbai_hang_12864(unsigned char hang){
	unsigned char i,y,ygroup,x,color_0,color_1,color_2,color_3,color_tmp;
	WriteCommand(0x36);//0011-01（扩充指令）1（开图像）0
	switch(hang){
		case 0:{color_0=0xff;color_1=0x00;color_2=0x00;color_3=0x00;break;}//根据行数给每一行赋值颜色
		case 1:{color_0=0x00;color_1=0xff;color_2=0x00;color_3=0x00;break;}
		case 2:{color_0=0x00;color_1=0x00;color_2=0xff;color_3=0x00;break;}
		case 3:{color_0=0x00;color_1=0x00;color_2=0x00;color_3=0xff;break;}
	}
	for(ygroup=0;ygroup<64;ygroup++){//行循环
		if(ygroup<16){x=0x80;y=ygroup+0x80;color_tmp=color_0;}//如果循环到第一行
		if(16<=ygroup&&ygroup<32){x=0x80;y=ygroup+0x80;color_tmp=color_1;}
		if(32<=ygroup&&ygroup<48){x=0x88;y=ygroup-32+0x80;color_tmp=color_2;}//下半屏幕
		if(48<=ygroup&&ygroup<64){x=0x88;y=ygroup-32+0x80;color_tmp=color_3;}
		//if(ygroup<32){x=0x80;y=ygroup+0x80;}
		//else{x=0x88;y=ygroup-32+0x80;}
		WriteCommand(y);//顺序别反了先y
		WriteCommand(x);
		for(i=0;i<16;i++){//连续写入一行的数据，x坐标会自动增加
			WriteData(color_tmp);
		}
	}
	WriteCommand(0x32);//0011-00(普通指令模式)1（图像依然开，要不会一开一关导致在闪）0
}

void Writecolor_hang_12864(unsigned char hang,unsigned char color){
	unsigned char i,x,y,color_tmp,ygroup;//一定要放第一排
	WriteCommand(0x36);//0011-01（扩充指令）1（开图像）0
	if(color==1){color_tmp=0xff;}//填充黑色
		else{color_tmp=0x00;}
	switch(hang){
		case 0:{x=0x80;y=0x80;break;}//
		case 1:{x=0x80,y=0x90;break;}
		case 2:{x=0x88;y=0x80;break;}
		case 3:{x=0x88;y=0x90;break;}
	}
	for(ygroup=0;ygroup<16;ygroup++){//16行循环
			WriteCommand(y+ygroup);//顺序别反了先y
			WriteCommand(x);
			for(i=0;i<16;i++){//连续写入一行的数据，x坐标会自动增加
				WriteData(color_tmp);
			}
	}
	WriteCommand(0x32);//0011-00(普通指令模式)1（图像依然开，要不会一开一关导致在闪）0
}