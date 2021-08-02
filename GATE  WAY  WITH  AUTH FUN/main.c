
#include "main.h" 
#include "lcd.c"

#define up    0b00011110
#define down  0b00011101
#define start 0b00011011
#define select 0b00010111
#define menu  0b00001111

#define UPS    1
#define DWN 2
#define STRT 3
#define SLE 4
#define Mnu  5

#define Chanelbed 1
#define ChanelMRI 2



#define  relaylowMRI PIN_a2
#define  relayMedMRI PIN_a1 
#define  relayhighMRI PIN_a0

#define  relaylow PIN_a5
#define  relayMed PIN_a4 
#define  relayhigh PIN_a3


#define TraiacMRI pin_c0
#define TraiacBed pin_c1


char keyVal,keypressed;
char k;
char passCodeValue[4];
char passcode;
char keyCount;
char keycode;
char ModeSelected;
char mode;
char Running =0,MenuSelected=0,menuIndex =0;
unsigned int para[8]={1,1,0,0,1,1,0,0};
char paraLimitLow[8]={1,1,0,0,1,1,0,0};
char  paraLimitHigh[8]={250,30,3,1,250,30,3,1};
char ctpress;
char preMenu;
char dot;
char dutyx = 80;
char BoostVoltage;
char currentIntencityBed =0;
char currentIntencityMRI=0;
char outEnable;
char secondCount;
char miniutCount;
char TimerCounterBed;
char TimerCounterMRI;
char frequency;
char displayupdate;

unsigned  int inter_bed;
unsigned int inter_MRI;
char ProgIndex;
char preprogNoSelected;
char buzCount =0;
int16  pulseCountMRI,pulseCountBed;
char BedDoom;
char menuIndexMin;
char menuIndexMax;
unsigned int Mri_intravel;
char dis;
unsigned int bed_intravel;



void  readKey();
void savepara(void);
void checkpasscode(void);
void parameterSelectMenu(void);
void modeselect(void);
void init(void);
void setpara(void);
void preMnu(char nameIndex);
void autoProgSel(void );
void displayHighlow(char indx);
void pointcur(char ax);
void  setintencity(char level,char chanel);
void setparaDefault(void);

BYTE CONST progData [12][6]= {200,15,2,5,15,3,
							100,15,1,220,15,2,
							10,15,3,5,15,1,
							18,14,3,220,15,2,
							250,10,0,5,15,3,
							200,12,3,90,15,0,
							150,10,3,180,15,1,
							5,1,0,5,10,3,
							10,15,2,150,15,0,
							45,10,1,45,1,0
							222,10,0,180,12,2,
							180,10,3,100,13,3																			
							};


///////////////////////////////////

#int_TIMER0
void  TIMER0_isr(void) 
{

	if(running==1)
	{
	
			//output_toggle(TraiacMRI);
			inter_MRI++;
			if(inter_MRI>0xfa)
			{
				if(TimerCounterMRI > 0)
				{ 
					pulseCountMRI++;
					output_toggle(TraiacMRI);
					inter_MRI = Mri_intravel; 
				}
			}
			inter_Bed++;
			if(inter_Bed >0xfa)
			{
				if(TimerCounterBed > 0) pulseCountbed++;
				output_toggle(Traiacbed);
				inter_Bed = bed_intravel;
			}	
	}
	else
	{
		output_low(TraiacMRI);
		output_low(Traiacbed);
	}
}
#int_TIMER1
void  TIMER1_isr(void) 
{
	secondCount--;
	if(secondCount==0)
	{
		if( buzCount ==0)
		{
		output_low(pin_b5);
		}
		else
		{
			output_toggle(pin_b5);
			buzCount--;
		}		
		displayupdate = 1;		
		secondCount = 8;
		miniutCount--;
		if( miniutCount ==0 ) 
		{
			miniutCount=59;
			if(TimerCounterMRI >0){TimerCounterMRI--;}
			if(TimerCounterBed >0){TimerCounterBed--;}
		}
	}
}
////////////////////////////////////


void  setintencity(char level,char chanel)
{
	if(chanel == 2)
	{
		switch (level)
		{
			case 0:
					output_low (relaylowMRI);
					output_low(relayMedMRI);
					output_low (relayhighMRI);
										
				 	break;
			case 1: output_high (relaylowMRI);
					output_low(relayMedMRI);
					output_low (relayhighMRI);
					
					break;
			case 2: output_high (relaylowMRI);
					output_high(relayMedMRI);
					output_low (relayhighMRI);
					
					break;
			case 3: output_high(relaylowMRI);
					output_high(relayMedMRI);
					output_high (relayhighMRI);
					
					break;
			default: break;

		}
	}
	else  if (Chanel == 1)
	{
		switch (level)
		{
			case 0:	
					output_low (relaylow);										
					output_low (relayhigh);
					output_low(relayMed);
					set_tris_a (0x00);																
				 	break;
			case 1:
					output_high (relaylow);					
					output_low (relayhigh);
					output_low(relayMed);
					output_high(relayMed);
					set_tris_a (0x00);	
					break;
			case 2: output_high (relaylow);					
					output_low(relayhigh);
					output_high(relayMed);
					output_high(relayMed);
					set_tris_a (0x10);					
					break;
			case 3:
					output_high(relaylow);					
					output_high(relayhigh);
					output_high(relayMed);
					output_high(relayMed);
					set_tris_a (0x10);				
					break;
			default:
					break;

		}

	}
}
/////////////////////////////////////////////////////////////////

//////////////////////////////////////////////
void main()
{                                                               ////

	init();
	setintencity(0,2);	
	setintencity(0,1);
	checkpasscode();
modesel:	
	modeselect();
	if(mode > 0)
	{		
		parameterSelectMenu();
		setpara();
		if(preMenu ==1) { goto modesel;}	
	}
	else
	{
		autoProgSel();	
	}
while(1)
{
	if(running==1)
	{			
								
		if(displayupdate == 1)
		{
			displayupdate = 0;
			if(dot < 9)
			{
				 dot++;
				 lcd_gotoxy(8+dot,2);		 
				 printf(lcd_putc,">");	
			}
			else
			{
				lcd_putc("\f");
				lcd_gotoxy(1,2);
				printf(lcd_putc," Runing ");
				dot=0;
			}
			if(mode ==0)
			{			
				lcd_gotoxy(1,1);
				preMnu(ProgIndex);
				if(currentIntencityBed >0)
				{					
					goto bed2;
				}
				else if(currentIntencityMRI>0)
				{				
					goto mri2;
				}	
			
			}
			else if (mode == 1) 
			{
				lcd_gotoxy(1,1);
				printf(lcd_putc," MRI+BED:");						
				if(dis == 1 )
				{
					dis =0;	
					goto bed1;
				}
				else
				{
					dis =1;
					goto mri1;
				}
			}
			else if(mode == 2)
			{
				lcd_gotoxy(1,1);
bed1:			printf(lcd_putc," BED:" );
				displayHighlow(2);
bed2:			lcd_gotoxy(1,3);
				printf(lcd_putc," Time :%d:%d Freq:%u ",TimerCounterbed-1,miniutCount,bed_intravel);
				lcd_gotoxy(1,4);
				printf(lcd_putc," Pulse Count:%lu ",pulseCountbed);
			}
			else if(mode == 3)
			{
				lcd_gotoxy(1,1);
mri1:			printf(lcd_putc," MRI:");
				displayHighlow(6);
mri2:			lcd_gotoxy(1,3);
				printf(lcd_putc," Time :%d:%d Freq:%u ",TimerCounterMRI-1,miniutCount,mri_intravel); 
				lcd_gotoxy(1,4);
				printf(lcd_putc," Pulse Count:%lu ",pulseCountMRI);
			}
				
		}
		if( TimerCounterMRI == 0)
		{			
			output_low(TraiacMRI);
			setintencity(0,ChanelMRI);				
		}
		if( TimerCounterBed == 0)
		{
			output_low(Traiacbed);	
			setintencity(0,Chanelbed);
		}
		if(	TimerCounterBed == 0 && TimerCounterMRI == 0)
		{
			buzCount = 100;
			running =0;				
			lcd_gotoxy(1,1);
			printf(lcd_putc,"\f STOPED");
			setparaDefault();
			delay_ms(8000);
			goto modesel;
		}
	}	
	readKey();
	k++;
	if(keyPressed==1)
	{
		if( keycode== strt)
		{
			output_high(pin_b5);			
			running =0;
			lcd_gotoxy(1,1);	
			setintencity(0,ChanelMRI);
			setintencity(0,Chanelbed);
			printf(lcd_putc,"\f STOPED");
			setparaDefault();
			delay_ms(1000);
			goto modesel;

		}
	}

}
}
//////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

void  readKey()
{
	keyVal =  input_b();
    keyVal= keyVal & 0x1f; 
	keyPressed = 1;
	switch(keyVal)
	{
		case up:
		{
			
			Keycode = 1;	
			
			break;
		}
		case down:
		{
			
			Keycode = 2;

			break;
		}				
		case start:
		{
			output_high(pin_b5);
			Keycode = 3;

			break;
		}
		case select:
		{
				output_high(pin_b5);
				Keycode = 4;

			break;
		}	
		case menu:
		{
				
				Keycode = 5;

			break;

		}
		case 0x1f:
		{
			keyPressed =0;
			break;

		}
		default:
		{
				keyPressed =0;
				break;

		}

	}


}
/////////////////////////////////////////////////////////////////////////////////////////////

void savepara(void)
{
	//	printf(lcd_putc,"\f");
	//	lcd_gotoxy(1,2);
	//	printf(lcd_putc," SAVING VALUES .");
	//	delay_ms(300);
	///	lcd_putc(".");
	///	delay_ms(300);
	///	lcd_putc(".");
	///	delay_ms(300);
	///	lcd_putc(".");
	//	lcd_gotoxy(1,2);
		//printf(lcd_putc," VALUES SAVED  ");		
	//	delay_ms(600);


}

/////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
void checkpasscode(void)
{
	delay_ms(100);
lable1:
	lcd_gotoxy(1,1);
	printf(lcd_putc,"BIO MED CORPORATION");
	lcd_gotoxy(1,2);
	printf(lcd_putc,"      COCHIN       ");
	lcd_gotoxy(1,3);
	printf(lcd_putc,"   MRI Digi-Pro   ");
	keyPressed =0;
	delay_ms(1000);
	while(keyVal != menu)
	{
		readKey();
		delay_ms(7);
	}
	//lcd_gotoxy(1,4);
//	printf(lcd_putc,"ENTER PASS CODE:   ");
//	lcd_send_byte(0,0x0f);
//	lcd_gotoxy(17,4);
//	passcode =0;
//	keyCount =0;
//	passCodeValue[0] =0;
//	passCodeValue[1] =0;
//	passCodeValue[2] =0;
//	passCodeValue[3] =0;
	//while(passcode== 0)
//	{
	//	readKey();
	//	if(keyPressed==1)
	//	{
	//		lcd_gotoxy(17+keyCount,4);
	//		printf(lcd_putc,"*");			
	//		while(keyPressed==1)
	//		{
	//			readKey();
	//			delay_ms(7);
	///		}
	//		passCodeValue[keyCount] =keycode;
	//		keyCount++;
	///		delay_ms(200);
	///		if(keyCount ==4)
	//		{
	//			if(passCodeValue[0]== 3 && passCodeValue[1]== 1 && passCodeValue[2]==5 && passCodeValue[3]==2)
	//			{
					printf(lcd_putc,"\f");
					lcd_gotoxy(1,2);
					printf(lcd_putc,"     WELCOME  ");
						lcd_gotoxy(1,3);
					printf(lcd_putc,"   MRI Digi-Pro   ");
					passcode =1;
					delay_ms(5000);
		//		}
		//		else
		//		{
						//printf(lcd_putc,"\f");
						//lcd_gotoxy(1,2);
						//printf(lcd_putc,"   WRONG PASS CODE ");
						//delay_ms(2000);
				//		goto lable1;

			//	}
		//	}			
//	}
//	}
}

void parameterSelectMenu(void)
{
	
		printf(lcd_putc,"\f");		
		lcd_gotoxy(2,4);
		if(mode == 2 || menuIndex >3 )printf(lcd_putc,"Save");	else printf(lcd_putc,"Next");		
		if(menuIndex < 4){	
			lcd_gotoxy(16,3);
			displayHighlow(2);
			lcd_gotoxy(2,1);
			printf(lcd_putc,"Frequency Bed:%u",para[0]);
			lcd_gotoxy(2,2);
			printf(lcd_putc,"Time Bed     :%u",para[1]);
			lcd_gotoxy(2,3);
			printf(lcd_putc,"Intencity Bed:"); 
			lcd_gotoxy(1,menuIndex+1);	
		}
		else 
		{
			lcd_gotoxy(16,3);
			displayHighlow(6);
			lcd_gotoxy(2,1);	
			printf(lcd_putc,"Frequency MRI:%u"para[4] );
			lcd_gotoxy(2,2);
			printf(lcd_putc,"Time MRI     :%u",para[5]);
			lcd_gotoxy(2,3);
			printf(lcd_putc,"Intencity MRI:");
			lcd_gotoxy(1,menuIndex-3);
		}


}


////////////////////////////////////////////////////////////////
void setpara(void)
{
	
	if (mode == 1)
	{
		menuIndexMin =0;
		menuIndexMax =3 ;
		menuIndex=0;

	}
	else if(mode == 2)
	{
		menuIndexMin =0;
		menuIndexMax =3;
		menuIndex=0;

	}
	else if (mode == 3)
	{
		menuIndexMin =4;
		menuIndexMax =7 ;
		menuIndex=4;
	

	}
	parameterSelectMenu();	
	lcd_send_byte(0,0x0f);
	Running =0;
	MenuSelected=0;		
	while(Running == 0)
	{
		readKey();
		ctpress =100;
xxx:
		if(keyPressed==1)
		{
			
		if(keycode == mnu)
			{
			   preMenu = 1;
				return;		
							
			}
			if(MenuSelected == 0)
			{
				if(keycode==ups)
				{
					if(menuIndex < menuIndexMax){menuIndex++;}
					pointcur(1);				
					//lcd_send_byte(0,0x0f);
					//parameterSelectMenu();				
				}
				else if(dwn== keycode)
				{
					if(menuIndex > menuIndexMin){menuIndex--;}	
					pointcur(1);				
					//lcd_send_byte(0,0x0f);
					//parameterSelectMenu();		
				}
				
				 if(keycode == sle)
				{
					if(menuIndex== menuIndexMax )
					{
						if(mode== 1)
						{
								menuIndexMin =4;
								menuIndexMax =7 ;
								menuIndex=4;
						}
						savepara();
						parameterSelectMenu();
						pointcur(1);
					}
					else
					{
						pointcur(16);
						lcd_send_byte(0,0x0f);				
						MenuSelected =1;
					}		
					
				}
				
			}
			else
			{
				if(keycode == sle)
				{
					MenuSelected =0;
					pointcur(1);
					lcd_send_byte(0,0x0f);
				}
				else
				{
					 if(keycode==dwn)
					{
					
							if(para[menuIndex]< paraLimitHigh[menuIndex]){para[menuIndex]++;}
							//else {para[menuIndex]=paraLimitHigh[menuIndex];}					
						
					}
					else if(ups== keycode)
					{
						
							if(para[menuIndex]> paraLimitLow[menuIndex]){para[menuIndex]--;}
							//else {para[menuIndex]=paraLimitlow[menuIndex];}					
					}				
					if(menuIndex==2||menuIndex==6)
					{	
						pointcur(16);					
						displayHighlow(menuIndex);
						pointcur(16);						
					}
					else 
					{
					
						pointcur(16);
						printf(lcd_putc,"%u ",para[menuIndex]);
						pointcur(16);						
					}
				}			
			
			}
			if(strt== keycode)
			{
				TimerCounterMRI = para[5];
				TimerCounterBed = para[1];			
				setintencity(para[2],Chanelbed);
				setintencity(para[6],ChanelMRI);
				miniutCount=59;
				Mri_intravel =para[4];
				bed_intravel = para[0];
				savepara();
				MenuSelected =0;
				ModeSelected =1;
				Running = 1;	
				preMenu =0;
				pulseCountMRI =0;
				pulseCountBed =0;
				dot= 9;
				lcd_send_byte(0,0x0c);
			}
			while(keyPressed==1 )
			{
				readKey();
				ctpress--;
				if (ctpress == 0)
				{
					ctpress=1;
					delay_ms(100);
					goto xxx;					
				}
				else
				{
					delay_ms(7);
				}
			}				
			
		}

	}
}

////////////////////////////////

void pointcur(char ax)
{
	if(menuIndex <4)
	{
		lcd_gotoxy(ax,menuIndex+1);
	}
	else
	{
		lcd_gotoxy(ax,menuIndex-3);
	}
}

///////////////////////////////////////////////////////////
void modeselect(void)
{	
	printf(lcd_putc,"\f");
	lcd_gotoxy(2,1);
	printf(lcd_putc,"Pre Programed");	
	lcd_gotoxy(2,2);
	printf(lcd_putc,"Bed+MRI");
	lcd_gotoxy(2,3);
	printf(lcd_putc,"Bed only");
	lcd_gotoxy(2,4);
	printf(lcd_putc,"MRI only");
	ModeSelected =0;
	lcd_send_byte(0,0x0f);
	mode = 0;
	lcd_gotoxy(1,1);
	while(ModeSelected== 0)
	{
		readKey();
		if(keyPressed==1)
		{
			if(keycode == ups)
			{
				if(mode<3) {mode++;}
				lcd_gotoxy(1,mode+1);			
			}
			else if(keycode== dwn)
			{
					if(mode>0) {mode--;}
					lcd_gotoxy(1,mode+1);
			}
			else if(keycode== SLE)
			{
				ModeSelected = 1;
			}			
			while(keyPressed==1)
			{
				readKey();
				delay_ms(7);
			}		
			
		}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void init(void)
{
	
  	setup_adc_ports(NO_ANALOGS);
    setup_adc(ADC_OFF);
	setup_spi(SPI_SS_DISABLED);		 
	lcd_init();
	lcd_init();
	setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);	
    setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);
	enable_interrupts(INT_TIMER1);
	enable_interrupts(INT_RTCC);	
	DISable_interrupts(INT_AD);
	secondCount =2;	
	ProgIndex =0;
	miniutCount =1;
	enable_interrupts(GLOBAL);

}



////////////////////////////////////////////////////////////////////////






void preMnu(char nameIndex)
{

	switch(nameIndex)
	{
		case 0:{printf(lcd_putc," ACUTE STRESS DISOR..");      break;} 
		case 1:{printf(lcd_putc," ADJUSTMENT DISORDER ");    break;} 
		case 2:{printf(lcd_putc," AGORAPHOBIA         ");    break;} 
		case 3:{printf(lcd_putc," BIPOLAR DISORDER    ");    break;} 
		case 4:{printf(lcd_putc," DEPRESSIVE DISORDER ");    break;} 
		case 5:{printf(lcd_putc," EATING DISORDERS    ");    break;} 
		case 6:{printf(lcd_putc," HYPERSOMNIA         ");    break;} 
		case 7:{printf(lcd_putc," INSOMNIA            ");    break;}
		case 8:{printf(lcd_putc," SOCIAL PHOBIA       ");    break;} 
		case 9:{printf(lcd_putc," STRESS AND ANXIETY  ");    break;} 
		case 10:{printf(lcd_putc," PANIC DISORDER     ");    break;} 

		default:{printf(lcd_putc," End Of  List       ");    break;} 
	}

}

void autoProgSel(void )
{
		lcd_putc("\f"); 
				lcd_gotoxy(1,1);
				preMnu(0);
				lcd_gotoxy(1,2);
				preMnu(1);
				lcd_gotoxy(1,3);
				preMnu(2);
				lcd_gotoxy(1,4);
				preMnu(3);
				ProgIndex =0;
				lcd_gotoxy(1,2);

	preprogNoSelected=0;
	while(preprogNoSelected == 0)
	{
		readKey();
		if(keyPressed==1)
		{
			if(keycode == ups)
			{
				if (ProgIndex< 10){ ProgIndex++;}							
				
			}
			else if(keycode== dwn)
			{
				if(ProgIndex > 0){ ProgIndex--;}
				
			}
			if(strt == keycode)
			{
				lcd_putc("\f"); 
				TimerCounterMRI = progData[ProgIndex][1];
				TimerCounterbed = progData[ProgIndex][4];
				bed_intravel =progData[ProgIndex][0];
				MRI_intravel =progData[ProgIndex][3];
				//frequency =  progData[ProgIndex][0];
				currentIntencityBed = progData[ProgIndex][2];
				currentIntencityMRI = progData[ProgIndex][5];			
				setintencity(currentIntencityBed,1);
				setintencity(currentIntencityMRI,2);
				//intravel = 600/frequency;			
				Running =1;
				outEnable =1;
				preprogNoSelected =1;
				lcd_send_byte(0,0x0c);
				dot= 9;
				pulseCountMRI =0;
				pulseCountbed =0;
				
			}
				lcd_putc("\f"); 
				lcd_gotoxy(1,1);
				preMnu(ProgIndex-1);
				lcd_gotoxy(1,2);
				preMnu(ProgIndex);
				lcd_gotoxy(1,3);
				preMnu(ProgIndex+1);
				lcd_gotoxy(1,4);
				preMnu(ProgIndex+2);
				lcd_gotoxy(1,2);
						
			while(keyPressed==1)
			{
				readKey();
				delay_ms(7);
			}			
		}
	}
}

void displayHighlow(char indx)
{
		
		if(para[indx]==1)printf(lcd_putc,"Low");
		else if(para[indx]==2)printf(lcd_putc,"Med");
		else if(para[indx]==3)printf(lcd_putc,"Hig");
		else{para[indx]=0;printf(lcd_putc,"Off");}

}



void setparaDefault(void)
{
		para[0]=1;
		para[1]=1;
		para[2]=0;
		para[3]=0;
		para[4]=1;
		para[5]=1;
		para[6]=0;
		para[7]=0;
}	

	