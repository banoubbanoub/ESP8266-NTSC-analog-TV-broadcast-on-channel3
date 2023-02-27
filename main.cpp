#include <Arduino.h>
#include <user/ntsc_broadcast.h>
#include <user/user_main.h>
#include <user/custom_commands.h>
#include <tablemaker/SynthTables.h>
#include <esp82xx/include/esp82xxutil.h>
#include <tablemaker/MakeCbTables.h>
void setup() {
ntsc_broadcast::testi2s_init();

}
	 
void loop() {
	
	int gframe=1;
int tbuffer = 1;//!(gframe&1);
	int lastframe = 0;

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
	

/*
 #define TABLESIZE 18
 int samples = 1408;//1408;//+2; //+2 if you want to see   WARNING: This MUST be divisible by 32!
int overshoot = (32*7);
	int stride = (samples+overshoot)/32;
double t = 0;	//In ns.
	int debug = 0;
	uint32_t databuffer[stride*TABLESIZE];
	memset( databuffer, 0, sizeof( databuffer ) );
	SynthTables::WriteSignal( 1.0, 0.0, 0.7, 0.0, debug, &databuffer[stride*0], 0xffffffff ); //Black.  //-18.1 db
	SynthTables::WriteSignal( 1.0, 0.0, 1.0, 0.0, debug, &databuffer[stride*2], 0xffff0000 ); //White
	SynthTables::WriteSignal( 1.0, 0.0, 0.7, 0.0, debug, &databuffer[stride*2], 0x0000ffff ); //Black.  //-18.1 db
	SynthTables::WriteSignal( 1.0, 0.0, 1.0, 0.0, debug, &databuffer[stride*8], 0x0000ffff ); //White
	SynthTables::WriteSignal( 1.0, 0.0, 0.7, 0.0, debug, &databuffer[stride*8], 0xffff0000 ); //Black.  //-18.1 db
	SynthTables::WriteSignal( 1.0, 0.0, 1.0, 0.0, debug, &databuffer[stride*10], 0xffffffff ); //White
//BLACK
	SynthTables::WriteSignal( 1.0, 0.0, 0.85, 0.0, debug, &databuffer[stride*1], 0xffffffff );  //GRAY
//BTW
	SynthTables::WriteSignal( 0.7, 1.0, 0.9, 0.0, debug, &databuffer[stride*3], 0xffffffff ); 
	SynthTables::WriteSignal( 0.7, 1.0, 0.9, 0.2, debug, &databuffer[stride*4], 0xffffffff ); 
	SynthTables::WriteSignal( 0.7, 1.0, 0.9, 0.4, debug, &databuffer[stride*5], 0xffffffff ); 
	SynthTables::WriteSignal( 0.7, 1.0, 0.9, 0.6, debug, &databuffer[stride*6], 0xffffffff ); 
	SynthTables::WriteSignal( 0.7, 1.0, 0.9, 0.8, debug, &databuffer[stride*7], 0xffffffff ); 
//WTB
	SynthTables::WriteSignal( 0.4, 0.5, 0.65, 0.0, debug, &databuffer[stride*9], 0xffffffff ); 
//WHITE
	SynthTables::WriteSignal( 0.5, 0.5, 0.8, 0.0, debug, &databuffer[stride*11], 0xffffffff ); 
	SynthTables::WriteSignal( 0.5, 0.5, 0.8, 0.2, debug, &databuffer[stride*12], 0xffffffff ); 
	SynthTables::WriteSignal( 0.5, 0.5, 0.8, 0.4, debug, &databuffer[stride*13], 0xffffffff ); 
	SynthTables::WriteSignal( 0.5, 0.5, 0.8, 0.6, debug, &databuffer[stride*14], 0xffffffff ); 
	SynthTables::WriteSignal( 0.5, 0.5, 0.8, 0.8, debug, &databuffer[stride*15], 0xffffffff ); 

	SynthTables::WriteSignal( 1.2, 0.3, 0.6, 0.0, debug, &databuffer[stride*16], 0xffffffff ); //Chroma.
	SynthTables::WriteSignal( 1.0, 0.0, 0.0, 0.0, debug, &databuffer[stride*17], 0xffffffff ); //Sync Tip.  //-16.3 db
	int i;
	
FILE * f = fopen( "broadcast_tables.cpp", "w" );
	fprintf( f, "#include \"broadcast_tables.h\"\n\n" );
	fprintf( f, "uint32_t premodulated_table[%d] = {", stride * TABLESIZE );
	for( i = 0; i < TABLESIZE*stride; i++ )
	{
		int imod = i % TABLESIZE;
		int idiv = i / TABLESIZE;

//Transposition makes selecting colors easier, but more difficult to get stripes.  Need to test performance.

		//idiv ^= 1; //Invert rows... So we flip our bit orders.  This looks weird, but it fixes our order-of-text.
		uint32_t val = databuffer[imod * stride + idiv];

		if( imod == 0 ) { fprintf( f, "\n\t" ); }
		fprintf( f, "0x%02x, ", (val) );
	}
	fprintf( f, "\n};\n" );
	fclose( f );

	f = fopen( "broadcast_tables.h", "w" );
	fprintf( f, "#include <c_types.h>\n\n" );
	fprintf( f, "#include <esp82xxutil.h>\n\n" );

	fprintf( f, "#define PREMOD_ENTRIES %d\n", samples/32 );
	fprintf( f, "#define PREMOD_ENTRIES_WITH_SPILL %d\n", (samples+overshoot)/32 );
	fprintf( f, "#define PREMOD_SIZE %d\n", TABLESIZE );
	fprintf( f, "#define SYNC_LEVEL 17\n" );
	fprintf( f, "#define COLORBURST_LEVEL 16\n" );
	fprintf( f, "#define BLACK_LEVEL 0\n" );
	fprintf( f, "#define GRAY_LEVEL 1\n" );
	fprintf( f, "#define WHITE_LEVEL 10\n" );
	fprintf( f, "\n" );
	fprintf( f, "extern uint32_t premodulated_table[%d];\n\n", stride * TABLESIZE );
	fclose( f );
  */

}