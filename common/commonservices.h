//Copyright 2015 <>< Charles Lohr Under the MIT/x11 License, NewBSD License or
// ColorChord License.  You Choose.

#ifndef _COMMON_H
#define _COMMON_H

#include "c_types.h"
#include <Arduino.h>
#define FLASH_PROTECTION_BOUNDARY 524288
#define MAX_DEVICE_NAME 32
#define USERDATA_SIZE 256
//Browse mechanism.
#define BROWSE_CLIENTS_LIST_SIZE 20
class commonservices{
	public:
//Returns nr bytes to return.  You must allocate retdata.
//It MUST be at least 1,300 bytes large and it MUST be 32-bit aligned.
//NOTE: It is SAFE to use pusrdata and retdata as the same buffer.
static int ICACHE_FLASH_ATTR issue_command(char * retdata, int retsize, char *pusrdata, unsigned short len);

//Includes UDP Control + HTTP Interfaces
static void ICACHE_FLASH_ATTR CSPreInit();
static void ICACHE_FLASH_ATTR CSInit();
static void ICACHE_FLASH_ATTR CSTick( int slowtick );
static void ICACHE_FLASH_ATTR CSConnectionChange();

static void ICACHE_FLASH_ATTR CSSettingsLoad(int force_reinit);
static void ICACHE_FLASH_ATTR CSSettingsSave();
//You must provide:
//Critical should not lock interrupts, just disable services that have problems
//with double-interrupt faults.  I.e. turn off/on any really fast timer interrupts.
//These generally only get called when doing serious operations like reflashing.
static void EnterCritical();
static void ExitCritical();

//If we receive a command that's not F, E or W (Flash Echo Wifi)
static int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len);

//Other utility stuff
static int need_to_switch_opmode; //0 = no, 1 = will need to after a scan. 2 = do it now. 3 = need to go back into station mode.
static struct BrowseClient FoundBrowseClients[BROWSE_CLIENTS_LIST_SIZE];

//Service name can be the title of the service, or can be "esp8266" to list all ESP8266's.
static void ICACHE_FLASH_ATTR BrowseForService( const char * servicename );

//Set the service name for this device.  No more than 10 chars allowed.
static void ICACHE_FLASH_ATTR SetServiceName( const char * myservice ); 
//static struct BrowseClient FoundBrowseClients[BROWSE_CLIENTS_LIST_SIZE];


};
struct CommonSettings
{
	uint8_t settings_key; //Needs to be 0xAF
	char DeviceName[MAX_DEVICE_NAME];
	char DeviceDescription[MAX_DEVICE_NAME];
	char UserData[USERDATA_SIZE];
	static struct CommonSettings SETTINGS;



};

struct BrowseClient
{
	uint32_t ip;
	char    service[11];
	char	devicename[11];
	char    description[17];
};



#endif

