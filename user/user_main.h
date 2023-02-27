
#ifndef User_main_h
#define User_main_h
//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include "uart.h"
#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "osapi.h"
#include "espconn.h"
#include "esp82xx/include/esp82xxutil.h"
#include "ntsc_broadcast.h"
//#include "esp82xx/include/commonservices.h"
//#include <esp82xx/include/mdns.h>
#include "3d.h"
#include <Arduino.h>

#define PORT 7777

#define procTaskPrio        0
#define procTaskQueueLen    1
//0 is the normal flow
//11 is the multi-panel scene.
#define INITIAL_SHOW_STATE 0
class user_main{
    public:
static os_timer_t some_timer;
static struct espconn *pUdpServer;

//Tasks that happen all the time.

static os_event_t    procTaskQueue[procTaskQueueLen];

static void ICACHE_FLASH_ATTR SetupMatrix( );
static void user_pre_init(void);
static int16_t Height( int x, int y, int l );
static void ICACHE_FLASH_ATTR DrawFrame(  );
static void ICACHE_FLASH_ATTR procTask(os_event_t *events);
static void ICACHE_FLASH_ATTR myTimer(void *arg);
static void ICACHE_FLASH_ATTR charrx( uint8_t c );
static void ICACHE_FLASH_ATTR user_init(void);
static void ICACHE_FLASH_ATTR udpserver_recv(void *arg, char *pusrdata, unsigned short len);
static void EnterCritical();
static void ExitCritical();

};
#endif