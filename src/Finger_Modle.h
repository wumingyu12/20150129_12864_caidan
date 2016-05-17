#ifndef __FINGER_MODEL_H__
#define __FINGER_MODEL_H__
	bit FM_VefPSW(void);//验证设备握手口令,成功返回1 
	void FM_Init();//初始化话主要是串口	
	bit FM_Empty(void);//清空所有指纹
	bit FM_CreatChar_buffer(unsigned char Bufid);
	bit FM_Save_model(unsigned char ID);
	unsigned char FM_ValidTempleteNum(bit addr);//返回可以用的模版数，返回0xff代表错误如果addr=0,就返回低位的字节，如果为1，就返回高位字节，一般低位就可以了，高位都为0
	//bit FM_Enroll_One(unsigned char bufferid);//采集一次指纹，并存储到bufferid中,试了40次
	bit FM_RegModel_Charbuffer();//根据两个特征码buffer生成一个指纹模版
	bit FM_GetImage(void);//探测手指并读出图像到图像寄存器中
	unsigned char FM_Searchfinger1(void);//搜索指纹(发送搜索命令、以及根据返回值确定是否存在),返回匹配号,用buffer1  
	unsigned char FM_Search(void);//读取指纹，搜索用户,并返回匹配的指纹号，如果是0xff就是错误的 
#endif