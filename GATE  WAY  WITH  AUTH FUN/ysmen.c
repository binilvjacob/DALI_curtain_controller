
#include <ysmen.h>
#include <dali.c>
/*
#define led1 PIN_a2
#define led2 PIN_a3
*/
char tempi =	16;

char answer;
char lampondelay;
 
char  step=2,pos;
char redCMD[8];
char frameDelay;
char DatPoint;
char frameError;
char h;
char SeenNo;
char address;
char DataReady =0;
char dataRxd[10],rxv[10];
int xindex,bufferIndex=0;
int datavalied=0;
char temp;

char count=0;
/////////////////
int16 readDly=300;
int1 atmp;

//////////////
int tx_error_check=0;
////////////

char KB_delay=1;

int value;
//////////////////////////////////////////////////////////////////////////////
#int_EXT
EXT_isr()
{
	output_low(pin_a3);
   disable_interrupts(int_ext);
   disable_interrupts(INT_RTCC);   
   DL_bitcount=0;
   enable_interrupts(INT_TIMER1);   
   setup_timer_1(T1_internal|T1_div_by_1);//settimer1with1us least count
   set_timer1(0xffff-840);//928//5100//923
   stopBitCount = 0;
   oddevenbit=1;
   DL_data[0]=0;
   DL_data[1]=0;
   DL_data[2]=0;   
   forwrdFrameFlag = 0;
   masterflag = 0;
   backwardFrameFlag =0;   
   DL_DataCount =0;	
}
#int_TIMER1
TIMER1_isr()
{
	
readDly=20;
error_flag=0;
	if(oddevenbit==1)
	{
		output_high(pin_a3);
		DL_a=input(rx); 
		atmp=DL_a ;          
		oddevenbit=0 ;
		
				if(atmp)
				{
					while(atmp)
						{
							atmp=input(rx);
							if(readDly>0)
								readDly--;
							else
								atmp=0;
								
						}
				}         
				else
				{
					while(!atmp)
						{
							atmp=input(rx);
							readDly--;
								if(readdly==0)
									{
									atmp=1;
									}	
						}
				}

			setup_timer_1(T1_internal|T1_div_by_1);//settimer1with1us least count
			set_timer1(0xffff-150);  //374  //  355             350////old value 150
	}
	else
	{ 
		output_low(pin_a3);
		DL_b=input(rx) ; // store data line status in the second half
		oddevenbit=1;
		setup_timer_1(T1_internal|T1_div_by_1);
		set_timer1(0xffff-350);  // delay  till the next call st to 73 us/////old value 350
		DL_ReadData();  // function  get the dat from the conditions of a and b
					
	}
return(0);
 

}


/////////////////////////////////////////////////////////////////////////////////////////////


#int_RTCC
  RTCC_isr(void) 
{	//restart_wdt();
//	output_toggle(PIN_C4);


	if(frameDelay>0){frameDelay --;}
	else if(bufferIndex >3 )
	{
		datavalied = 1;
		for(xindex =0;xindex<9;xindex++)
		{
			dataRxd[xindex]= rxv[xindex];
		}
           bufferIndex=0;
	}
   	if(settling_time< 24)
   	{		
    	settling_time++;
   	}
RETURN(1);
}


#int_CCP1 
void  CCP1_isr(void)
{ 


}

#int_rda
void RDA_isr(void)
{

		    frameDelay = 50;
        	temp=getc(); 
			if(temp == '*')
			{	
				bufferIndex=0;
				rxv[bufferIndex] =temp;			
			} 
			else
			{
				if(bufferIndex < 8)	
				{ 					
					bufferIndex++;	
					rxv[bufferIndex] =temp;
				}
				else
				{
					bufferIndex= 0;

				}
			} 
}

void main()
{
	rxv="~~~~";
	tempi =16;
	setup_comparator(NC_NC_NC_NC);
	setup_vref(FALSE);
	setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
//	setup_wdt(WDT_18MS);
	setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);    
   setup_timer_2(T2_DISABLED,0,1);
	//setup_ccp1(CCP_CAPTURE_FE);	;
	ext_int_edge( H_TO_L ); 
	
   disable_interrupts(INT_RB);  
   disable_interrupts(INT_TBE);
   //disable_interrupts(INT_RDA);
   disable_interrupts(INT_TIMER1);
   disable_interrupts(INT_TIMER2);   
   disable_interrupts(INT_COMP);
   //enable_interrupts(INT_TIMER0);
   disable_interrupts(INT_EEPROM);
   	disable_interrupts(INT_CCP1);
	enable_interrupts(int_rtcc);
	enable_interrupts(INT_RDA);
	enable_interrupts(INT_EXT); 
	disable_interrupts(GLOBAL);	
	pos=read_eeprom(0x07);
	enable_interrupts(global);
	
	frameDelay =100;
	DatPoint=0;
	while(1)
	{
							/*		lamp_on(255);
									delay_ms(2000);
								    lamp_off(255);	
									delay_ms(2000);
					//	delay_ms(1000);
							*/		
											

		if(DL_dataReady== 1)
	{
	
	//	disable_interrupts(int_rtcc);
	//	disable_interrupts(INT_RDA);	
		if(r_a==0)
		{
		putc('@');
		putc(DL_command);
		putc(DL_address);
	    putc(DL_databyte);
		}		
		else if(r_a==1)
		{
		putc('@');
		putc(DL_command);
		putc(DL_address);
		putc('#');
		}
		

		DL_dataReady =0;
	//	enable_interrupts(int_rtcc);
	//	enable_interrupts(INT_RDA);
	}
	if(datavalied==1)
	{  		
	//	disable_interrupts(int_rtcc);
	//	disable_interrupts(INT_RDA);
	//	disable_interrupts(int_ext);
		if(	dataRxd[0]=='*')
			{			
						
						
				
						switch(dataRxd[1])
							{
								case '%' :
									count=0;
									DL_tx_buffer[0] =dataRxd[4]; //Data
									DL_tx_buffer[1] =dataRxd[2]; //Command
									DL_tx_buffer[2] =dataRxd[3] ; //Address	
									do
									{
									tx_error_check=txmit(1,3);
									count++;
									}while(count<5 && tx_error_check==1);
									count=0;
									datavalied=0;
									break;
							    case '$' :
									count=0;
									DL_tx_buffer[0] =dataRxd[3]; //Address as databyte
									DL_tx_buffer[1] =dataRxd[2]; //Command
									DL_tx_buffer[2] =dataRxd[3]; //Address as databyte
									do
									{
									tx_error_check=txmit(1,3);
									count++;
									}while(count<5 && tx_error_check==1);
									count=0;	
									//DL_dataReady=0;
									datavalied=0;
									break;
								default : 	
											break;
							}	
					
						
								
				
			
	       } 

		
			datavalied=0;
	//		enable_interrupts(int_rtcc);
	//		enable_interrupts(INT_RDA);
	//		enable_interrupts(int_ext);


	}	
	
	if(error_flag==1)
		{
			putc('e');
			error_flag=0;
		}	
	}
}
