//Unless what else is individually marked, all code in this file is
//Copyright 2015 <>< Charles Lohr Under the MIT/x11 License, NewBSD License or
// ColorChord License.  You Choose.

#include "esp82xxutil.h"
#include <c_types.h>
#include <mem.h>
#include "osapi.h"
#include <Arduino.h>
//#include <user_interface.h>

const char * enctypes[6] = { "open", "wep", "wpa", "wpa2", "wpa_wpa2", 0 };
char * generic_ptr;
char * esp82xxutil::parameters;
uint8_t  esp82xxutil::paramcount;

int32 ICACHE_FLASH_ATTR esp82xxutil::safe_atoi( const char * in )
{
	int positive = 1; //1 if negative.
	int hit = 0;
	int val = 0;

	while( *in && hit < 11 	)
	{
		if( *in == '-' )
		{
			if( positive == -1 )
			{
				//Two negatives aren't okay.
				return val*positive;
			}
			positive = -1;
		} else if( *in >= '0' && *in <= '9' )
		{
			val *= 10;
			val += *in - '0';
			hit++;
		} else if (!hit && ( *in == ' ' || *in == '\t' || *in == '\n' ) )
		{
			//okay before we hit a number.
		} else
		{
			//We already hit a number, now we hit something else.  Gotta bail.
			if( hit ) esp82xxutil::paramcount++;
			return val*positive;
		}
		in++;
	}
	if( hit ) esp82xxutil::paramcount++;
	return val*positive;
}

void ICACHE_FLASH_ATTR esp82xxutil::Uint32To10Str( char * out, uint32 dat )
{
	int tens = 1000000000;
	int val;
	int place = 0;

	while( tens > 1 )
	{
		if( dat/tens ) break;
		tens/=10;
	}

	while( tens )
	{
		val = dat/tens;
		dat -= val*tens;
		tens /= 10;
		out[place++] = val + '0';
	}

	out[place] = 0;
}

char ICACHE_FLASH_ATTR esp82xxutil::tohex1( uint8_t i )
{
	i = i&0x0f;
	return (i<10)?('0'+i):('a'-10+i);
}

int8_t ICACHE_FLASH_ATTR fromhex1( char c )
{
	if( c >= '0' && c <= '9' )
		return c - '0';
	else if( c >= 'a' && c <= 'f' )
		return c - 'a' + 10;
	else if( c >= 'A' && c <= 'F' )
		return c - 'A' + 10;
	else
		return -1;
}

void ICACHE_FLASH_ATTR esp82xxutil::NixNewline( char * str )
{
	if( !str ) return;
	int sl = ets_strlen( str );
	if( sl > 1 && str[sl-1] == '\n' ) str[sl-1] = 0;
	if( sl > 2 && str[sl-2] == '\r' ) str[sl-2] = 0;
}




void ICACHE_FLASH_ATTR esp82xxutil::PushString( const char * buffer )
{
	char c;
	while( c = *(buffer++) )
		PushByte( c );
}

void ICACHE_FLASH_ATTR esp82xxutil::PushBlob( const uint8 * buffer, int len )
{
	int i;
	for( i = 0; i < len; i++ )
		PushByte( buffer[i] );
}


int8_t ICACHE_FLASH_ATTR esp82xxutil::TCPCanSend( struct espconn * conn, int size )
{
#ifdef SAFESEND
	return TCPDoneSend( conn );
#else
	struct espconn_packet infoarg;
	sint8 r = espconn_get_packet_info(conn, &infoarg);

	if( infoarg.snd_buf_size >= size && infoarg.snd_queuelen > 0 )
		return 1;
	else
		return 0;
#endif
}

int8_t ICACHE_FLASH_ATTR esp82xxutil::TCPDoneSend( struct espconn * conn )
{
	return conn->state == ESPCONN_CONNECT;
}

const char * ICACHE_FLASH_ATTR  my_strchr( const char * st, char c )
{
	while( *st && *st != c ) st++;
	if( !*st ) return 0;
	return st;
}





//from http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
static const char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                      'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                      '4', '5', '6', '7', '8', '9', '+', '/'};

static const int mod_table[] = {0, 2, 1};

void ICACHE_FLASH_ATTR esp82xxutil::my_base64_encode(const unsigned char *data, size_t input_length, uint8_t * encoded_data )
{

	int i, j;
    int output_length = 4 * ((input_length + 2) / 3);

    if( !encoded_data ) return;
	if( !data ) { encoded_data[0] = '='; encoded_data[1] = 0; return; }

    for (i = 0, j = 0; i < input_length; ) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

	encoded_data[j] = 0;
}



void ICACHE_FLASH_ATTR esp82xxutil::SafeMD5Update( MD5_CTX * md5ctx, uint8_t*from, uint32_t size1 )
{
	char  __attribute__ ((aligned (32))) buffer[32];

	while( size1 > 32 )
	{
		ets_memcpy( buffer, from, 32 );
		//MD5Update( md5ctx, buffer, 32 );
		size1-=32;
		from+=32;
	}
	ets_memcpy( buffer, from, 32 );
	//MD5Update( md5ctx, buffer, size1 );
}

char * ICACHE_FLASH_ATTR esp82xxutil::strdup( const char * src )
{
	int len = ets_strlen( src );
	char * ret = (char*)os_malloc( len+1 );
	ets_memcpy( ret, src, len+1 );
	return ret;
}

char * ICACHE_FLASH_ATTR esp82xxutil::strdupcaselower( const char * src )
{
	int i;
	int len = ets_strlen( src );
	char * ret = (char*)os_malloc( len+1 );
	for( i = 0; i < len+1; i++ )
	{
		if( src[i] >= 'A' && src[i] <= 'Z' )
			ret[i] = src[i] - 'A' + 'a';
		else
			ret[i] = src[i];
	}
	return ret;
}

uint32_t ICACHE_FLASH_ATTR esp82xxutil::GetCurrentIP( )
{
	struct ip_info sta_ip;
	wifi_get_ip_info(STATION_IF, &sta_ip);
	if( sta_ip.ip.addr == 0 )
	{
		wifi_get_ip_info(SOFTAP_IF, &sta_ip);
	}
	if( sta_ip.ip.addr != 0 )
		return sta_ip.ip.addr;
	else
		return 0;
}

char * ParamCaptureAndAdvance( )
{
	if( esp82xxutil::parameters == 0 ) return 0;  //If the string to start with was null
	if( *esp82xxutil::parameters == 0 ) return 0; //If we got to the end of the string.
	esp82xxutil::paramcount++;
	char * ret = esp82xxutil::parameters;
	while( *esp82xxutil::parameters != 0 && *esp82xxutil::parameters != 9 ) esp82xxutil::parameters++;
	if( *esp82xxutil::parameters )
	{
		*esp82xxutil::parameters = 0;
		esp82xxutil::parameters++;
	}
	return ret;
}

int32_t    ParamCaptureAndAdvanceInt( )
{
	char * r = ParamCaptureAndAdvance( );
	if( !r )
		return 0;
	else
	{
		esp82xxutil::paramcount--; //Back out value from ParamCaptureAndAdvance.
		return esp82xxutil::safe_atoi( r );
	}
}


int ICACHE_FLASH_ATTR esp82xxutil::MakePinGPIO( int nr )
{
	static const uint32_t AFMapper[16] = {
		0, PERIPHS_IO_MUX_U0TXD_U, 0, PERIPHS_IO_MUX_U0RXD_U,
		0, 0, 1, 1,
		1, 1, 1, 1,
		PERIPHS_IO_MUX_MTDI_U, PERIPHS_IO_MUX_MTCK_U, PERIPHS_IO_MUX_MTMS_U, PERIPHS_IO_MUX_MTDO_U
	};

	if( AFMapper[nr] == 1 ) {
		return -1;
	} else if( AFMapper[nr] )
		PIN_FUNC_SELECT( AFMapper[nr], 3);  //Select AF pin to be GPIO.

	return 0;
}


/*==============================================================================
 * Partition Map Data
 *============================================================================*/
#include <version.h>

#if ESP_SDK_VERSION_NUMBER > 0x020000

#define SYSTEM_PARTITION_OTA_SIZE_OPT2                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR_OPT2               0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR_OPT2              0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR_OPT2            0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR_OPT2    0xfd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR_OPT2 0x7c000
#define SPI_FLASH_SIZE_MAP_OPT2                        2

#define SYSTEM_PARTITION_OTA_SIZE_OPT3                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR_OPT3               0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR_OPT3              0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR_OPT3            0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR_OPT3    0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR_OPT3 0x7c000
#define SPI_FLASH_SIZE_MAP_OPT3                        3

#define SYSTEM_PARTITION_OTA_SIZE_OPT4                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR_OPT4               0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR_OPT4              0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR_OPT4            0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR_OPT4    0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR_OPT4 0x7c000
#define SPI_FLASH_SIZE_MAP_OPT4                        4
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM 100
#define EAGLE_FLASH_BIN_ADDR                100 + 1
#define EAGLE_IROM0TEXT_BIN_ADDR            100+ 2
#define SYSTEM_PARTITION_RF_CAL 0xfb000
#define SYSTEM_PARTITION_PHY_DATA 0xfc000

#define SYSTEM_PARTITION_SYSTEM_PARAMETER    0xfd000




static  partition_item_t partition_table_opt2[] =
{
    { EAGLE_FLASH_BIN_ADDR,              0x00000,                                     0x10000},
    { EAGLE_IROM0TEXT_BIN_ADDR,          0x10000,                                     0x60000},
    { SYSTEM_PARTITION_RF_CAL,           SYSTEM_PARTITION_RF_CAL_ADDR_OPT2,           0x1000},
    { SYSTEM_PARTITION_PHY_DATA,         SYSTEM_PARTITION_PHY_DATA_ADDR_OPT2,         0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR_OPT2, 0x3000},
};

static const partition_item_t partition_table_opt3[] =
{
    { EAGLE_FLASH_BIN_ADDR,              0x00000,                                     0x10000},
    { EAGLE_IROM0TEXT_BIN_ADDR,          0x10000,                                     0x60000},
    { SYSTEM_PARTITION_RF_CAL,           SYSTEM_PARTITION_RF_CAL_ADDR_OPT3,           0x1000},
    { SYSTEM_PARTITION_PHY_DATA,         SYSTEM_PARTITION_PHY_DATA_ADDR_OPT3,         0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR_OPT3, 0x3000},
};

static const partition_item_t partition_table_opt4[] =
{
    { EAGLE_FLASH_BIN_ADDR,              0x00000,                                     0x10000},
    { EAGLE_IROM0TEXT_BIN_ADDR,          0x10000,                                     0x60000},
    { SYSTEM_PARTITION_RF_CAL,           SYSTEM_PARTITION_RF_CAL_ADDR_OPT4,           0x1000},
    { SYSTEM_PARTITION_PHY_DATA,         SYSTEM_PARTITION_PHY_DATA_ADDR_OPT4,         0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR_OPT4, 0x3000},
};

/**
 * Required function as of ESP8266_NONOS_SDK_v3.0.0. Must call
 * system_partition_table_regist(). This tries to register a few different
 * partition maps. The ESP should be happy with one of them.
 */
 bool esp82xxutil::system_partition_table_regist(const partition_item_t* partition_table,uint32_t partition_num, uint32_t map)
    { return 0;}
void ICACHE_FLASH_ATTR esp82xxutil::LoadDefaultPartitionMap()
{
    if(esp82xxutil::system_partition_table_regist(
                partition_table_opt2,
                sizeof(partition_table_opt2) / sizeof(partition_table_opt2[0]),
                SPI_FLASH_SIZE_MAP_OPT2))
    {
        os_printf("system_partition_table_regist 2 success!!\r\n");
    }
    else if(esp82xxutil::system_partition_table_regist(
                partition_table_opt4,
                sizeof(partition_table_opt4) / sizeof(partition_table_opt4[0]),
                SPI_FLASH_SIZE_MAP_OPT4))
    {
        os_printf("system_partition_table_regist 4 success!!\r\n");
    }
    else if(esp82xxutil::system_partition_table_regist(
                partition_table_opt3,
                sizeof(partition_table_opt3) / sizeof(partition_table_opt3[0]),
                SPI_FLASH_SIZE_MAP_OPT3))
    {
        os_printf("system_partition_table_regist 3 success!!\r\n");
    }
    else
    {
        os_printf("system_partition_table_regist all fail\r\n");
        while(1);
    }
}

#endif
