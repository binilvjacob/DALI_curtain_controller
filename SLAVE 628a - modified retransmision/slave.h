#include <16F628A.h>

//#FUSES WDT                 	//No Watch Dog Timer
//#FUSES xt
#FUSES INTRC_IO                      	//Crystal osc <= 4mhz for PCM/PCH , 3mhz to 10 mhz for PCD
#FUSES PUT                 	//No Power Up Timer
#FUSES PROTECT             	//Code not protected from reading
#FUSES BROWNOUT            	//No brownout reset
#FUSES MCLR                  	//Master Clear pin enabled
#FUSES NOLVP                 	//No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOCPD                 	//No EE protection

#use delay(clock=4000000,RESTART_WDT)

#use rs232(baud=9600,parity=N,xmit=PIN_b2,rcv=PIN_b1,bits=8,DISABLE_INTS,ERRORS)
setup_adc( ADC_OFF );
//#use rs232(baud=19200,parity=N,xmit=PIN_b2,rcv=PIN_b1,bits=8,TIMEOUT=X

