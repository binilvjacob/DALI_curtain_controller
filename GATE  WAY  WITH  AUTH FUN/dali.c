

char DL_dataCount;
char DL_a,DL_b,oddevenbit,error_flag,over_flowflag,r_a;
char addr;
char r_duty;
char sc;
char pre_command;
char rec_count,rec_time;
char DL_bitcount;
char settling_time;
extern char  step,pos;

int1 txmit_error;


char DL_data[3], DL_tx_buffer[3];  
char DL_j,DL_n,replay,zoneAdPir;

// variables and flags for the function recive dat

char stopBitCount,DL_Address ,DL_Databyte,DL_command ;	
char DL_dataready=0,forwrdFrameFlag,backwardFrameFlag ,masterFlag,zoneAdLight ;


#define  rx pin_B0
#define  tx pin_A2

#bit intf = 0x0b.1
//#bit timerOnOff =0x10.0
int1 tx_status=0;



#define MinIntencity   100
#define MaxIntencity 150

void DL_ReadData(void); 
void stopbit(void);
int txmit(char priority,char length);
void DL_copyData(void);
void txmit0(void);
void txmit1(void);
void repalyHandle(void);

void lamp_on(char add);
void lamp_off(char add);
void bright_lamp (char lamp_add);
void dim_lamp (char lamp_add);
void bright_zone(char);
void dim_zone(char);
void store(char sc);
void scene(int sce);
void setAutoSensorEnable(void);
void setAutoSensorDisable(void);
void replaysensorLight(void);
void replaysensorPIR(void);
void fan_pos(char);






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
	delay_us(290);///old value 340
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
    delay_us(290); ///old value 270
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
    delay_us(290); ///old value 334
	if (input(rx)==1)
	{		
		txmit_error = 1;
		return;
	}
    return;
}
//         transmission of 2 bits to the bus		//
/*********************************************************************
 * Function:       void txmit(char priority,char length);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          transmission of  2 bits to the bus	
 *
 * Side Effects:    None
 *
 * Note:            None
**********************************************************************/
int txmit(char priority,char length)
{

	DL_j= 8*length;
	while (settling_time < 12+priority);      // priority	
    disable_interrupts(global);	
	txmit1();         // start bit
	for(DL_n=0;DL_n<DL_j;DL_n++)
	{
		if (shift_left(DL_tx_buffer,3,1)==1)
		{
			txmit1();
		}
		else
		{
			txmit0();
		}
		if (txmit_error ==1)
		{
			goto endx;
		}
	} 
	stopbit(); stopbit();	
	stopbit(); stopbit();	
	endx:output_bit(tx,1);
	settling_time = 0;
	intf = 0;
	DL_dataCount=0;
	tx_status=1;	
	enable_interrupts(global);	
	enable_interrupts(INT_RTCC);

	return txmit_error;
}

//   transmission of stopbit to the bus		//
/*********************************************************************
 * Function:       void  stopbit(void);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          transmission of stopbit to the bus	
 *
 * Side Effects:    None
 *
 * Note:            None
**********************************************************************/

void  stopbit(void)
{
output_bit(tx,1);
delay_us(830);
return;
}
//*****************************************************************************

// dali recive functions ///////////
/*********************************************************************
 * Function:       void  readData(void));
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          reception function
 *
 * Side Effects:    None
 *
 * Note:            None
**********************************************************************/

void DL_ReadData(void)
{

	error_flag=0;
	DL_dataCount++;
      forwrdFrameFlag = 0;
	  backwardFrameFlag =0;
	if(DL_dataCount < 27)
	{
		if((DL_a==0 )&& (DL_b==1))
	    {
			shift_left(DL_data,3,1);
		}
		else if((DL_a==1)&&(DL_b==0))
		{
			shift_left(DL_data,3,0);
		}
		else if ( DL_a==1 && DL_b==1)
		{

			switch (DL_dataCount)
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
                        dl_copyData();
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
						dl_copyData();
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
                      setup_timer_1(T1_disabled);
                      enable_interrupts(INT_EXT);
					  enable_interrupts(INT_RTCC); 
                      settling_time = 0;
                      break;
                }
             }   
          } 
		else
		{
			error_flag=1;    
			settling_time = 0;
			setup_timer_1(T1_disabled);       
			enable_interrupts(INT_EXT);       
			enable_interrupts(INT_RTCC);         
		}
      }
	else  // the  data count grater than 27 
	{
		over_flowflag =1 ;
		settling_time = 0;
		setup_timer_1(T1_disabled);       
		enable_interrupts(INT_EXT);       
		enable_interrupts(INT_RTCC);         
	}

    return;
}

/*********************************************************************
 * Function:        void DL_copyData(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:			storing of data into buffer
 *
 * Side Effects:    None
 *
 * Note:            None
 ********************************************************************/

void DL_copyData(void)
{

	DL_dataReady =1;
	if( r_a==1)
	{
		DL_Address = DL_data[1];
		DL_command =DL_data[0];
	}

	else if( r_a==0)
	{	 
		DL_address = DL_data[2];
		DL_command =DL_data[1];
		DL_databyte=DL_data[0]; 
		
	}       
    setup_timer_1(T1_disabled);
    intf =0;
    enable_interrupts(INT_EXT);
    disable_interrupts(int_timer1);
    enable_interrupts(INT_RTCC);
    settling_time = 0;
    return;
}

/*********************************************************************
 * Function:       replay();
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          replay handling
 *
 * Side Effects:    None
 *
 * Note:            None
**********************************************************************/
void repalyHandle(void)
{  
	if(forwrdFrameFlag==1)
		{			
			
         	if(DL_Address>0x70&&DL_Address<=0x79)
			{
				//if(replay==1)
				//{
					zoneAdPir =  DL_Address +79;
					replaysensorPIR();					
				//}			
			}
			 else if(DL_Address>0x80&&DL_Address<=0x89)
			{
				//if(replay==1)
				//{
					zoneAdLight =  DL_Address +64;
					replaysensorLight();
				//}			
			}
			else
			{
	
				switch (pre_command)
				{
					case 234: // sccen 
					{
						if(DL_Databyte==sc)
							{
							rec_count++;						
						    }
					break;
					}
					case 208: // lam on 
					{
				     	if( bit_test(DL_Databyte,7)==1) 
							{
							r_duty=DL_Databyte&0x7f;
							set_pwm1_duty(r_duty);
							}
						else
							{
								delay_ms(30);
								lamp_on(addr);
							}
					break;
					}
					case 212: //lamp off
					{
		                if( bit_test(DL_Databyte,7)==0) 
							{
							set_pwm1_duty(00);
							}
						else
							{
								delay_ms(30);
								lamp_off(addr);
							}
					
					break;
					}
					case 220:
					{
	                if( bit_test(DL_Databyte,7)==1) 
							{
							r_duty=DL_Databyte&0x7f;
							set_pwm1_duty(r_duty);
							}
					break;
					}
					case 216:
					{
					if( bit_test(DL_Databyte,7)==1) 
							{
							r_duty=DL_Databyte&0x7f;
							set_pwm1_duty(r_duty);
							}
					break;
					}
				}
			}
		}

	else if(backwardFrameFlag ==1)
	{}
	else if(masterFlag ==1)
	{}
return;
}


/*********************************************************************
 * Function:       void lamp_on(char addr);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:         transmitting the data to make the lamp on
 *
 * Side Effects:    None
 *
 * Note:            None
***********************************************************************/
void lamp_on(char addr)
{
//printf("\n \r LAMP ON");
pre_command=208;
DL_tx_buffer[2] = addr;
DL_tx_buffer[1] =208; //command
DL_tx_buffer[0] = addr;// addreass
txmit(0,2);
return;
}
/*********************************************************************
 * Function:       void lamp_off(char addr);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:         transmitting the data to make the lamp off
 *
 * Side Effects:    None
 *
 * Note:            None
***********************************************************************/
void lamp_off(char addr)
{
//printf("\n \r LAMP Off");
pre_command=212;
DL_tx_buffer[2] = addr;
DL_tx_buffer[1] =212; //command
DL_tx_buffer[0] = addr;// addreass
txmit(1,2);
return;
}

// ....both single and group dim...///
/*********************************************************************
 * Function:       void dim_lamp(char lamp_addr);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:         transmitting the data to dim the lamp
 *
 * Side Effects:    None
 *
 * Note:            None
***********************************************************************/

void dim_lamp (char lamp_add)
{
   //printf("\n \r LAMP dim");

   pre_command=216;
   DL_tx_buffer[2] = lamp_add;// addreass
   DL_tx_buffer[1] =216; //command
   DL_tx_buffer[0] = step;
   txmit(0,3);
   return;

}
/*********************************************************************
 * Function:       void bright_lamp(char addr);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:         transmitting the data to bright the lamp
 *
 * Side Effects:    None
 *
 * Note:            None
***********************************************************************/
void bright_lamp (char lamp_add)
{
	//printf("\n \r LAMP bright");
	pre_command=220;
	DL_tx_buffer[2] = lamp_add;
	DL_tx_buffer[1] =220; //command
	DL_tx_buffer[0] = step;
	txmit(0,3);
	return;
}

void fan_pos(char fan_addr)
{
	
	pre_command=220;
	DL_tx_buffer[2] = fan_addr;
	DL_tx_buffer[1] =100; //command
	DL_tx_buffer[0] = pos;
	txmit(0,3);
	return;
}


void bright_zone(char zone_add)
{
//	printf("\n \r zone bright");
	pre_command=240;
	DL_tx_buffer[2] = zone_add;
	DL_tx_buffer[1] =240; //command
	DL_tx_buffer[0] = zone_add;// addreass
	txmit(0,2);
    return;
}


void dim_zone(char zone_add)
{  
  // printf("\n \r zone dim"); 
   pre_command=241;
   DL_tx_buffer[2] = zone_add;
   DL_tx_buffer[1] =241; //command
   DL_tx_buffer[0] = zone_add;// addreass
   txmit(0,2);
   return;
}

void store(char sc)
{
//	printf("\n \r store");	
	DL_tx_buffer[0] = sc;
	DL_tx_buffer[1] = 231;
	DL_tx_buffer[2] = 255;
	txmit(1,3);
}

void scene(sc)
{
	//	printf("\n \r scene");			
		pre_command=234;
		rec_time=5; 
		rec_count=0;
		DL_tx_buffer[0] =sc ;
		DL_tx_buffer[1] = 234;
		DL_tx_buffer[2] = 255;
		txmit(1,3);
}

void scene2(sc)
{
	//	printf("\n \r scene");			
		pre_command=234;
		rec_time=5; 
		rec_count=0;
		DL_tx_buffer[0] =sc ;
		DL_tx_buffer[1] = 234;
		DL_tx_buffer[2] = 255;
		txmit(1,3);
}


void setAutoSensorEnable()
{	
	replay=1;	
	DL_tx_buffer[2] = 127;
	DL_tx_buffer[1] =242; //command
	DL_tx_buffer[0] = 127;// addreass
	txmit(0,2);

}
void setAutoSensorDisable()
{	
	replay=0;	
	DL_tx_buffer[2] = 127;
	DL_tx_buffer[1] =243; //command
	DL_tx_buffer[0] = 127;// addreass
	txmit(0,2);
}

void replaysensorPIR(void)
{

	if(DL_Databyte==0xff)
	{		
		DL_tx_buffer[2] = zoneAdPir ;
		DL_tx_buffer[1] = 222; //command
		DL_tx_buffer[0] = zoneAdPir ;// addreass
		txmit(0,2);
		
	}
	else if(DL_Databyte==0)
	{	
		DL_tx_buffer[2] = zoneAdPir ;
		DL_tx_buffer[1] =241; //command
		DL_tx_buffer[0] = zoneAdPir ;// addreass
		txmit(1,2);
		txmit(1,2);	
	}
}

void replaysensorLight(void)
{
if(DL_Databyte > MaxIntencity)
	{		
		DL_tx_buffer[2] = zoneAdLight;
		DL_tx_buffer[1] =241; //command    // icrease intencityzone
		DL_tx_buffer[0] = zoneAdLight;// addreass
		txmit(0,2);
	}
else if(DL_Databyte< MinIntencity)
	{		
		DL_tx_buffer[2] = zoneAdLight;
		DL_tx_buffer[1] =240; //command  Dicease intencity 
		DL_tx_buffer[0] = zoneAdLight;// addreass
		txmit(1,2);
	}
}

