//Copyright 2015 <>< Charles Lohr, see LICENSE file.

//#include <common/commonservices.h>
#include <user/ntsc_broadcast.h>
#include <esp82xx/include/esp82xxutil.h>
#include <user/custom_commands.h>
#include "tablemaker/broadcast_tables.h"
#include <Arduino.h>
uint8_t custom_commands::showstate;
uint8_t custom_commands::showallowadvance;
 int8_t custom_commands::jam_color;

int ICACHE_FLASH_ATTR custom_commands::CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len)
{
	char * buffend = buffer;

	switch( pusrdata[1] )
	{
	case 'C': case 'c': //Custom command test
	{
		buffend += ets_sprintf( buffend, "CC" );
		return buffend-buffer;
	}

	case 'o': case 'O':  //co xxyy   (where xx = current show state, yy = allow advancing)
	{
		//Show control
		char * bp = &buffer[3];
		uint8_t rh = 0;
		rh = esp82xxutil::fromhex1( *(bp++) );
		showstate = (rh << 4) | esp82xxutil::fromhex1( *(bp++) );
		rh = esp82xxutil::fromhex1( *(bp++) );
		showallowadvance = (rh << 4) | esp82xxutil::fromhex1( *(bp++) );
		rh = esp82xxutil::fromhex1( *(bp++) );
		jam_color = (rh << 4) | esp82xxutil::fromhex1( *(bp++) );
		break;
	}

	case 'v': case 'V': //cv xxnnnnnnnnnnnnn (where xx is the color #, nnnnnnnn is the color data)
	{
		int i;
		char * bp = &buffer[3];

		uint8_t ch = esp82xxutil::fromhex1( *(bp++) ); 
		ch = (ch<<4) | esp82xxutil::fromhex1( *(bp++) );

		if( ch >= PREMOD_SIZE )
		{
			buffend += ets_sprintf( buffend, "!CV" );
			break;
		}

		//XXX Todo: make sure we don't read off the end of the input array.
		for( i = 0; i < PREMOD_ENTRIES; i++ )
		{
			int k;
			uint32_t colval = 0;
			for( k = 0; k < 8; k++ )
			{
				colval = (colval<<4) | esp82xxutil::fromhex1( *(bp++) );
			}
			broadcast_tables::premodulated_table[i*PREMOD_SIZE + ch] = colval;
		}

		//Add the overspill bits.
		for( i = PREMOD_ENTRIES; i < PREMOD_ENTRIES_WITH_SPILL; i++ )
		{
			broadcast_tables::premodulated_table[i*PREMOD_SIZE + ch] = broadcast_tables::premodulated_table[(i-PREMOD_ENTRIES)*PREMOD_SIZE + ch];
		}
		
		break;
	}
	}
	return -1;
}
