
#ifndef Custom_Commands_h
#define Custom_Commands_h
//#include <common/commonservices.h>
#include <user/ntsc_broadcast.h>
#include <esp82xx/include/esp82xxutil.h>
#include "tablemaker/broadcast_tables.h"
#include <Arduino.h>
class custom_commands{
    public:
static uint8_t showstate;
static uint8_t showallowadvance;
static int8_t jam_color;
static int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len);
};
#endif