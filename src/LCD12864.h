#ifndef __LCD12864_H__
#define __LCD12864_H__

#include<reg52.h>

//液晶端口
sbit  CS=P1^0;	 //RS   　模组片选端，高电平有效
sbit SID=P1^1;   //RW 　  串行数据输入端
sbit SCK=P1^2;   //E 　　 串行同步时钟：上升沿时读取SID数据
sbit PSB=P1^6;	 //串口方式  (PSB接低电平）
sbit RST=P1^3;	 //复位端，低电平有效


void SendByte(unsigned char Dbyte);
unsigned char ReceiveByte(void);
void CheckBusy(void);                   //查询液晶是否忙
void WriteCommand(unsigned char Cbyte );//写命令
void WriteData(unsigned char Dbyte );	//写数据
void LcmInit( void );
void LcmClearTXT( void );
void LcmClearBMP( void );
void dprintf(unsigned char row,unsigned char col,unsigned char *puts);//写汉字，col是相对与汉字来说的

void fangbai_hang_12864(unsigned char hang);//在某一行填充一个矩形，与汉字结合就可以反白了，自动异或，这个是全屏画图实测会很慢
void Writecolor_hang_12864(unsigned char hang,unsigned char color);//用这个会快点
#endif
