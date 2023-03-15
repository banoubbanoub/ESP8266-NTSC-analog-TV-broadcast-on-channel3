#include <Arduino.h>
#include <user/ntsc_broadcast.h>
#include <user/user_main.h>
#include <user/custom_commands.h>
#include <IRremote.h>
//#include <Wire.h>
#define IR_pin D0
const int RECV_PIN = IR_pin; // IR Receiver pin
IRrecv irrecv(RECV_PIN);
decode_results results;
void setup() {
 //Serial.begin(9600);
 ntsc_broadcast::testi2s_init();
 irrecv.enableIRIn();
}
	 
void loop() {


	 if(irrecv.decode(&results)){
    
      switch (results.value)
      {
	 case 3772811383:
	   custom_commands::showstate =0;
	   break;
	   case 3772784863:
	   custom_commands::showstate =1;
	   break;
       case 3772817503:
	   custom_commands::showstate =2;
	   break;
	    case 3772801183:
	   custom_commands::showstate =3;
	   break;
       case 3772780783:
	   custom_commands::showstate =4;
	   break;
	   case 3772813423:
	   custom_commands::showstate =5;
	   break;
	   case 3772797103:
	   custom_commands::showstate =6;
	   break;
	   case 3772788943:
	   custom_commands::showstate =7;
	   break;
	   case 3772821583:
	   custom_commands::showstate =8;
	   break;
	   case 3772805263:
	   custom_commands::showstate =9;
	   break;
	   case 3772809343:
	    custom_commands::showstate =10;
		break;
	  }
	
	 }
	 irrecv.resume();
	   static uint8_t lastframe = 0;
	uint8_t tbuffer = !(ntsc_broadcast::gframe&1);
	if( lastframe != tbuffer )
	{
		d3d::frontframe = (uint8_t*)&ntsc_broadcast::framebuffer[((FBW2/4)*FBH)*tbuffer ];
		user_main::DrawFrame( );
		lastframe = tbuffer;
	}
	system_os_post(procTaskPrio, 0, 0 );
	   
	
}        