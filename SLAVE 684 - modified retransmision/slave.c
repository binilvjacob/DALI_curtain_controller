#include <slave.h>
#include<math.h>
 #include <STDLIB.H>



#use delay(clock=4000000)

/////////// Driver details ///////////////////
/*
fan driver

*/
/////////////////////////////////////////////////

#define device_type 3 /////////// setting device type 

#define curtain_open_pin pin_c3
#define curtain_close_pin pin_c2


#define Fixlampid 65                  // LAMP ADDRESS //
#define zoneid_init   255 // zone address // 1-6 192//,5-8 193//,9-12 194//,13-16 195//
#define G1 0b00000001
#define G2 0b00000000
#define rx pin_a2
#define tx pin_a0
#define self 0x01

#define MaxDuty  100 //254 1 led 53,2- 58,3-95
#define MinDuty  0//10 


///////////////////////////////////////////////////////////////

#define PowerOnLevelStore    		0           //[0]
#define SystemFailureLevelStore  	    1           //[1]
#define MinimumLevelStore           	2			// [2]
#define MaximumLevelStore  			3 			// [3]
#define FadeRateStore      			4  			//[4]
#define FadeTimeStore       			5			//[5]
#define ShortAddressStore  			6  			//[6]
#define Group_07Store    			7			//[7]
#define Group_815Store  			8			///15 [8]
#define SceneStore  				9			//15 [9-24]/
#define RandomAddressStore0  			25			//[25-27]
#define RandomAddressStore1  			26			//[25-27]
#define RandomAddressStore2  			27			//[25-27]
#define FastFadeTimeStore  			28			//[28]
#define FailureStatusStore  			29			//[29]
#define OperatingModeStore  30 //[30]
#define DimmingCurveStore  31   //[31]     
#define ZoneIDStore 32



////////////////// Device types ////////////////////////
/*
	lamp =1
	fan=2
	curtain=3
	strip=4
*/
////////////////////////////////////////////////////////

#byte dutyreg = 0x15
 
#bit intf = 0x0b.1
#bit timerOnOff =0x10.0

int1 oddevenbit,a,atmp,b,error_flag,over_flowflag;
unsigned int8 dataCount;
char data[3],bitcount,tout;


unsigned char curtain_duty=0;
int16 curtain_time=0;	


char settling_time,i,dly=4,j;
int1 txmit_error;
char tx_buffer[3];
char r_a,currentSceen;
char command_st,RetryCount;
char FadeRateCount=1;

char zoneid=zoneid_init;


char stopBitCount,address ,command,databyte;
int1 dataready,forwrdFrameFlag,backwardFrameFlag ,masterFlag ;
int16 readDly=300;
int16 GroupSelectReg;
char gindex;
/////// new  //////
int txmit_count=0;
int error_value=0;
///////////////////

char MinimumLevel;
char MaximumLevel;
char SystemFailureLevel;
int16 FadeRate;
char FadeTime;
char RandomAddress0;
char RandomAddress1;
char RandomAddress2;
char FastFadeTime;
char FailureStatus;
char OperatingMode;
char DimmingCurv;
char PowerOnLevel;
char DTR,DwriteLocation,DTR_Ready;

char lampid  = Fixlampid;

void readData(void);
void init(void);
void handle(void );
void copyData(void);
void commands(void);
void txmit(char priority,char length);
void txmit1(void);
void txmit0(void);
void stopbit(void);
void lamp_on(void);
void lamp_off(void);
void startBit(void);
void init_from_eeprom(void);
void goto_position(int16);




#rom  0x2100={MaxDuty,50,MinDuty,Maxduty,5,6,Fixlampid,G1,G2,6,7,8}

#rom  0x2120={zoneid_init}


#int_EXT
EXT_isr() 
{
		restart_wdt(); 		
            disable_interrupts(int_ext);
            disable_interrupts(INT_RTCC);
            bitcount=0;
            setup_timer_1(T1_internal|T1_div_by_1);
            set_timer1(0xffff-840); //858  880///old value 923
            enable_interrupts(int_timer1);
            stopBitCount = 0;
            oddevenbit=1;
            data[0]=0;
            data[1]=0;
            data[2]=0;
            tout=0 ;
            datacount = 0;   
			settling_time = 0; 
}




#int_TIMER1
TIMER1_isr()
{


readDly=20;
error_flag=0;
	if(oddevenbit==1)
	{
	//	output_high( );
		a=input(rx); 
		atmp=a ;          
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
	//	output_low(pin_c3);
		b=input(rx) ; // store data line status in the second half
		oddevenbit=1;
		setup_timer_1(T1_internal|T1_div_by_1);
		set_timer1(0xffff-350);  // delay  till the next call st to 73 us/////old value 350
		readData();  // function  get the dat from the conditions of a and b
					
	}
return(0);





}

#int_RTCC
RTCC_isr()
{	

	restart_wdt(); 
	dly--;
  	if (dly == 0)
  	{	
      dly = 4;
      if(settling_time < 25)
      {
          settling_time++;
      }              
   }   

}


void main(void)

{
	init_from_eeprom();
	init();		
	GroupSelectReg = MAKE16(read_EEPROM (Group_815Store ),read_EEPROM (Group_07Store));	

PowerOnLevel = read_EEPROM (PowerOnLevelStore);

/*	if(PowerOnLevel<= 2)
	{
	 	curtain_duty=0;	
	
	}
	else
	{
		curtain_duty = PowerOnLevel;		
	}
*/
faderate=3;
start:
//output_toggle(pin_C3);
	restart_wdt(); 	
	if (dataReady ==1)
	{
		if(address == 0xff)
		{
			handle(); 
		}		
		else if(address==lampid)		{
			
			handle(); 
		}		
    	else if(address == zoneid)
		{
			handle();	
		}
		else if(address>191 && address<209)
		{	
			gindex = address &0x0F;
			if ( bit_test (GroupSelectReg, gindex)==1)
			{ 				
				handle();
			}	
		}
		dataReady =0;
	}
/*	if(txmit_error==1&&txmit_count<64)
	{
		txmit_count++;
		txmit(2,2);
	}	
	else
	{
		txmit_count=0;
	}
*/
	goto start;
}

void init(void)

{
//	setup_timer_2(T2_OFF);		//26.0 us overflow, 26.0 us interrupt
 	setup_ccp1(CCP_OFF);
//	set_pwm1_duty((int16)10);	
	setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
	//setup_wdt(WDT_2304MS|WDT_DIV_16);
	setup_timer_1(T1_internal|T1_div_by_1);
	timerOnOff=0;
	ext_int_edge( H_TO_L );
	enable_interrupts(INT_EXT);
	enable_interrupts(INT_RTCC);
	disable_interrupts(INT_TIMER2);
	enable_interrupts(global);	
	settling_time =23;
	dataReady =0;
	//set_pwm1_duty(0);
	    
	faderate=3;
/*
	output_high(curtain_open_pin);
	delay_ms(500);
	output_low(curtain_open_pin);
	delay_ms(2000);
	output_high(curtain_close_pin);
	delay_ms(500);
	output_low(curtain_close_pin);
	delay_ms(500);
*/	
	return;
}



void handle(void )

{
	commands();
	delay_ms(2);
	RetryCount =0;

return;

}


//				trnsmission of  bit 1			//
/*********************************************************************
 * Function:       void txmit0(void);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          transmission of  bit 1 to the bus	
 *
 * Side Effects:    None
 *
 * Note:            None
**********************************************************************/

void txmit1(void)

{     
  	txmit_error = 0;
	if (input(rx)==1)
	{  
		output_bit(tx,0);
	}
	delay_us(79);
	if (input(rx)==1)
	{
		output_bit(tx,1);
		txmit_error = 1;
		return;
	}			  
	delay_us(290);//345
	if (input(rx)==0)
	{
		output_bit(tx,1);
	}
	else
	{
		output_bit(tx,1);
		txmit_error = 1;
		return;
	}
	delay_us(79);
	if (input(rx)==0)
	{
		output_bit(tx,1);
		txmit_error = 1;
		return;
	}
    delay_us(290);
	if (input(rx)==0)
	{
		output_bit(tx,1);
		txmit_error = 1;
		return;
	}
    return;
}





//         transmission of 0 to the bus      //
/*********************************************************************
 * Function:       void txmit0(void);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          transmission of  0 bit to the bus	
 *
 * Side Effects:    None
 *
 * Note:            None
**********************************************************************/

void txmit0(void)

{
	txmit_error = 0;	
	output_bit(tx,1);
	delay_us(79);
	if (input(rx)!=1)
	{		
		txmit_error = 1;
		return;
	}   
	delay_us(290);
	if (input(rx)==1)
	{
		output_bit(tx,0);
	}
    else
	{
		output_bit(tx,1);
		txmit_error = 1;
		return;
	}
    delay_us(79);
    if (input(rx)==1)
	{		
		txmit_error = 1;
		return;
	}
    delay_us(290);
	if (input(rx)==1)
	{		
		txmit_error = 1;
		return;
	}
    return;
}
//-----------------------------------------------------------------------------
                   // txmit2 bit
//-----------------------------------------------------------------------------

void txmit(char priority,char length)
{ 
     j= 8*length;
	 while (settling_time < 12+priority);      // priority
     disable_interrupts(global);
     txmit1();        // start bit  
     for(i=0;i<j;i++)
         {
            if (shift_left(tx_buffer,3,1)==1)
            {
                 txmit1();
            }
            else
            {
                  txmit0();
            }
            if (txmit_error ==1)
            {
               goto rr;
            }		
         }        
     stopbit();    
     stopbit(); 
	stopbit(); stopbit();
rr:  output_bit(tx,1);
	 settling_time = 0;
     intf =0;
     enable_interrupts(global);	
	 enable_interrupts(INT_RTCC);
     return;
}

//--------------------------------------------------------------------------
          // stop bit function //
//--------------------------------------------------------------------------
void  stopbit(void)
{
      output_bit(tx,1);
	  restart_wdt(); 
      delay_us(830);
      return;
}

//--------------------------------------------------------------------------


void readData(void)
{
	restart_wdt(); 
      error_flag=0;
      datacount++;
      forwrdFrameFlag = 0;
	  backwardFrameFlag =0;
      if(datacount< 27)
      {
         if((a==0 )&& (b==1))
         {
            shift_left(data,3,1);  // a one  detewcted on bus 
         }
         else if((a==1)&&(b==0))
         {
            shift_left(data,3,0);  // a zero is  deted on the bus 
         }
         else if ( a==1 && b==1)
         {
            switch (datacount)
            {
               case 17:
               {
                     stopBitCount ++;
                     break;
               }
               case 18:
               {
                  stopBitCount ++;
                  if(stopBitCount == 2)
                  {
                        r_a=1; 
                        copyData();
                        forwrdFrameFlag = 1;
                        masterflag = 0;
                        backwardFrameFlag =0;

                  }
                  else
                  {
						error_flag =1;
                  }
                  break;
               }
              	case 25:
				{
					stopBitCount ++;
					break;
				}
              	case 26: 
				{
					stopBitCount ++;
					if(stopBitCount == 2)
					{
						r_a=0; 
						copyData();
						forwrdFrameFlag =0;
						masterflag = 1;
						backwardFrameFlag =0;
					}
					else
					{
						error_flag =1;
					}
					break;
				}
                default:
                {
                      error_flag=1;
                      timerOnOff=0;
                      enable_interrupts(INT_EXT);
                      settling_time = 0;
                      break;
                }
             }   
          } 
		else
		{
			error_flag=1;    
			settling_time = 0;
			timerOnOff=0;       
			enable_interrupts(INT_EXT);       
			enable_interrupts(INT_RTCC);         
		}
      }
	else  // the  data count grater than 27 
	{
		over_flowflag =1 ;
		settling_time = 0;
		timerOnOff=0;       
		enable_interrupts(INT_EXT);       
		enable_interrupts(INT_RTCC);         
	}
    return;
}




void copyData(void)
{ 
	restart_wdt(); 
	dataReady =1;    
	if( r_a==1)
	{
		address = data[1];
		command =data[0];						
	}
	else if( r_a==0)
	{	
		address = data[2];
		command =data[1];
		databyte=data[0]; 
	}       
    timerOnOff=0;
    intf =0;
    enable_interrupts(INT_EXT);
    disable_interrupts(int_timer1);
    enable_interrupts(INT_RTCC);
    settling_time = 0;
    return;
}


void commands(void)
{ 
	command_st =0;	
	switch(command)
	{
		
	   	case 42:	// goto  level 
		{  
			
			if(databyte > MaximumLevel )
			{

                curtain_duty=MaximumLevel;
			}
			else if(databyte< MinimumLevel )
		    {
			
                curtain_duty=MinimumLevel;
			}
			else
			{
		
                curtain_duty=databyte;
			} 		
			goto_position(curtain_duty);	
										
			break;
		}
		case 40:	// on
		{  		
		
            curtain_duty=100;			
			goto_position(curtain_duty);						
			break;
		}
		case 41:	//off
		{  	
		
         curtain_duty=0;
		goto_position(curtain_duty);
			break;
		}
	/*	case 216:	//dim
		case 241:		//ZONE DIM
		{
			if(l_st==1)
			{				
				if(duty>MinimumLevel)
				{							
					duty--;
					SetDimmLevel(duty);					
				}
			}
			break;
		}
		case 220:	//bright
		case 240:  //zone  bright
		{
			if(l_st==1)
			{			
				if(duty < MaximumLevel)
				{									
					duty++;
					SetDimmLevel(duty);			
				}
			}
			break;
		}*/	
	
		case 234: // scene select 
		{
			
			if(databyte < 17)
			{				
				currentSceen = databyte;			
		        curtain_duty = read_EEPROM (currentSceen+SceneStore);	
			     	if(curtain_duty<=MinimumLevel)
					{
					   	curtain_duty=0;					
					
					}
					else
					{								
							
					}			
			}
			goto_position(curtain_duty);
			break;
		 }
		case 231:  // store sceen 
		{

			if(databyte < 17)
			{				
				disable_interrupts (global);
				write_eeprom(databyte+SceneStore,curtain_duty);
				delay_us(5);			
				enable_interrupts(global);	
			}
			
			break;
		}
		case 0x09:
		{		
				GroupSelectReg = MAKE16(read_EEPROM (Group_815Store ),read_EEPROM (Group_07Store));	
				gindex = databyte &0x0f;				
				switch (databyte & 0x10)
				{
					case 0:
						{
							bit_clear(GroupSelectReg,gindex);
							write_eeprom(Group_07Store  ,make8(GroupSelectReg,0));
							delay_us(10);
							write_eeprom(Group_815Store,make8(GroupSelectReg,1));
 							delay_us(10);
							break;
						}
					case 16:
						{
							bit_set(GroupSelectReg,gindex);
							write_eeprom(Group_07Store  ,make8(GroupSelectReg,0));
							delay_us(10);
							write_eeprom(Group_815Store,make8(GroupSelectReg,1));
 							delay_us(10);
							break;
						}
					
					default: break;

				}
				
				break ;
		}
		case 0x22:    // store  short  aress 
		{
			if(databyte <64)
			{
					lampid = databyte;
					write_eeprom(ShortAddressStore ,lampid);
					delay_us(10);
			}
		
			break;		
		}
		case 0x23:    // write  DTR 
		{
					DTR = databyte;	
					DTR_Ready =1;
					break;
		}
		case 0x24:    // write  DTR  to  adress  location  in data  byte 
		{
					
				DwriteLocation = databyte;	
				if(DTR_Ready ==1 && DwriteLocation<33 )
				{
					DTR_Ready =0;
					write_eeprom(DwriteLocation,DTR);
					DELAY_US(20);
				}
			init_from_eeprom();
			break;
		}
		case 0x25:    // Read  DTR  to  adress  location  in data  byte 
		{
				tx_buffer[2]=lampid;tx_buffer[1]=DTR; txmit(2,2);  /////////priority changed
				break;			
		}
		case 0x26:    // Read  eeprom  and  store  in dtr   to  adress  location  in data  byte 
		{
				DwriteLocation = databyte;	
				if( DwriteLocation<33 )
				{
					DTR=Read_eeprom(DwriteLocation);
				}	
				break;				
		}
		case 0x27:
		{
			tx_buffer[2]=lampid;tx_buffer[1]=Read_eeprom(0); 
	    	txmit(2,2);
		
			break;

		}
		case 0x31: //////////////// case for device type query /////////////////
		{
			tx_buffer[2]=lampid;tx_buffer[1]=device_type; 
			txmit(2,2);
					
			
			break;

		}
		default:
		{
			command_st=1;
			break;
		}
	}
	if(command_st==0)
	{ 		
		  write_eeprom(PowerOnLevelStore,curtain_duty);		
	}


return;
}



void goto_position(int16 position)
{

	faderate=2;
	curtain_time=position * faderate * 10;
	
	if(position == 0)
	{
	output_high(curtain_close_pin);
	delay_ms(100);
	output_low(curtain_close_pin);
	}
	else if(position==100)
	{
	output_high(curtain_open_pin);
	delay_ms(100);
	output_low(curtain_open_pin);
	}
	else if(position>0 && position <100)
	{
	// code to go to position 0
	output_high(curtain_close_pin);
	delay_ms(100);
	output_low(curtain_close_pin);
	delay_ms(3000);

	// code to go to desired position
	output_high(curtain_open_pin);
	delay_ms(100);
	output_low(curtain_open_pin);
	delay_ms(curtain_time);
	output_high(curtain_open_pin);
	delay_ms(100);
	output_low(curtain_open_pin);
	}
	else
	{
	output_high(curtain_close_pin);
	delay_ms(100);
	output_low(curtain_close_pin);
	}
	
	return;


}

void init_from_eeprom(void)
{
GroupSelectReg = MAKE16(read_EEPROM (Group_815Store ),read_EEPROM (Group_07Store));	
delay_us(10);
PowerOnLevel 		= read_EEPROM (PowerOnLevelStore);
delay_us(10); 
SystemFailureLevel	= read_EEPROM ( SystemFailureLevelStore ); 
delay_us(10);	    
MinimumLevel		= read_EEPROM ( MinimumLevelStore );   
delay_us(10);       	
MaximumLevel 		= read_EEPROM ( MaximumLevelStore); 
delay_us(10);  		
FadeRate 			= read_EEPROM ( FadeRateStore);
delay_us(10);
FadeTime 			= read_EEPROM ( FadeTimeStore );
delay_us(10);
lampid 				= read_EEPROM ( ShortAddressStore );
delay_us(10);

RandomAddress0 		= read_EEPROM ( RandomAddressStore0); 
delay_us(10);  
RandomAddress1 		= read_EEPROM ( RandomAddressStore1); 
delay_us(10);
RandomAddress2 		= read_EEPROM ( RandomAddressStore2);
delay_us(10);
  		
FastFadeTime 		= read_EEPROM ( FastFadeTimeStore); 
delay_us(10); 		
FailureStatus 		= read_EEPROM ( FailureStatusStore); 
delay_us(10); 
OperatingMode 		= read_EEPROM ( OperatingModeStore); 
delay_us(10);
DimmingCurv 		= read_EEPROM ( DimmingCurveStore);
delay_us(10); 
zoneid=read_EEPROM(zoneidstore);

}


