
#include <ysmen.h>
#include <dali.c>


////////spi pins////
#define strobe pin_a0
#define  sdo pin_a1
#define clock pin_a2

char KB_delay=1,display_delay,intex_bit;
int16 led_1,a;
char preKey=0,KeyX=0,countPressDelay1=0,countPressDelay2=0,key_code=0,key;
int1 Key_pressed =0,keyRelease =1,countPress =0,cpToggle =0,data;
int1 lamp1_toggle,lamp2_toggle;


//////////led numder////
#define k1l1 10
#define k2l1 12
#define k2l2 15
#define k2l3 14
#define k2l4 13
#define k3l1 5
#define k4l1 2
#define k5l1 6
#define k6l1 4
#define k7l1 1
#define k8l1 0
BYTE CONST ledptr [12]= {k1l1,k2l3,k3l1,k4l1,k5l1,k6l1,k7l1,k8l1};

void kb_read(void);
char decode_key(void);
void key_handle(void);

#int_EXT
EXT_isr()
{
	restart_wdt();
   disable_interrupts(int_ext);
  // disable_interrupts(INT_RTCC);   
   DL_bitcount=0;
   setup_timer_1(T1_internal|T1_div_by_1);//settimer1with1us least count
   set_timer1(0xffff-5100);//928
   enable_interrupts(INT_TIMER1);
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
	restart_wdt();
	if(oddevenbit==1)
   {
	   //output_high(pin_d2);  // For Timing Corraction
	   DL_a=input(rx);  
	   oddevenbit=0 ;   
	   setup_timer_1(T1_internal|T1_div_by_1);
	   set_timer1(0xffff-2010);     
   }
	else
   {
	   //output_low(pin_d2);  // For Timing Corraction
	   DL_b=input(rx) ; 
	   oddevenbit=1;
	   setup_timer_1(T1_internal|T1_div_by_1);
	   set_timer1(0xffff-1990);
	   DL_ReadData();  
   }
}

#int_RTCC
void  RTCC_isr(void) 
{

	KB_delay--;	
 	if(KB_delay == 0)
		{
			kb_read();
			KB_delay=1;
		}
	
	if(display_delay>0)
		{
		display_delay--;
		led_1=a;
		intex_bit=16;
		}
	else
	{

		output_bit(clock,0);
		output_low(strobe);
		data=bit_test (led_1,intex_bit-1);
		output_bit( sdo,data);
		intex_bit--;
		if(intex_bit==0)
		{
			intex_bit=16;
			output_high(strobe);
			display_delay=50;
		}
		output_bit(clock,1);	
	}



   	if(settling_time< 24)
   	{
    	settling_time++;
   	}

}


void main()
{
	char data;

	setup_timer_1(T1_DISABLED);
	setup_timer_2(T2_DISABLED,0,1);
	setup_comparator(NC_NC_NC_NC);
	setup_vref(FALSE);
	setup_counters(RTCC_INTERNAL,RTCC_DIV_16);// 64 for 16 ms
	setup_timer_1(T1_INTERNAL|T1_DIV_BY_4);
	setup_timer_2(T2_DISABLED,0,1);
	ext_int_edge( H_TO_L ); 
	enable_interrupts(int_rtcc);
	enable_interrupts(INT_EXT);	
	enable_interrupts(global);
	
//Setup_Oscillator parameter not selected from Intr Oscillator Config tab

   // TODO: USER CODE!!
a=0x0000;
output_low(pin_a3);
disable_interrupts(global);
	a=make16(read_eeprom(0x03),read_eeprom(0x02));	
enable_interrupts(global);
	while(1)
	{
scene(8);
//output_toggle(pin_b2);
//txmit1();

			if(Key_pressed ==1)
			{	
				key=decode_key();					
				key_handle();
				Key_pressed = 0;
			}

	}


}

void kb_read(void)
{	

	char keyRead=0xf0;
	preKey = KeyX;
	if(!input(PIN_B4)){bit_clear(keyRead,4);}
	if(!input(PIN_B5)){bit_clear(keyRead,5);}
	if(!input(PIN_B6)){bit_clear(keyRead,6);}
	if(!input(PIN_B7)){bit_clear(keyRead,7);}

//	KeyX = input_b();
keyX=keyRead;
	KeyX=KeyX & 0x30;
	if(preKey == KeyX)
	{
		if(preKey ==0x30)
		  {	
			Key_pressed =0;
			keyRelease =1;
			countPress =0;
			countPressDelay1= 250;	
			countPressDelay2=2;			
			return;				
          } 

		if(countPressDelay1 > 0)
			{
			countPressDelay1--;
			}
		else
			{
			countPressDelay1= 250;
			countPressDelay2--;
			if(countPressDelay2==0)
			{countPress = 1;}
			}

		keyRelease = 0;
		key_code = KeyX;
	 	Key_pressed =1;
	}
		
}
char decode_key()
	{

//	BYTE CONST KeyTable [8]= {0xb0,0xd0,0xe0,0x70,0xa0,0xc0,0x50,0x60};	
	BYTE CONST KeyTable [8]= {0x20,0x10,0xb0,0x70,0xa0,0xc0,0x50,0x60};
	char indxa_key;
	  for(indxa_key = 0;indxa_key <9;indxa_key++)
		{
		if(key_code == keyTable[indxa_key])
			{
				key =indxa_key;			
				return (key);
			}
		} 
	return 0;
	}
void key_handle(void)
{
	 
 	while(countPress ==0 && keyRelease==0)
	{ 
      if(key==6)
		{
			if(lamp1_toggle==1)
				{
					lamp_off(1);  
					bit_clear(a,k7l1);
				}
			else
				{
					lamp_on(1);
	  				bit_set(a,k7l1);
				}
		}
	  else if(key==7)
		{
			if(lamp2_toggle==1)
				{lamp_off(2); bit_clear(a,k8l1);}
			else
				{lamp_on(2);   bit_set(a,k8l1); }
		}
	}
	if(countPress==0)
	{
	
		if(key<6)
		{
			scene(key);
			a=0x0000;bit_set(a,ledptr[key]);
		}
		if(key==6)
			{
				if(lamp1_toggle==1)
					lamp1_toggle=0;
				else
					lamp1_toggle=1;
			}
		if(key==7)
			{
				if(lamp2_toggle==1)
					lamp2_toggle=0;
				else
					lamp2_toggle=1;
			}

	}
	else
	{	
		if(key<6)
			store(key);
		if(key==6)
			dim_lamp(1);
		if(key==7)
			dim_lamp(2);

	}	
	
	disable_interrupts(global);
  	write_eeprom(0x02,make8(a,0));
	write_eeprom(0x03,make8(a,1));
	enable_interrupts(global);
}