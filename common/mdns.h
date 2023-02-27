#ifndef MDNS_H
#define MDNS_H

#include "c_types.h"

#define MAX_MDNS_NAMES 5
#define MAX_MDNS_SERVICES 5
#define MAX_MDNS_PATH 32
class mdns{

   public:
static uint8_t * ICACHE_FLASH_ATTR TackTemp( uint8_t * obptr );
static void ICACHE_FLASH_ATTR SetupMDNS();
static int ICACHE_FLASH_ATTR JoinGropMDNS(); //returns nonzero on failure.

//This _does_ dup the data.  Don't worry about keeping the data around.
//Matching name, to respond with full suite of what-I-have
//The first name is automatically inserted to be the device name.
static void ICACHE_FLASH_ATTR AddMDNSAddName( const char * ToDup );  

//Add service names here, I.e. _http._tcp.  or "esp8266" this will make us respond when we get
//those kinds of requests.
//
//All data is dupped
static void ICACHE_FLASH_ATTR AddMDNSService( const char * ServiceName, const char * Text, int port );

//Reset all services and matches.
static void ICACHE_FLASH_ATTR ClearMDNS();
static void ICACHE_FLASH_ATTR ClearMDNSNames();
static uint8_t * ICACHE_FLASH_ATTR ParseMDNSPath( uint8_t * dat, char * topop, int * len );
//Sends part of a path, but, does not terminate, so you an concatinate paths.
static uint8_t * ICACHE_FLASH_ATTR SendPathSegment( uint8_t * dat, const char * path );
};
#endif
