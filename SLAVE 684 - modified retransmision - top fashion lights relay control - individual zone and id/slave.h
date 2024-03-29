#include <16F684.h>

#device adc=8

#FUSES NOWDT                 	//No Watch Dog Timer
#FUSES INTRC_IO                     	//Crystal osc <= 4mhz for PCM/PCH , 3mhz to 10 mhz for PCD
//#FUSES XT
#FUSES NOPROTECT             	//Code not protected from reading
#FUSES BROWNOUT              	//Reset when brownout detected
#FUSES NOMCLR                  	//Master Clear pin enabled
#FUSES NOCPD                 	//No EE protection
#FUSES NOPUT                 	//No Power Up Timer
#FUSES NOIESO                	//Internal External Switch Over mode disabled
#FUSES NOFCMEN               	//Fail-safe clock monitor disabled

#use delay(clock=4000000)