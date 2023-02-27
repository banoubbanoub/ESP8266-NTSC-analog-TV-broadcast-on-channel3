/******************************************************************************
 * Copyright 2013-2015 Espressif Systems
 *           2015 <>< Charles Lohr
 *
 * FileName: i2s_freertos.c
 *
 * Description: I2S output routines for a FreeRTOS system. Uses DMA and a queue
 * to abstract away the nitty-gritty details.
 *
 * Modification history:
 *     2015/06/01, v1.0 File created.
 *     2015/07/23, Switch to making it a WS2812 output device.
 *     2016/01/28, Modified to re-include TX_ stuff.
*******************************************************************************

Notes:

 This is pretty badly hacked together from the MP3 example.
 I spent some time trying to strip it down to avoid a lot of the TX_ stuff. 
 That seems to work.

 Major suggestions that I couldn't figure out:
	* Use interrupts to disable DMA, so it isn't running nonstop.
    * Use interrupts to flag when new data can be sent.

 When I try using interrupts, it seems to work for a bit but things fall apart
 rather quickly and the engine just refuses to send anymore until reboot.

 The way it works right now is to keep the DMA running forever and just update
 the data in the buffer so it continues sending the frame.

Extra copyright info:
  Actually not much of this file is Copyright Espressif, comparativly little
  mostly just the stuff to make the I2S bus go.

*******************************************************************************/


#include "slc_register.h"
#include "esp82xx/include/esp82xxutil.h"
#include <c_types.h>
#include <user/ntsc_broadcast.h>
#include "user_interface.h"
#include "tablemaker/broadcast_tables.h"
#include <include/dmastuff.h>
#include <Arduino.h>
#include "tablemaker/CbTable.h" 
#define FUNC_I2SO_DATA                      1

#define WS_I2S_BCK 1  //Can't be less than 1.
#define WS_I2S_DIV 2

#define I2SDMABUFLEN (159)		//Length of one buffer, in 32-bit words.
//#define LINE16LEN (I2SDMABUFLEN*2)
#define LINE32LEN I2SDMABUFLEN

//Bit clock @ 80MHz = 12.5ns
//Word clock = 400ns
//Each NTSC line = 15,734.264 Hz.  63556 ns
//Each group of 4 bytes = 

#define LINETYPES 6

 int8_t ntsc_broadcast::jam_color = -1; 

//WS_I2S_DIV - if 1 will actually be 2.  Can't be less than 2.

//		CLK_I2S = 160MHz / I2S_CLKM_DIV_NUM
//		BCLK = CLK_I2S / I2S_BCK_DIV_NUM
//		WS = BCLK/ 2 / (16 + I2S_BITS_MOD)
//		Note that I2S_CLKM_DIV_NUM must be >5 for I2S data
//		I2S_CLKM_DIV_NUM - 5-127
//		I2S_BCK_DIV_NUM - 2-127



//I2S DMA buffer descriptors
static struct sdio_queue i2sBufDesc[DMABUFFERDEPTH];
uint32_t i2sBD[I2SDMABUFLEN*DMABUFFERDEPTH];

//Queue which contains empty DMA buffers

//This routine is called as soon as the DMA routine has something to tell us. All we
//handle here is the RX_EOF_INT status, which indicate the DMA has sent a buffer whose
//descriptor has the 'EOF' field set to 1.
int gline = 0;
uint32_t ntsc_broadcast::last_internal_frametime;
int ntsc_broadcast::gframe = 0;
static int linescratch;
uint16_t ntsc_broadcast::framebuffer[((FBW2/4)*(FBH))*2];

const uint32_t * tablestart = &broadcast_tables::premodulated_table[0];
const uint32_t * tablept = &broadcast_tables::premodulated_table[0];
const uint32_t * tableend = &broadcast_tables::premodulated_table[PREMOD_ENTRIES*PREMOD_SIZE];
uint32_t * curdma;

uint8_t pixline; //line number currently being written out.

//Each "qty" is 32 bits, or .4us
 void ntsc_broadcast::fillwith( uint16_t qty, uint8_t color )
{
//	return;

	//We're using this one.
	if( qty & 1 )
	{
		*(curdma++) = tablept[color]; tablept += PREMOD_SIZE;
	}
	qty>>=1;
	for( linescratch = 0; linescratch < qty; linescratch++ )
	{
		*(curdma++) = tablept[color]; tablept += PREMOD_SIZE;
		*(curdma++) = tablept[color]; tablept += PREMOD_SIZE;
		if( tablept >= tableend ) tablept = tablept - tableend + tablestart;
	}
	
}


//XXX TODO: Revisit the length of time the system is at SYNC, BLACK, etc.

 void ntsc_broadcast::FT_STA()
{
	pixline = 0; //Reset the framebuffer out line count (can be done multiple times)

	ntsc_broadcast::fillwith( 6, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 73, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 6, SYNC_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN - (6+73+6), BLACK_LEVEL );
}


 void ntsc_broadcast::FT_STB()
{
	ntsc_broadcast::fillwith( 68, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 11, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 68, SYNC_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN - (68+11+68), BLACK_LEVEL );
}

//Margin at top and bottom of screen (Mostly invisible)
//Closed Captioning would go somewhere in here, I guess?
void ntsc_broadcast::FT_B()
{
	ntsc_broadcast::fillwith( 12, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 2, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 4, COLORBURST_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN-12-6, (pixline<1)?GRAY_LEVEL:WHITE_LEVEL );
	//Gray seems to help sync if at top.  TODO: Investigate if white works even better!
}

 void ntsc_broadcast::FT_SRA()
{
	ntsc_broadcast::fillwith( 6, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 73, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 68, SYNC_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN - (6+73+68), BLACK_LEVEL );
}

 void ntsc_broadcast::FT_SRB()
{
	ntsc_broadcast::fillwith( 68, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 11, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 6, SYNC_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN - (6+11+68), BLACK_LEVEL );
}

 void ntsc_broadcast::FT_LIN()
{
	//TODO: Make this do something useful.
	ntsc_broadcast::fillwith( 12, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 1, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 7, COLORBURST_LEVEL );
	ntsc_broadcast::fillwith( 11, WHITE_LEVEL );
#define HDR_SPD (12+1+7+11)

	int fframe = gframe & 1;
//#define FILLTEST
#ifdef FILLTEST

	ntsc_broadcast::fillwith( FBW/32, BLACK_LEVEL );
	ntsc_broadcast::fillwith( FBW/32, 1 );
	ntsc_broadcast::fillwith( FBW/32, 2 );
	ntsc_broadcast::fillwith( FBW/32, 3 );
	ntsc_broadcast::fillwith( FBW/32, 4 );
	ntsc_broadcast::fillwith( FBW/32, 5 );
	ntsc_broadcast::fillwith( FBW/32, 6 );
	ntsc_broadcast::fillwith( FBW/32, 0 );
	ntsc_broadcast::fillwith( FBW/32, 0 );
	ntsc_broadcast::fillwith( FBW/32, 0 );
	ntsc_broadcast::fillwith( FBW/32, 0 );
	ntsc_broadcast::fillwith( FBW/32, 0 );
	ntsc_broadcast::fillwith( FBW/8, BLACK_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN - (HDR_SPD+FBW/2), BLACK_LEVEL );

#else
	uint16_t * fbs = (uint16_t*)(&framebuffer[ ( (pixline * (FBW2/2)) + ( ((FBW2/2)*(FBH))*(fframe)) ) / 2 ]);

	for( linescratch = 0; linescratch < FBW2/4; linescratch++ )
	{
		uint16_t fbb;
		fbb = fbs[linescratch];
		*(curdma++) = tablept[(fbb>>0)&15];		tablept += PREMOD_SIZE;
		*(curdma++) = tablept[(fbb>>4)&15];		tablept += PREMOD_SIZE;
		*(curdma++) = tablept[(fbb>>8)&15];		tablept += PREMOD_SIZE;
		*(curdma++) = tablept[(fbb>>12)&15];	tablept += PREMOD_SIZE;
		if( tablept >= tableend ) tablept = tablept - tableend + tablestart;
	}

	ntsc_broadcast::fillwith( LINE32LEN - (HDR_SPD+FBW2), WHITE_LEVEL );
#endif

	pixline++;
}

static uint32_t systimex = 0;
static uint32_t systimein = 0;
uint32_t last_internal_frametime;
 void ntsc_broadcast::FT_CLOSE_M()
{
	ntsc_broadcast::fillwith( 12, SYNC_LEVEL );
	ntsc_broadcast::fillwith( 2, BLACK_LEVEL );
	ntsc_broadcast::fillwith( 4, COLORBURST_LEVEL );
	ntsc_broadcast::fillwith( LINE32LEN-12-6, WHITE_LEVEL );
	gline = -1;
	gframe++;

	ntsc_broadcast::last_internal_frametime = systimex;
	systimex = 0;
	systimein = system_get_time();
}

void (*CbTable[FT_MAX_d])() = { ntsc_broadcast::FT_STA, ntsc_broadcast::FT_STB, ntsc_broadcast::FT_B, ntsc_broadcast::FT_SRA, ntsc_broadcast::FT_SRB, ntsc_broadcast::FT_LIN, ntsc_broadcast::FT_CLOSE_M, };

void slc_isr(void * v, void * v2) {
	//portBASE_TYPE HPTaskAwoken=0;
	struct sdio_queue *finishedDesc;
	uint32 slc_intr_status;
	int x;

	slc_intr_status = READ_PERI_REG(SLC_INT_STATUS);
	//clear all intr flags
	WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);//slc_intr_status);
	if ( (slc_intr_status & SLC_RX_EOF_INT_ST))
	{
		//The DMA subsystem is done with this block: Push it on the queue so it can be re-used.
		finishedDesc=(struct sdio_queue*)READ_PERI_REG(SLC_RX_EOF_DES_ADDR);
		curdma = (uint32_t*)finishedDesc->buf_ptr;

		//Allow signal jamming, useful for testing output.
		if( ntsc_broadcast::jam_color < 0 )
		{
			//*startdma = broadcast_tables::premodulated_table[0];
			int k = 0;
			if( gline & 1 )
				k = (CbTable::CbLookup[gline>>1]>>4)&0x0f;
			else
				k = CbTable::CbLookup[gline>>1]&0x0f;
			systimein = system_get_time();
			CbTable[k]();
			systimex += system_get_time() - systimein;
			gline++;
		}
		else
		{
			ntsc_broadcast::fillwith( LINE32LEN, ntsc_broadcast::jam_color );
		}
	}
}

//Initialize I2S subsystem for DMA circular buffer use
 void ICACHE_FLASH_ATTR ntsc_broadcast::testi2s_init() {
	
	int x = 0, y;

	ntsc_broadcast::jam_color = -1;
	/*
	uint32_t endiantest[1] = { 0xAABBCCDD };
	uint8_t * et = (uint8_t*)endiantest;
	printf( "%02x %02x %02x %02x\n", et[0], et[1], et[2], et[3] );
*/

	//Bits are shifted out

	//Initialize DMA buffer descriptors in such a way that they will form a circular
	//buffer.
	for (x=0; x<DMABUFFERDEPTH; x++) {
		i2sBufDesc[x].owner=1;
		i2sBufDesc[x].eof=1;
		i2sBufDesc[x].sub_sof=0;
		i2sBufDesc[x].datalen=I2SDMABUFLEN*4;
		i2sBufDesc[x].blocksize=I2SDMABUFLEN*4;
		i2sBufDesc[x].buf_ptr=(uint32_t)&i2sBD[x*I2SDMABUFLEN];
		i2sBufDesc[x].unused=0;
		i2sBufDesc[x].next_link_ptr=(int)((x<(DMABUFFERDEPTH-1))?(&i2sBufDesc[x+1]):(&i2sBufDesc[0]));
	}


	//Reset DMA
	SET_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);
	CLEAR_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);

	//Clear DMA int flags
	SET_PERI_REG_MASK(SLC_INT_CLR,  0xffffffff);
	CLEAR_PERI_REG_MASK(SLC_INT_CLR,  0xffffffff);

	//Enable and configure DMA
	CLEAR_PERI_REG_MASK(SLC_CONF0, (SLC_MODE<<SLC_MODE_S));
	SET_PERI_REG_MASK(SLC_CONF0,(1<<SLC_MODE_S));
	SET_PERI_REG_MASK(SLC_RX_DSCR_CONF,SLC_INFOR_NO_REPLACE|SLC_TOKEN_NO_REPLACE);
	CLEAR_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_RX_FILL_EN|SLC_RX_EOF_MODE | SLC_RX_FILL_MODE);
	
	//Feed dma the 1st buffer desc addr
	//To send data to the I2S subsystem, counter-intuitively we use the RXLINK part, not the TXLINK as you might
	//expect. The TXLINK part still needs a valid DMA descriptor, even if it's unused: the DMA engine will throw
	//an error at us otherwise. Just feed it any random descriptor.
	CLEAR_PERI_REG_MASK(SLC_TX_LINK,SLC_TXLINK_DESCADDR_MASK);
	SET_PERI_REG_MASK(SLC_TX_LINK, ((uint32)&i2sBufDesc[1]) & SLC_TXLINK_DESCADDR_MASK); //any random desc is OK, we don't use TX but it needs something valid
	CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);
	SET_PERI_REG_MASK(SLC_RX_LINK, ((uint32)&i2sBufDesc[0]) & SLC_RXLINK_DESCADDR_MASK);

	//Attach the DMA interrupt
	//ets_isr_attach(ETS_SLC_INUM, slc_isr, 0);
	ets_isr_attach(ETS_SLC_INUM, slc_isr, 0);
	//Enable DMA operation intr
	WRITE_PERI_REG(SLC_INT_ENA,  SLC_RX_EOF_INT_ENA);
	//clear any interrupt flags that are set
	WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);
	///enable DMA intr in cpu
	ets_isr_unmask(1<<ETS_SLC_INUM);

	//We use a queue to keep track of the DMA buffers that are empty. The ISR will push buffers to the back of the queue,
	//the mp3 decode will pull them from the front and fill them. For ease, the queue will contain *pointers* to the DMA
	//buffers, not the data itself. The queue depth is one smaller than the amount of buffers we have, because there's
	//always a buffer that is being used by the DMA subsystem *right now* and we don't want to be able to write to that
	//simultaneously.
	//dmaQueue=xQueueCreate(I2SDMABUFCNT-1, sizeof(int*));

	//Start transmission
	SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START);
	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START);

//----

	//Init pins to i2s functions
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_I2SO_DATA);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_I2SO_WS);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_I2SO_BCK);

	//Enable clock to i2s subsystem
	i2c_writeReg_Mask_def(i2c_bbpll, i2c_bbpll_en_audio_clock_out, 1);

	//Reset I2S subsystem
	CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);
	SET_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);
	CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);

	//Select 16bits per channel (FIFO_MOD=0), no DMA access (FIFO only)
	CLEAR_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN|(I2S_I2S_RX_FIFO_MOD<<I2S_I2S_RX_FIFO_MOD_S)|(I2S_I2S_TX_FIFO_MOD<<I2S_I2S_TX_FIFO_MOD_S));
	//Enable DMA in i2s subsystem
	SET_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN);

	//tx/rx binaureal
	CLEAR_PERI_REG_MASK(I2SCONF_CHAN, (I2S_TX_CHAN_MOD<<I2S_TX_CHAN_MOD_S)|(I2S_RX_CHAN_MOD<<I2S_RX_CHAN_MOD_S));

	//Clear int
	SET_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
			I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
	CLEAR_PERI_REG_MASK(I2SINT_CLR, I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
			I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);

	//trans master&rece slave,MSB shift,right_first,msb right
	CLEAR_PERI_REG_MASK(I2SCONF, I2S_TRANS_SLAVE_MOD|
						(I2S_BITS_MOD<<I2S_BITS_MOD_S)|
						(I2S_BCK_DIV_NUM <<I2S_BCK_DIV_NUM_S)|
						(I2S_CLKM_DIV_NUM<<I2S_CLKM_DIV_NUM_S));
	SET_PERI_REG_MASK(I2SCONF, I2S_RIGHT_FIRST|I2S_MSB_RIGHT|I2S_RECE_SLAVE_MOD|
						I2S_RECE_MSB_SHIFT|I2S_TRANS_MSB_SHIFT|
						((WS_I2S_BCK&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
						((WS_I2S_DIV&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S));

	//No idea if ints are needed...
	//clear int
	SET_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
			I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
	CLEAR_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
			I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
	//enable int
	SET_PERI_REG_MASK(I2SINT_ENA,   I2S_I2S_TX_REMPTY_INT_ENA|I2S_I2S_TX_WFULL_INT_ENA|
	I2S_I2S_RX_REMPTY_INT_ENA|I2S_I2S_TX_PUT_DATA_INT_ENA|I2S_I2S_RX_TAKE_DATA_INT_ENA);

	//Start transmission
	SET_PERI_REG_MASK(I2SCONF,I2S_I2S_TX_START);
	
}


