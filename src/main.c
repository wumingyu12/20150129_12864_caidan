#include <reg52.h>
#include <intrins.h>

#include " LCD12864.h "	//ÀïÃæ¶¨ÒåÁË12864Òº¾§µÄ¶Ë¿Ú½Ó·¨  ÒÔ¼° 12864³ÌĞòÉùÃû
#include "Finger_Modle.h"//Ö¸ÎÆÄ£¿éµÄÍ·ÎÄ¼ş

//=====P3µÄ¸ß4Î»½ÓÁË°´¼ü£¬¿ÉÒÔÔÚkeyscanÀïÃæ¸Ä=============
sfr KEY=0xB0; //¾ÍÊÇP3,²Î¿¼reg52.h
#define G_upkey 0x80 //px.7 °´¼ü£¬ÏòÉÏ¼ü
#define G_downkey 0x40 //px.6 °´¼ü£¬ÏòÏÂ¼ü
#define G_entkey 0x20 //px.5 °´¼ü£¬È·¶¨¼ü
#define G_cankey 0x10//px.4 °´¼ü£¬È¡Ïû¼ü
//===========°´¼ü======================================

sbit relay =P1^4; //¼ÌµçÆ÷Òı½Å
sbit buzzer=P1^5; //·äÃùÆ÷Òı½Å
sbit red=   P2^7;//Â¼ÈëÄ£Ê½Ö¸Ê¾µÆ ÔÚ°å×Ó¿¿½üµ¥Æ¬»ú´¦
sbit green= P2^0;//Ê¶±ğÄ£Ê½Ö¸Ê¾µÆ ÔÚ°å×ÓÔ¶Àëµ¥Æ¬»ú´¦

void delayms(int ms);
void keyscan();//°´¼üÉ¨Ãè

unsigned char Trg;//°´¼ü´¥·¢£¬Ò»×ép3ÀïÃæÖ»»á³öÏÖÒ»´Î
unsigned char Cont;//°´¼ü³¤°´£¬±»keyscanº¯Êı¸Ä±ä

unsigned char KeyFuncIndex=0;    //´æ·Åµ±Ç°µÄ²Ëµ¥Ë÷Òı

void (*KeyFuncPtr)();            //¶¨Òå°´¼ü¹¦ÄÜÖ¸Õë
//¶¨ÒåÀàĞÍ 
typedef struct 
{
   unsigned char KeyStateIndex;   //µ±Ç°µÄ×´Ì¬Ë÷ÒıºÅ
   unsigned char KeyUpState;      //°´ÏÂÏòÉÏ¼üÊ±µÄ×´Ì¬Ë÷ÒıºÅ
	 unsigned char KeyDownState;    //°´ÏÂÏòÏÂ¼üÊ±µÄ×´Ì¬Ë÷ÒıºÅ
   unsigned char KeyEnterState;   //°´ÏÂ»Ø³µ¼üÊ±µÄ×´Ì¬Ë÷ÒıºÅ
   unsigned char KeyCancle;       //°´ÏÂÈ¡ÏûÍË»ØÉÏ¼¶²Ëµ¥
	 void (*CurrentOperate)(void);      //µ±Ç°×´Ì¬Ó¦¸ÃÖ´ĞĞµÄ¹¦ÄÜ²Ù×÷
}  StateTab;

//===========================================================================
//================ÏÂÃæÊÇ¸÷²Ëµ¥²ãµÄÊµÊ©º¯Êı================================
//============================================================================

void Stat00(void){//µÚÒ»¸öÒ³ÃæÏÔÊ¾ÊÇ·ñÎÕÊÖ³É¹¦£¬³É¹¦µÄ»°°´ÈÎÒâ¼ü½øÈëÏÂÒ»¸öÒ³Ãæ£¬²»³É¹¦µÄ»°¾Í²»¿ª¶¨Ê±¼üÅÌÉ¨ÃèÖĞ¶Ï
	unsigned char i;
	ET0=0;//ÏÈ¹Øµô¶¨Ê±Æ÷ÖĞ¶Ï£¬±ÜÃâÔÚ´¦ÀíÊ±ÏìÓ¦ÁË¼üÅÌ£¬½øÈëÁËÏÂÒ»¸ö×´Ì¬£¬¶øÕâÕâ×´Ì¬»¹Ã»ÔËĞĞÍê
	dprintf(0,0,"  ÄÏÄÁÖ¸ÎÆ¿ª¹Ø  ");
	for(i=0;i<6;i++)//¿ªÊ¼ÎÕÊÖ6´Î£¬Èç¹ûÃ»ÓĞÒ»´Î³É¹¦£¬±íÊ¾Ä£¿éÍ¨ĞÅ²»Õı³£¡£Ö»Òª³É¹¦¾ÍÌø³ö´ËÑ­»·
	{
		if(FM_VefPSW())//ÓëÄ£¿éÎÕÊÖÍ¨¹ı£¬ÂÌµÆÁÁÆğ¡£½øÈëÊ¶±ğÄ£Ê½
		{
      dprintf(1,0,"    ÎÕÊÖ³É¹¦    ");
			dprintf(3,0,"°´ÈÎÒâ¼ü½øÈëÏµÍ³");
			//ET0=1;//³É¹¦µÄ»°¿ª¶¨Ê±¼üÅÌÉ¨ÃèÖĞ¶Ï£¬¿ÉÒÔ½øÈëÏÂÒ»¸ö×´Ì¬£¬·ñÔòµÄ»°¾ÍÒ»Ö±ÏÔÊ¾ÏÂÃæµÄÊ§°Ü½çÃæ
			break;//ÍË³öÈ«²¿Ñ­»·£¬Ö´ĞĞÏÂÃæforµÄÓï¾ä£¬²»Í¬Óëcontinue
		}
	    else
		{
			dprintf(1,0,"    ÎÕÊÖÊ§°Ü    ");//Èç¹ûÊ§°Ü¼ÌĞøÖ´ĞĞÏÂÒ»´Îfor¿´»á²»»á³É¹¦
			dprintf(3,0,"Çë¼ì²éÏßÂ·Ä£¿é");
			//break;
		}
	}
	ET0=1;
}
void Stat10(void){ //ui´ò¿ª¿ª¹Ø
	ET0=0;//¹Ø¶¨Ê±Æ÷ÖĞ¶Ï±ÜÃâÏìÓ¦°´¼ü£¬¼ÇµÃ¿ª»ØÀ´
	dprintf(0,0,"    ¿ª¹Ø»úÆ÷   ");//·´°×ÏÔÊ¾
	dprintf(1,0,"    Â¼ÈëÖ¸ÎÆ    ");
	dprintf(2,0,"    Çå¿ÕÖ¸ÎÆ    ");
	dprintf(3,0,"                ");
	Writecolor_hang_12864(0,1);//·´°×µÚÒ»ĞĞ£¬0´ú±íµÚÒ»ĞĞ£¬1´ú±íºÚÉ«
	Writecolor_hang_12864(1,0);
	ET0=1;
}
void Stat11(void){ //uiÂ¼ÈëÖ¸ÎÆ
	ET0=0;
	dprintf(0,0,"    ¿ª¹Ø»úÆ÷    ");//·´°×ÏÔÊ¾
	dprintf(1,0,"    Â¼ÈëÖ¸ÎÆ    ");
	dprintf(2,0,"    Çå¿ÕÖ¸ÎÆ    ");
	dprintf(3,0,"                ");
	Writecolor_hang_12864(0,0);//°ÑÉÏÒ»ĞĞµÄÌî³ä²ìµô
	Writecolor_hang_12864(2,0);//È·±£ÖØÏÂÍùÉÏ°´Ê±ÏÂÃæÄÇĞĞ²Áµô
	Writecolor_hang_12864(1,1);//·´°×µÚ¶şĞĞ
	ET0=1;
}
void Stat12(void){ //uiÇå¿ÕÖ¸ÎÆ
	ET0=0;
	dprintf(0,0,"    ¿ª¹Ø»úÆ÷    ");//·´°×ÏÔÊ¾
	dprintf(1,0,"    Â¼ÈëÖ¸ÎÆ    ");
	dprintf(2,0,"    Çå¿ÕÖ¸ÎÆ    ");
	dprintf(3,0,"                ");
	Writecolor_hang_12864(1,0);
	Writecolor_hang_12864(3,0);
	Writecolor_hang_12864(2,1);
	ET0=1;
}
/*
void Stat13(void){ 
	ET0=0;
	dprintf(0,0,"    ´ò¿ª¿ª¹Ø    ");//·´°×ÏÔÊ¾
	dprintf(1,0,"    Â¼ÈëÖ¸ÎÆ    ");
	dprintf(2,0,"    Çå¿ÕÖ¸ÎÆ    ");
	dprintf(3,0,"    ´ò¿ªµçÔ´    ");
	Writecolor_hang_12864(2,0);
	Writecolor_hang_12864(3,1);
	ET0=1;
}*/

void Stat20(void){//´ò¿ª¿ª¹Ø--È·¶¨
	unsigned char num=0;
	unsigned char strnum[4]={0};//ÊıÁ¿×ª»»Îª×Ö·û´®
	ET0=0;
  LcmClearTXT();//Çå³ıÎÄ±¾
	LcmClearBMP();//Çå³ıÍ¼Ïñ
	if(relay==0){//Èç¹û¼ÌµçÆ÷ÊÇ¿ªÆôµÄ
		dprintf(0,0,"¿ª¹ØÒÑ¾­¿ªÆô    ");//keil bugÓÃÄÚÂë
		dprintf(1,0,"ÊäÈëÖ¸ÎÆ¹Ø±Õ    ");
		dprintf(2,0,"                ");
		dprintf(3,0,"µÈ\xB4\xFDÊäÈë........");
	}else{
		dprintf(0,0,"¿ª¹ØÒÑ¾­¹Ø±Õ    ");//keil bugÓÃÄÚÂë
		dprintf(1,0,"ÊäÈëÖ¸ÎÆ¿ªÆô    ");
		dprintf(2,0,"                ");
		dprintf(3,0,"µÈ\xB4\xFDÊäÈë........");
	}
	num=FM_Search();
	strnum[0]=32;//acsiiÂë¿Õ¸ñ
	strnum[1]= num/100+48;     //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  °Ù
	strnum[2]= (num%100)/10+48;//+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  Ê®
	strnum[3]= num%10+48;      //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  ¸ö?
	if(num==0xff){//²Ù×÷Ê§°Ü
		dprintf(0,0,"¸ÃÖ¸ÎÆ²»´æÔÚ    ");//keil bugÓÃÄÚÂë
		dprintf(1,0,"²Ù×÷Ê§°Ü        ");
		dprintf(2,0,"                ");
		dprintf(3,0,"°´ÈÎÒâ¼ü·µ»Ø    ");
		buzzer=0;
		delayms(500);
		buzzer=1;
	}else{//¸ÃÖ¸ÎÆ´æÔÚ²Ù×÷³É¹¦
		dprintf(0,0,"²Ù×÷³É¹¦        ");
		dprintf(1,0,"                ");
	  dprintf(2,0,"ÓÃ»§ºÅ: ");
		dprintf(2,4,strnum);
		dprintf(2,6,"   ");
		dprintf(3,0,"°´ÈÎÒâ¼ü·µ»Ø    ");
		relay=~relay;//¼ÌµçÆ÷·­×ª
		buzzer=0;
		delayms(500);
		buzzer=1;
	}
	ET0=1;
}

void Stat21(void){//Â¼ÈëÖ¸ÎÆ--È·¶
 
	unsigned char strnum[4]={0};//ÊıÁ¿×ª»»Îª×Ö·û´®
	unsigned char FM_model_num=0,search_num=0;//Ä£¿éÀïÃæµÄÖ¸ÎÆ´æ´¢ÊıÁ¿
  bit ok1=0,ok2=0;
	ET0=0;//¹Ø°´¼üÉ¨Ãè
	LcmClearTXT();//Çå³ıÎÄ±¾
	LcmClearBMP();//Çå³ıÍ¼Ïñ
	FM_model_num=FM_ValidTempleteNum(0);//¶ÁÈ¡Ö¸ÎÆÄ£¿éÀïÃæµÄÄ£°æÊıÁ¿µÍÎ»£¬µÍÎ»¾Í¹»?

	
	if((FM_model_num==0xff)){//Èç¹û¶ÁÄ£°æÊıÖ¸Áî·µ»Ø´íÎóÖµ
		dprintf(0,0,"¶ÁÈ¡Ä£°æ\xCA\xFD´íÎó  ");//keil bugÓÃÄÚÂë
		dprintf(1,0,"                ");
		dprintf(2,0,"                ");
		dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
		goto end;//Ìø¹ıÁË¶ÁÈ¡Ö¸ÎÆ
	}else{//Èç¹ûÊÇÕı³£¶ÁÈ¡Ä£°æÊı
		if((FM_model_num==0)||(FM_model_num==1)){//ÄãÊÇµÚÒ»¸ö£¬µÚ2¸öÊäÈëÖ¸ÎÆµÄ£¬¾ßÓĞ×î¸ßÈ¨ÏŞ
			dprintf(0,0,"    ×î¸ßÈ¨ÏŞ    ");
			goto begin;
		}else{//ÄãÊÇµÚÈı¸ö»òÆäËûÊäÈëµÄÈË£¬ÄãµÄÂ¼ÈëÊÇ²»Í¬µÄ£¬£¬ÄãµÄÊäÈëĞèÒª×î¸ßÈ¨ÏŞµÄÍ¬Òâ£¬ÄãÖ»ÓĞÆÕÍ¨È¨ÏŞ
			dprintf(0,0,"    ÓÃ»§È¨ÏŞ    ");
			dprintf(1,0,"Çë¹ÜÀíÔ±ÊäÈëÖ¸ÎÆ");
			dprintf(2,0,"µÈ\xB4\xFDÊäÈë........");
			search_num=FM_Search();
			if((search_num==1)||(search_num==2)){//Èç¹ûÊÇµÚ	1£¬2ºÅÖ¸ÎÆ¾ÍÊÇ¹ÜÀíÔ±£¬¾ßÓĞÈ¨ÏŞÔËĞĞÏÂÃæµÄÂ¼ÈëÖ¸ÎÆ
				strnum[0]=32;//acsiiÂë¿Õ¸ñ
				strnum[1]= search_num/100+48;     //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  °Ù
				strnum[2]= (search_num%100)/10+48;//+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  Ê®
				strnum[3]= search_num%10+48;      //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  ¸ö?
				dprintf(1,0,"ÑéÖ¤³É¹¦        ");
				dprintf(2,0,"¹ÜÀíÔ±: ");
				dprintf(2,4,strnum);
				dprintf(2,6,"   ");
				buzzer=0;
				delayms(500);//·äÃù
				buzzer=1;
				goto begin;
			}else if(search_num==0xff){//²»ÄÜËÑË÷µ½¸ÃÖ¸ÎÆ
				dprintf(0,0,"    ÓÃ»§È¨ÏŞ    ");
				dprintf(1,0,"¸ÃÖ¸ÎÆ»¹Ã»Â¼Èë  ");
				dprintf(2,0,"                ");
				dprintf(3,0,"°´ÈÎÒâ¼üÍË»ØÖØÊÔ");
				buzzer=0;
				delayms(500);//·äÃù
				buzzer=1;
				goto end;
			}else{//Ö¸ÎÆÊÇÂ¼ÈëµÄµ«²»ÊÇ1£¬2ºÅÖ¸ÎÆ£¬Ã»ÓĞ×î¸ßÈ¨ÏŞ
				strnum[0]=32;//acsiiÂë¿Õ¸ñ
				strnum[1]= search_num/100+48;     //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  °Ù
				strnum[2]= (search_num%100)/10+48;//+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  Ê®
				strnum[3]= search_num%10+48;      //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  ¸ö?
				dprintf(1,0,"ÄãµÄÈ¨ÏŞ²»×ã    ");
				dprintf(2,0,"ÓÃ»§ºÅ: ");
				dprintf(2,4,strnum);
				dprintf(2,6,"   ");
				dprintf(3,0,"°´ÈÎÒâ¼üÍË»ØÖØÊÔ");
				buzzer=0;
				delayms(500);//·äÃù
				buzzer=1;
				goto end;
			}
		}
	}
	    //--------------------¶ÁÈ¡Á½´ÎÖ¸ÎÆ±£´æµ½Ä£°æ---------------------------------
			begin:
		//Êı×Ö×ªÎªcharÀàĞÍÏÔÊ¾
			strnum[0]=32;//acsiiÂë¿Õ¸ñ
			strnum[1]= FM_model_num/100+48;     //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  °Ù
			strnum[2]= (FM_model_num%100)/10+48;//+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  Ê®
			strnum[3]= FM_model_num%10+48;      //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  ¸ö?
			dprintf(1,0,"Ö¸ÎÆ\xCA\xFD: ");//keilÀïÃæµÄbugÒªÓÃºº×ÖÄÚÂë´úÌæ¶ÔÓÚfdµÄºº×Ö,Ö¸ÎÆÊı
			dprintf(1,4,strnum);
			dprintf(1,6,"    ");//Ò»ĞĞÄÚ²¹¿Õ¸ñ±ÜÃâÁËÏÔÊ¾ÂÒÂë
			dprintf(2,0,"µÚÒ»´ÎÂ¼Èë      ");
			dprintf(3,0,"µÈ\xB4\xFDÖĞ..........");//µÈ´ıÖĞ......
			if(FM_GetImage()==1){//»ñÈ¡Ö¸ÎÆÍ¼Ïñ
				dprintf(1,0,"µÚÒ»´Î»ñÈ¡³É¹¦  ");
				dprintf(2,0,"                ");
				dprintf(3,0,"\xD5\xFDÔÚ±£´æµ½¼Ä´æÆ÷");
				if(FM_CreatChar_buffer(1)==1){//±£´æµ½¼Ä´æÆ÷1
					if(FM_Searchfinger1()==0xff){//ÓÃbuffer1ËÑË÷Ö¸ÎÆÈç¹ûÃ»ÓĞÊÕµ½ÄÇ¾ÍÊÇ¸ÃÖ¸ÎÆ»¹Ã»Â¼Èë
						goto con_tinue;//¼ÌĞøÕı³£µÄ³ÌĞòÁ÷
					}else{
						dprintf(1,0,"¸ÃÖ¸ÎÆÒÑ¾­´æÔÚ  ");
						dprintf(2,0,"                ");
						dprintf(3,0,"ÈÎÒâ¼üÍË»ØÖØÊÔ  ");
						buzzer=0;
						delayms(500);//·äÃù
						buzzer=1;
						goto end;
					}
					con_tinue:
					dprintf(1,0,"±£´æ³É¹¦        ");
					dprintf(2,0,"                ");
					dprintf(3,0,"                ");
					buzzer=0;
					delayms(500);//·äÃùÆ÷100ms
					buzzer=1;
					delayms(1000);//ÈÃ×ÖÄ»ÏÔÊ¾2Ãë
					dprintf(3,0,"¿ªÊ¼µÚ¶ş´ÎÂ¼Èë  ");
					if(FM_GetImage()==1){//»ñÈ¡Ö¸ÎÆÍ¼Ïñ
						dprintf(1,0,"µÚ¶ş´Î»ñÈ¡³É¹¦  ");
						dprintf(2,0,"                ");
						dprintf(3,0,"\xD5\xFDÔÚ±£´æµ½¼Ä´æÆ÷");
						if(FM_CreatChar_buffer(2)==1){//½«Í¼Ïñ±£´æµ½¼Ä´æÆ÷2
							dprintf(1,0,"±£´æ³É¹¦        ");
							dprintf(2,0,"                ");
							dprintf(3,0,"                ");
							buzzer=0;
							delayms(500);//·äÃùÆ÷100ms
							buzzer=1;
							delayms(1000);//ÈÃ×ÖÄ»ÏÔÊ¾2Ãë
							dprintf(3,0,"¿ªÊ¼Éú³ÉÄ£°æ    ");
							if(FM_RegModel_Charbuffer()==1){//¸ù¾İÂ¼ÈëµÄÁ½¸öÖ¸ÎÆÌØÕ÷ÂëÉú³ÉÄ£°æ
								dprintf(1,0,"Éú³ÉÄ£°æ³É¹¦    ");
								dprintf(2,0,"                ");
								dprintf(3,0,"ÕıÔÚ±£´æÄ£°æ    ");
								FM_model_num=FM_model_num+1;//ÓÃµ±Ç°ÓĞĞ§Ö¸ÎÆÊı¼Ó1
							  //Êı×Ö×ªÎªcharÀàĞÍÏÔÊ¾
								strnum[0]=32;//acsiiÂë¿Õ¸ñ
								strnum[1]= FM_model_num/100+48;     //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  °Ù
								strnum[2]= (FM_model_num%100)/10+48;//+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  Ê®
								strnum[3]= FM_model_num%10+48;      //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  ¸ö?
								if(FM_Save_model(FM_model_num)==1){//±£´æÄ£°æ³É¹¦£¬¸ù¾İÂ¼ÈëµÄÁ½¸öÖ¸ÎÆÌØÕ÷ÂëÉú³ÉÄ£°æ£¬±£´æµ½FM_model_num+1ÖĞ
									//dprintf(0,0,"    ×î¸ßÈ¨ÏŞ    ");
									dprintf(1,0,"  ±£´æÖ¸ÎÆ³É¹¦  ");
									dprintf(2,0,"Ö¸ÎÆºÅ:");
									dprintf(2,4,strnum);
									dprintf(2,6,"    ");//Ò»ĞĞÄÚ²¹¿Õ¸ñ±ÜÃâÁËÏÔÊ¾ÂÒÂë
									dprintf(3,0,"°´ÈÎÒâ¼ü·µ»Ø    ");
									goto end;
								}else{//±£´æÄ£°æÊ§°Ü
									dprintf(1,0,"±£´æÖ¸ÎÆÊ§°Ü    ");
									dprintf(2,0,"                ");
									dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
									goto end;
								}
							}else{
								dprintf(1,0,"Éú³ÉÖ¸ÎÆÄ£°æÊ§°Ü");
								dprintf(2,0,"                ");
								dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
								goto end;
							}
						}else{
							dprintf(1,0,"±£´æ¼Ä´æÆ÷¶şÊ§°Ü");
							dprintf(2,0,"                ");
							dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
							goto end;
						}
					}else{
						dprintf(1,0,"»ñÈ¡Ö¸ÎÆ¶şÊ§°Ü  ");
						dprintf(2,0,"                ");
						dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
						goto end;
					}
				}else
				{
					dprintf(1,0,"±£´æ¼Ä´æÆ÷Ò»Ê§°Ü");
					dprintf(2,0,"                ");
					dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
					goto end;
				}
			}else{//»ñÈ¡Ö¸ÎÆÊ§°Ü
				dprintf(1,0,"»ñÈ¡Ö¸ÎÆÒ»Ê§°Ü  ");
				dprintf(2,0,"                ");
				dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
				goto end;
			}
			//----------------¶ÁÈ¡Á½´ÎÖ¸ÎÆ²¢Éú³ÉÄ£°æ£¬±£´æÄ£°æ½áÊø-------------------------
	end:  
	ET0=1;//¿ªÉ¨Ãè
}

void Stat22(void){//Çå¿ÕÖ¸ÎÆ--È·¶¨
	unsigned char search_num=0;
	//unsigned int time_count=0;
	ET0=0;
  LcmClearTXT();//Çå³ıÎÄ±¾
	LcmClearBMP();//Çå³ıÍ¼Ïñ
	dprintf(0,0,"×¢Òâ¸Ã²Ù×÷»áÉ¾\xB3\xFD");
	dprintf(1,0,"ËùÓĞÖ¸ÎÆ°üÀ¨¹ÜÀí");
	dprintf(2,0,"Ô±Ö¸ÎÆ!!        ");
	dprintf(3,0,"È·¶¨¼ÌĞøÍË³öÈ¡Ïû");
	while(1){//µÈ´ıÓÃ»§°´¼üÑ¡Ôñ¼ÌĞø»¹ÊÇÍË³,ö
		keyscan();//°´¼üÉ¨Ãè
		switch(Trg){
			case  G_cankey:			  //°´ÏÂÈ¡Ïû¼ü,P3.4
			{
				goto end;//ÍË³ö
			}
			case G_entkey://°´ÏÂÈ·¶¨¼üÅÌ
			{

				goto begin;
			}
			default:break;
		}
		//time_count++;
	}
	goto end;//Èç¹ûÊÇwhileÒòÎª³¬Ê±ÍË³öµÄÏÂÃæµÄÑéÖ¤¾Í²»ÔËĞĞÁË
	begin://Èç¹ûÓÃ»§°´ÏÂÈ·¶¨¼üÅÌ
	dprintf(0,0,"É¾\xB3\xFDËùÓĞÖ¸ÎÆ    ");
	dprintf(1,0,"Çë¹ÜÀíÔ±ÊäÈëÖ¸ÎÆ");
	dprintf(2,0,"                ");
	dprintf(3,0,"µÈ\xB4\xFDÊäÈë........");
	search_num=FM_Search();
	buzzer=0;
	delayms(300);
	buzzer=1;
	if(search_num==1||search_num==2){//ÄãÊÇ¹ÜÀíÔ±
		if(FM_Empty()==1){//³É¹¦
			  dprintf(0,0,"Çå¿ÕÖ¸ÎÆ¿â³É¹¦  ");
				dprintf(1,0,"                ");
				dprintf(2,0,"                ");
				dprintf(3,0,"°´ÈÎÒâ¼üÅÌ·µ»Ø  ");
				goto end;
		}else{
				dprintf(0,0,"Çå¿ÕÖ¸ÎÆ¿âÊ§°Ü  ");
				dprintf(1,0,"                ");
				dprintf(2,0,"                ");
				dprintf(3,0,"°´ÈÎÒâ¼üÅÌ·µ»Ø  ");
				goto end;
		}
	}else if(search_num=0xff){//ÄãµÄÖ¸ÎÆÊäÈëÓĞ´í£¬²»´æÔÚ
		dprintf(0,0,"ÄãµÄÖ¸ÎÆ²»´æÔÚ  ");
		dprintf(1,0,"                ");
		dprintf(2,0,"                ");
		dprintf(3,0,"°´ÈÎÒâ¼ü·µ»Ø    ");
		goto end;
	}else{//ÆäËû²»¾ßÓĞ¹ÜÀíÔ±È¨ÏŞµÄÖ¸ÎÆ
		dprintf(0,0,"ÄãµÄÖ¸ÎÆÈ¨ÏŞ²»¹»");
		dprintf(1,0,"½ö¹ÜÀíÔ±¿ÉÒÔ²Ù×÷");
		dprintf(2,0,"                ");
		dprintf(3,0,"°´ÈÎÒâ¼ü·µ»Ø    ");
		goto end;
	}
	
	end:
	ET0=1;
}

/*
void Stat23(void){//Çå¿ÕÖ¸ÎÆ--È·¶
	unsigned char num;
	unsigned char strnum[4]={0};//ÊıÁ¿×ª»»Îª×Ö·û´®
	ET0=0;
	LcmClearBMP();//Çå³ıÍ¼Ïñ
	num=FM_Search();
								strnum[0]=32;//acsiiÂë¿Õ¸ñ
								strnum[1]= num/100+48;     //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  °Ù
								strnum[2]= (num%100)/10+48;//+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  Ê®
								strnum[3]= num%10+48;      //+48ÊÇÎªÁË×ª»»ÔÚASCIIÂë  ¸ö?
	if(num==0xff){//Ê§°Ü
			  dprintf(0,0,"Çå¿ÕÖ¸ÎÆÊ§°Ü    ");
				dprintf(1,0,"                ");
				dprintf(2,0,"                ");
				dprintf(3,0,"ÇëÍË»Ø          ");
	}else{//³É¹¦
				dprintf(0,0,"Çå¿ÕÖ¸ÎÆ³É¹¦    ");
				dprintf(1,0,"                ");
				dprintf(2,0,strnum);
				dprintf(2,4,"            ");
				dprintf(3,0,"ÇëÍË»ØÖØÊÔ      ");
	}
	ET0=1;
}
*/
/*-------------------------------------------------------------*/
 //Êı¾İ½á¹¹Êı×é,Êı×Ö´ú±íÃ¿¸ö°´¼ü°´ÏÂÊÇµÄ½øÈëµÄ×´Ì¬
StateTab code KeyTab[12]=
{
	{0,1,1,1,1,   (*Stat00)}, //ÎÕÊÖ¼ì²â£¬okµÄ»°¿ÉÒÔÏÔÊ¾ÏÂ¸ö×´Ì¬²»ĞĞµÄ»°¾Í¹Ø¼üÅÌÖĞ¶ÏÉ¨Ãè
	{1,1,2,4,1,   (*Stat10)},    //¶¥²ã£¬´ò¿ª¿ª¹Ø  µ±Ç°µÄ×´Ì¬Ë÷ÒıºÅ,°´ÏÂÏòÏÂ¼üÊ±µÄ×´Ì¬Ë÷ÒıºÅ,°´ÏÂÏòÉÏ¼üÊ±µÄ×´Ì¬Ë÷ÒıºÅ,°´ÏÂ»Ø³µ¼üÊ±µÄ×´Ì¬Ë÷ÒıºÅ,µ±Ç°×´Ì¬Ó¦¸ÃÖ´ĞĞµÄ¹¦ÄÜ²Ù×÷
	{2,1,3,5,2,   (*Stat11)},	   //¶¥²ã£¬Â¼ÈëÖ¸ÎÆ
	{3,2,3,6,3,   (*Stat12)},	   //¶¥²ã£¬Çå¿ÕÖ¸ÎÆ
	//{4,3,4,8,4,   (*Stat13)},	   //¶¥²ã£¬²âÊÔ
	
	{4,1,1,1,1,   (*Stat20)},//µÚ¶ş²ã£¬´ò¿ª¿ª¹ØµÄ×Ó²Ëµ¥
	{5,2,2,2,2,   (*Stat21)},//µÚ¶ş²ã,Â¼ÈëÖ¸ÎÆ
	{6,3,3,3,3,   (*Stat22)},//µÚ¶ş²ã Çå¿ÕÖ¸ÎÆ
	//{8,2,2,2,2,   (*Stat23)},//µÚ¶ş²ã Çå¿ÕÖ¸ÎÆ	
};

//==================================================================================
//========×´Ì¬»ú½á¹¹½áÊø======================================
//=================================================

//**************?????***************************
 void delayms(int ms) 
{      
 unsigned char j;
 while(ms--)
 {
  	for(j =0;j<120;j++);
 }
}
//=======°´¼üÉ¨Ãè=============
void keyscan(){//·µ»ØÊÇÄÄ¸ö°´¼ü
	unsigned char ReadDate=KEY^0xff;;
	Trg=ReadDate&(ReadDate^Cont);
	Cont=ReadDate;
}
/*-------------------------------------------------------------*/
void MenuOperate() interrupt 1
{
   keyscan(); //¼üÅÌÉ¨Ãè£¬¸Ä²»ÁËTrg£¬ContµÄÖµ
    switch(Trg) //¼ì²â°´¼ü´¥·¢
	{
	    case  G_upkey:		       //ÏòÉÏµÄ¼ü£¬TrgÎª1000 0000 ´ú±íP3.7´¥·¢ÁËÒ»´Î
		{
		    KeyFuncIndex=KeyTab[KeyFuncIndex].KeyUpState;
				//ÏÂÃæÊÇÖ´ĞĞ°´¼üµÄ²Ù×÷
			  KeyFuncPtr=KeyTab[KeyFuncIndex].CurrentOperate;
				(*KeyFuncPtr)();     //Ö´ĞĞµ±Ç°µÄ°´¼ü²Ù×÷
			buzzer=0;delayms(20);buzzer=1;//·äÃùÆ÷¶ÌÏìÆğ
			break; 
		}
		case  G_entkey:			  //»Ø³µ¼ü,P3.5
		{
			KeyFuncIndex=KeyTab[KeyFuncIndex].KeyEnterState;
				//ÏÂÃæÊÇÖ´ĞĞ°´¼üµÄ²Ù×÷
			KeyFuncPtr=KeyTab[KeyFuncIndex].CurrentOperate;
			(*KeyFuncPtr)();     //Ö´ĞĞµ±Ç°µÄ°´¼ü²Ù×÷
			buzzer=0;delayms(20);buzzer=1;//·äÃùÆ÷¶ÌÏìÆğ
			break; 
		}
		case  G_downkey:			  //ÏòÏÂµÄ¼ü,P3.6
		{
			KeyFuncIndex=KeyTab[KeyFuncIndex].KeyDownState;
				//ÏÂÃæÊÇÖ´ĞĞ°´¼üµÄ²Ù×÷
			KeyFuncPtr=KeyTab[KeyFuncIndex].CurrentOperate;
			(*KeyFuncPtr)();     //Ö´ĞĞµ±Ç°µÄ°´¼ü²Ù×÷
			buzzer=0;delayms(20);buzzer=1;//·äÃùÆ÷¶ÌÏìÆğ
			break; 
		}
		case  G_cankey:			  //°´ÏÂÈ¡Ïû¼ü,P3.4
		{
			KeyFuncIndex=KeyTab[KeyFuncIndex].KeyCancle;
				//ÏÂÃæÊÇÖ´ĞĞ°´¼üµÄ²Ù×÷
			KeyFuncPtr=KeyTab[KeyFuncIndex].CurrentOperate;
			(*KeyFuncPtr)();     //Ö´ĞĞµ±Ç°µÄ°´¼ü²Ù×÷
			buzzer=0;delayms(20);buzzer=1;//·äÃùÆ÷¶ÌÏìÆğ
			break; 
		}
		//´Ë´¦Ìí¼Ó°´¼ü´íÎó´úÂë,¶¨Ê±É¨ÃèÃ»ÓĞ¼ì²âµ½°´¼ü°´ÏÂ
		default:return;
	}
}	
void main(void){
	delayms(10);//µÈ´ıµ¥Æ¬»ú¸´Î»
	PSB=0;      //Òº¾§Îª´®¿ÚÏÔÊ¾Ä£Ê½£¬½«PSBÒı½ÅÉèÖÃÎª0
	
	//¿ª¶¨Ê±ÖĞ¶Ï-------------------------
	TMOD=0x01;//ÉèÖÃ¶¨Ê±Æ÷0Î»¹¤×÷Ä£Ê½1
	TH0=(65536-22936)/256;//×°³õÖµ11.0592Mhz¾§Õñ¶¨Ê±50msÊıÎª45872,25msÎª22936
	TL0=(65536-22936)%256;//
	EA=1;//¿ª×ÜÖĞ¶Ï
	ET0=1;//¿ª¶¨Ê±Æ÷0ÖĞ¶Ï
	TR0=1;//Æô¶¯¶¨Ê±Æ÷0
	//================================
	
	LcmInit(); //12864³õÊ¼»¯
	LcmClearTXT();//Çå³ıÎÄ±¾
	LcmClearBMP();//Çå³ıÍ¼Ïñ
	FM_Init();//³õÊ¼»¯Ö¸ÎÆÄ£¿éÖ÷ÒªÊÇ´®¿Ú
	//ÔËĞĞÒ»´Î½çÃæÏÔÊ¾
	KeyFuncPtr=KeyTab[KeyFuncIndex].CurrentOperate;
	(*KeyFuncPtr)();     //Ö´ĞĞµ±Ç°µÄ°´¼ü²Ù×÷£¬¾ÍÊÇstat00();
	
	while(1){
		//relay=1;
		//50ms¶¨Ê±ÖĞ¶Ï£¬É¨Ãè¼üÅÌÊäÈë,²»ÓÃ¶¨Ê±ÏìÓ¦»áºÜÂı
	}
}


//-------------------¸÷ÖÖ×Öº¯Êı-----------------------------















