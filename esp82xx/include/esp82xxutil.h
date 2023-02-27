//Unless what else is individually marked, all code in this file is
//Copyright 2015 <>< Charles Lohr Under the MIT/x11 License, NewBSD License or
// ColorChord License.  You Choose.

#ifndef _ESP82XXUTIL_H
#define _ESP82XXUTIL_H

#include <ets_sys.h>
//#include <c_types.h>
//#include <esp82xx/include/esp8266_rom.h>
#include <Arduino.h>
//#ifndef NOSDK

#include <mem.h>
#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <esp82xx/include/uart.h>
#include <common/mystuff.h>
//XXX WARNING As of 1.3.0, "cansend" doesn't work.
//the SDK seems to misbehave when trying to send without a full
//response packet.  XXX 2: This is not true.  By setting:
//	espconn_set_opt(pespconn, 0x04);
//It makes it so you can keep doing this. 
//As of 1.5.4 that appears to be gone.
#define SAFESEND


#define HTONS(x) ((((uint16_t)(x))>>8)|(((x)&0xff)<<8))

#define IP4_to_uint32(x) (((uint32_t)x[3]<<24)|((uint32_t)x[2]<<16)|((uint32_t)x[1]<<8)|x[0])
#define uint32_to_IP4(x,y) {y[0] = (uint8_t)(x); y[1] = (uint8_t)((x)>>8); y[2] = (uint8_t)((x)>>16); y[3] = (uint8_t)((x)>>24);}

extern const char * enctypes[6];// = { "open", "wep", "wpa", "wpa2", "wpa_wpa2", 0 };

#ifndef DONT_OVERRIDE_PRINTF
#define printf( ... ) { char generic_print_buffer[384]; ets_sprintf( generic_print_buffer, __VA_ARGS__ );  uart::uart0_sendStr( generic_print_buffer ); }
#endif


#define HW_WDT_DISABLE  { *((volatile uint32_t*) 0x60000900) &= ~(1); } // Hardware WDT OFF
#define HW_WDT_ENABLE   { *((volatile uint32_t*) 0x60000900) |= 1; } // Hardware WDT ON

#define PIN_OUT        ( *((volatile uint32_t*)0x60000300) )
#define PIN_OUT_SET    ( *((volatile uint32_t*)0x60000304) )
#define PIN_OUT_CLEAR  ( *((volatile uint32_t*)0x60000308) )
#define PIN_DIR        ( *((volatile uint32_t*)0x6000030C) )
#define PIN_DIR_OUTPUT ( *((volatile uint32_t*)0x60000310) )
#define PIN_DIR_INPUT  ( *((volatile uint32_t*)0x60000314) )
#define PIN_IN         ( *((volatile uint32_t*)0x60000318) )
//Utility functions for dealing with packets on the stack.
#define START_PACK char generic_buffer[1500] __attribute__((aligned (32))); generic_ptr=generic_buffer;
#define PACK_LENGTH (generic_ptr-&generic_buffer[0])
#define END_TCP_WRITE( c ) if(generic_ptr!=generic_buffer) { int r = espconn_sent(c,generic_buffer,generic_ptr-generic_buffer);	}

#define PushByte( c ) { *(generic_ptr++) = c; }
 struct  partition_type_t{
    uint32_t SYSTEM_PARTITION_INVALID = 0;
    uint32_t SYSTEM_PARTITION_BOOTLOADER;           /* user can't modify this partition address, but can modify size */
    uint32_t SYSTEM_PARTITION_OTA_1;                /* user can't modify this partition address, but can modify size */
   uint32_t  SYSTEM_PARTITION_OTA_2;                /* user can't modify this partition address, but can modify size */
  // uint32_t  SYSTEM_PARTITION_RF_CAL;                /* user must define this partition */
  // uint32_t SYSTEM_PARTITION_PHY_DATA;              /* user must define this partition */
 //uint32_t   SYSTEM_PARTITION_SYSTEM_PARAMETER;     /* user must define this partition */
   uint32_t SYSTEM_PARTITION_AT_PARAMETER;
  uint32_t  SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY;
   uint32_t SYSTEM_PARTITION_SSL_CLIENT_CA;
 uint32_t   SYSTEM_PARTITION_SSL_SERVER_CERT_PRIVKEY;
  uint32_t  SYSTEM_PARTITION_SSL_SERVER_CA;
  uint32_t  SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY;
  uint32_t  SYSTEM_PARTITION_WPA2_ENTERPRISE_CA;
  uint32_t  SYSTEM_PARTITION_CUSTOMER_BEGIN = 100;  /* user can define partition after here */
  uint32_t  SYSTEM_PARTITION_MAX;
} ;

    
 struct partition_item_t {
  partition_type_t typesaad;    /* the partition type */
    uint32_t addr;            /* the partition address */
    uint32_t size;            /* the partition size */
} ;

class esp82xxutil{
    public:
static bool system_partition_table_regist(
        const partition_item_t* partition_table,
        uint32_t partition_num,
        uint32_t map
    );
 static  char ICACHE_FLASH_ATTR tohex1( uint8_t i );
 static  int8_t ICACHE_FLASH_ATTR fromhex1( char c ); //returns -1 if not hex char.

 static  int32 ICACHE_FLASH_ATTR safe_atoi( const char * in ); //If valid number, paramcount increments

 static  void ICACHE_FLASH_ATTR Uint32To10Str( char * out, uint32 dat );

 static  void ICACHE_FLASH_ATTR NixNewline( char * str ); //If there's a newline at the end of this string, make it null.

//For holding TX packet buffers
static char * generic_ptr;
static  int8_t ICACHE_FLASH_ATTR  TCPCanSend( struct espconn * conn, int size );
static   int8_t ICACHE_FLASH_ATTR  TCPDoneSend( struct espconn * conn );
static  void  ICACHE_FLASH_ATTR  EndTCPWrite( struct espconn * conn );
static  void ICACHE_FLASH_ATTR PushString( const char * buffer );
static void ICACHE_FLASH_ATTR PushBlob( const uint8 * buffer, int len );


//As much as it pains me, we shouldn't be using the esp8266's base64_encode() function
//as it does stuff with dynamic memory.
static void ICACHE_FLASH_ATTR my_base64_encode(const unsigned char *data, size_t input_length, uint8_t * encoded_data );


static void ICACHE_FLASH_ATTR SafeMD5Update( MD5_CTX * md5ctx, uint8_t*from, uint32_t size1 );

static char * ICACHE_FLASH_ATTR strdup( const char * src );
static char * ICACHE_FLASH_ATTR strdupcaselower( const char * src );

//data: Give pointer to tab-deliminated string.
//returns pointer to *data
//Searches for a \t in *data.  Once found, sets to 0, advances dat to next.
//I.e. this pops tabs off the beginning of a string efficiently.
//These are used in the flash re-writer and cannot be ICACHE_FLASH_ATTR'd out.
//WARNING: These functions are NOT threadsafe.
static char * parameters;
static uint8_t paramcount;
 static char *  ICACHE_FLASH_ATTR ParamCaptureAndAdvance( ); //Increments intcount if good.
 static int32_t ICACHE_FLASH_ATTR ParamCaptureAndAdvanceInt( ); //Do the same, but we're looking for an integer.

 static uint32_t ICACHE_FLASH_ATTR GetCurrentIP( );

//

//#define _BV(x) ((1)<<(x))
static int ICACHE_FLASH_ATTR MakePinGPIO( int pinno ); //returns 0 if OK. Returns nonzero if not.

static  void ICACHE_FLASH_ATTR LoadDefaultPartitionMap();
};
//#endif
#endif
