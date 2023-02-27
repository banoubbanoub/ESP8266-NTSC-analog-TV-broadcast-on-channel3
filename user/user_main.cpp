//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include "uart.h"
#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "osapi.h"
#include "espconn.h"

#include "ntsc_broadcast.h"

#include "user/3d.h"
#include <Arduino.h>
#include <user/user_main.h>

#include <user/custom_commands.h>


#include "esp82xx/include/esp82xxutil.h"
#include <esp82xx/include/uart.h>

#define PORT 7777

#define procTaskPrio        0
#define procTaskQueueLen    1

os_timer_t user_main::some_timer;
//struct espconn user_main::*pUdpServer;

//Tasks that happen all the time.

os_event_t    user_main::procTaskQueue[procTaskQueueLen];
//int CNFGPenX, CNFGPenY;
void ICACHE_FLASH_ATTR user_main::SetupMatrix( )
{
	int16_t lmatrix[16];
	d3d::tdIdentity( d3d::ProjectionMatrix );
	d3d::tdIdentity( d3d::ModelviewMatrix );
  
	d3d::Perspective( 600, 250, 50, 8192, d3d::ProjectionMatrix );
}

void user_main::user_pre_init(void)
{
	//You must load the partition table so the NONOS SDK can find stuff.
	esp82xxutil::LoadDefaultPartitionMap();
}
 

//0 is the normal flow
//11 is the multi-panel scene.
#define INITIAL_SHOW_STATE 0

 int gframe;
char lastct[256];
uint8_t showstate = INITIAL_SHOW_STATE;
uint8_t showallowadvance = 1;
int framessostate = 0;
int showtemp = 0;

int16_t user_main::Height( int x, int y, int l )
{
	return d3d::tdCOS( (x*x + y*y) + l );
}

void ICACHE_FLASH_ATTR user_main::DrawFrame(  )
{
	char * ctx = &lastct[0];
	int x = 0;
	int y = 0;
	int i;
	int sqsiz = gframe&0x7f;
	int newstate = showstate;
	d3d::CNFGPenX = 14;
	d3d::CNFGPenY = 20;
	ets_memset( d3d::frontframe, 0x00, ((FBW/4)*FBH) );
	int16_t rt[16];
	d3d::tdIdentity( d3d::ModelviewMatrix );
	d3d::tdIdentity( d3d::ProjectionMatrix );
	d3d::CNFGColor( 17 );
   


	switch( custom_commands::showstate)
   {
	 case 11:  // State that's not in the normal set.  Just displays boxes.
	 {
		for( i = 0; i < 16; i++ )
		{
			int x = i%4;
			int y = i/4;
			x *= (FBW/4);
			y *= (FBH/4);
			d3d::CNFGColor( i );
			d3d::CNFGTackRectangle( x, y, x+(FBW/4)-1, y+(FBH/4)-1);
		}
      }break;
	
	 case 10:
    {
		int i;
		for( i = 0; i < 16; i++ )
		{
			d3d::CNFGPenX = 14;
			d3d::CNFGPenY = (i+1) * 12;
			d3d::CNFGColor( i );
			d3d::CNFGDrawText( "Hello", 3 );
			d3d::CNFGTackRectangle( 120, (i+1)*12, 180, (i+1)*12+12);
		}

		user_main::SetupMatrix();
		d3d::tdRotateEA( d3d::ProjectionMatrix, -20, 0, 0 );
		d3d::tdRotateEA( d3d::ModelviewMatrix, framessostate, 0, 0 );

		for( y = 3; y >= 0; y-- )
		for( x = 0; x < 4; x++ )
		{
			d3d::CNFGColor( x+y*4 );
			d3d::ModelviewMatrix[11] = 1000 + d3d::tdSIN( (x + y)*40 + framessostate*2 );
			d3d::ModelviewMatrix[3] = 600*x-850;
			d3d::ModelviewMatrix[7] = 600*y+800 - 850;
			d3d::DrawGeoSphere();
		}


		if( framessostate > 500 ) newstate = 9;
	
      }break;
	 case 9:
      {
		const char * s = "Direct modulation.\nDMA through the I2S Bus!\nTry it yourself!\n\nhttp://github.com/cnlohr/\nchannel3\n";

		i = ets_strlen( s );
		if( i > framessostate ) i = framessostate;
		ets_memcpy( lastct, s, i );
		lastct[i] = 0;
		d3d::CNFGDrawText( lastct, 3 );
		if( framessostate > 500 ) newstate = 0;
	  }break;

	 case 8:
      {
		int16_t lmatrix[16];
		d3d::CNFGDrawText( "Dynamic 3D Meshes", 3 );
		user_main::SetupMatrix();
		d3d::tdRotateEA( d3d::ProjectionMatrix, -20, 0, 0 );
		d3d::tdRotateEA( d3d::ModelviewMatrix, 0, 0, framessostate );

		for( y = -18; y < 18; y++ )
		for( x = -18; x < 18; x++ )
		{
			int o = -framessostate*2;
			int t = user_main::Height( x, y, o )* 2 + 2000;
			d3d::CNFGColor( ((t/100)%15) + 1 );
			int nx = Height( x+1, y, o ) *2 + 2000;
			int ny = Height( x, y+1, o ) * 2 + 2000;
			//printf( "%d\n", t );
			int16_t p0[3] = { x*140, y*140, t };
			int16_t p1[3] = { (x+1)*140, y*140, nx };
			int16_t p2[3] = { x*140, (y+1)*140, ny };
			d3d::Draw3DSegment( p0, p1 );
			d3d::Draw3DSegment( p0, p2 );
		}

		if( framessostate > 400 ) newstate = 10;
      }break;
	
	 case 7:
       {
		int16_t lmatrix[16];
		d3d::CNFGDrawText( "Matrix-based 3D engine.", 3 );
		user_main::SetupMatrix();
		d3d::tdRotateEA( d3d::ProjectionMatrix, -20, 0, 0 );
		d3d::tdRotateEA( d3d::ModelviewMatrix, framessostate, 0, 0 );
		int sphereset = (framessostate / 120);
		if( sphereset > 2 ) sphereset = 2;
		if( framessostate > 400 )
		{
			newstate = 8;
		}
		for( y = -sphereset; y <= sphereset; y++ )
		for( x = -sphereset; x <= sphereset; x++ )
		{
			if( y == 2 ) continue;
			d3d::ModelviewMatrix[11] = 1000 + d3d::tdSIN( (x + y)*40 + framessostate*2 );
			d3d::ModelviewMatrix[3] = 500*x;
			d3d::ModelviewMatrix[7] = 500*y+800;
			d3d::DrawGeoSphere();
		}

      }break;
	
	 case 6:
	 {
		d3d::CNFGDrawText( "Lines on double-buffered 232x220.", 2 );
		if( framessostate > 60 )
		{
			for( i = 0; i < 350; i++ )
			{
				d3d::CNFGColor( rand()%16 );
				d3d::CNFGTackSegment( rand()%FBW2, rand()%(FBH-30)+30, rand()%FBW2, rand()%(FBH-30)+30 );
			}
		}
		if( framessostate > 240 )
		{
			newstate = 7;
		}
      }break;
	 case 5:
	  {
		ets_memcpy( d3d::frontframe, (uint8_t*)(framessostate*(FBW/8)+0x3FFF8000), ((FBW/4)*FBH) );
		d3d::CNFGColor( 17 );
		d3d::CNFGTackRectangle( 70, 110, 180+200, 150 );		
		d3d::CNFGColor( 16 );
		if( framessostate > 160 ) newstate = 6;
	  }break;
	 case 4:
     {
		d3d::CNFGPenY += 14*7;
		d3d::CNFGPenX += 60;
		d3d::CNFGDrawText( "38x14 TEXT MODE", 2 );

		d3d::CNFGPenY += 14;
		d3d::CNFGPenX -= 5;
		d3d::CNFGDrawText( "...on 232x220 gfx", 2 );

		if( framessostate > 60 && showstate == 4 )
		{
			newstate = 5;
		}
	 }break;
	 case 3:
	 {
		for( y = 0; y < 14; y++ )
		{
			for( x = 0; x < 38; x++ )
			{
				i = x + y + 1;
				if( i < framessostate && i > framessostate - 60 )
					lastct[x] = ( i!=10 && i!=9 )?i:' ';
				else
					lastct[x] = ' ';
			}
			if( y == 7 )
			{
				ets_memcpy( lastct + 10, "36x12 TEXT MODE", 15 );
			}
			lastct[x] = 0;
			d3d::CNFGDrawText( lastct, 2 );
			d3d::CNFGPenY += 14;
			if( framessostate > 120 ) newstate = 4;
		}
	 
	 }break;
	 case 2:
	 {
		ctx += ets_sprintf( ctx, "ESP8266 Features:\n 802.11 Stack\n Xtensa Core @80 or 160 MHz\n 64kB IRAM\n 96kB DRAM\n 16 GPIO\n\
      SPI\n UART\n PWM\n ADC\n I2S with DMA\n                                                   \n Analog Broadcast Television\n" );
		int il = ctx - lastct;
		if( framessostate/2 < il )
			lastct[framessostate/2] = 0;
		else 
			showtemp++;
		d3d::CNFGDrawText( lastct, 2 );
		if( showtemp == 60 ) newstate = 3;
	 }break;
	 case 1:
	 {	i = ets_strlen( lastct );
		lastct[i-framessostate] = 0;
		if( i-framessostate == 1 ) newstate = 2;
      }
	 case 0:
     {
		int stat = wifi_station_get_connect_status();

		d3d::CNFGDrawText( lastct, 2 );
 
		int rssi = wifi_station_get_rssi();
		uint16 sysadc = system_adc_read();
		ctx += ets_sprintf( ctx, "Channel 3 Broadcasting.\nframe: %d\nrssi: %d\nadc:  %d\nsstat:%d\n", gframe, rssi,sysadc, stat );
		struct station_config wcfg;
		struct ip_info ipi;
		wifi_get_ip_info(0, &ipi);
		if( ipi.ip.addr || stat == 255 )
		{
			ctx += ets_sprintf( ctx, "IP: %d.%d.%d.%d\n", (ipi.ip.addr>>0)&0xff,(ipi.ip.addr>>8)&0xff,(ipi.ip.addr>>16)&0xff,(ipi.ip.addr>>24)&0xff );
			ctx += ets_sprintf( ctx, "NM: %d.%d.%d.%d\n", (ipi.netmask.addr>>0)&0xff,(ipi.netmask.addr>>8)&0xff,(ipi.netmask.addr>>16)&0xff,(ipi.netmask.addr>>24)&0xff );
			ctx += ets_sprintf( ctx, "GW: %d.%d.%d.%d\nESP Online\n", (ipi.gw.addr>>0)&0xff,(ipi.gw.addr>>8)&0xff,(ipi.gw.addr>>16)&0xff,(ipi.gw.addr>>24)&0xff );
			showtemp++;
			if( showtemp == 30 ) newstate = 1;
		}

	  
	  } break;
	}

	if( showstate != newstate && showallowadvance )
	{
		showstate = newstate;
		framessostate = 0;
		showtemp = 0;
	}
	else
	{
		framessostate++;
	}

}

 void ICACHE_FLASH_ATTR user_main::procTask(os_event_t *events)
{
	static uint8_t lastframe = 0;
	uint8_t tbuffer = !(gframe&1);

	//CSTick( 0 );

	if( lastframe != tbuffer )
	{
		printf( "FT: %d - ", ntsc_broadcast::last_internal_frametime );
		uint32_t tft = system_get_time();
		d3d::frontframe = (uint8_t*)&ntsc_broadcast::framebuffer[((FBW2/4)*FBH)*tbuffer];
		user_main::DrawFrame();
		ets_memset( d3d::frontframe, 0xaa, ((FBW/4)*FBH) );
		lastframe = tbuffer;
		printf( "%d\n", system_get_time() - tft );
	}

	system_os_post(procTaskPrio, 0, 0 );
}

//Timer event.
void ICACHE_FLASH_ATTR user_main::myTimer(void *arg)
{
	//CSTick( 1 );
}


//Called when new packet comes in.
 void ICACHE_FLASH_ATTR user_main::udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *)arg;

	//uart0_sendStr("X");
/*
	ws2812_push( pusrdata+3, len-3 );

	len -= 3;
	if( len > sizeof(last_leds) + 3 )
	{
		len = sizeof(last_leds) + 3;
	}
	ets_memcpy( last_leds, pusrdata+3, len );
	last_led_count = len / 3;*/
}

void ICACHE_FLASH_ATTR user_main::charrx( uint8_t c )
{
	//Called from UART.
}

void ICACHE_FLASH_ATTR user_main::user_init(void)
{
	
	uart::uart_init(BIT_RATE_115200, BIT_RATE_115200);
	uart::uart0_sendStr("\r\nesp8266 ws2812 driver\r\n");
//	int opm = wifi_get_opmode();
//	if( opm == 1 ) need_to_switch_opmode = 120;
//	wifi_set_opmode_current(2);
//Uncomment this to force a system restore.
	system_restore();
/*
#ifdef FORCE_SSID
#define SSID ""
#define PSWD ""
#endif

	//Override wifi.
#if FORCE_SSID
	{
		struct station_config stationConf;
		wifi_station_get_config(&stationConf);
		os_strcpy((char*)&stationConf.ssid, SSID );
		os_strcpy((char*)&stationConf.password, PSWD );
		stationConf.bssid_set = 0;
		wifi_station_set_config(&stationConf);
		wifi_set_opmode(1);
	}
#endif

	//CSSettingsLoad( 0 );
	//CSPreInit();

	//Override wifi.
#if FORCE_SSID
	{
		struct station_config stationConf;
		wifi_station_get_config(&stationConf);
		os_strcpy((char*)&stationConf.ssid, SSID );
		os_strcpy((char*)&stationConf.password, PSWD );
		stationConf.bssid_set = 0;
		wifi_station_set_config(&stationConf);
		wifi_set_opmode(1);
	}
#else
		wifi_set_opmode(2);
#endif


   // pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	//ets_memset( pUdpServer, 0, sizeof( struct espconn ) );
//	//espconn_create( pUdpServer );
	//pUdpServer->type = ESPCONN_UDP;
	//pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	//pUdpServer->proto.udp->local_port = 7777;
	//espconn_regist_recvcb(pUdpServer, udpserver_recv);

	//if( espconn_create( pUdpServer ) )
	//{
		//while(1) { uart0_sendStr( "\r\nFAULT\r\n" ); }
	//}

     //CSInit(true);
    
	//SetServiceName( "ws2812" );
  	mdns::AddMDNSAddName( "cn8266" );
	mdns::AddMDNSAddName( "ws2812" );
	mdns::AddMDNSService( "_http._tcp", "An ESP8266 Webserver", 80 );
	mdns::AddMDNSService( "_ws2812._udp", "WS2812 Driver", 7777 );
	mdns::AddMDNSService( "_cn8266._udp", "ESP8266 Backend", 7878 );
    */
	//Add a process
	system_os_task(procTask, procTaskPrio, user_main::procTaskQueue, procTaskQueueLen);

	//Timer example
	os_timer_disarm(&user_main::some_timer);
	os_timer_setfn(&user_main::some_timer, (os_timer_func_t *)myTimer, NULL);
	os_timer_arm(&user_main::some_timer, 100, 1);


	ntsc_broadcast::testi2s_init();

	system_update_cpu_freq( SYS_CPU_160MHZ );

	system_os_post(procTaskPrio, 0, 0 );
}


//There is no code in this project that will cause reboots if interrupts are disabled.
void user_main::EnterCritical()
{
}

void user_main::ExitCritical()
{
}


