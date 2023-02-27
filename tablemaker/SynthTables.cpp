
#include <tablemaker/SynthTables.h>
#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
SynthTables::SynthTables(){}
double eps = 0.01; //Used to help resolve and prevent issues at 0.
double NTSC_Frequency = 315.0/88.0;   //3.579545455 MHz
//To find precise frequency divisors... (1408/80)×67.25(nominal frequency) = 1183.6 ... round up.... 1184/(1408/80) = 67.272727273
double MODULATION_Frequency = CHANNEL_3;
double BIT_Frequency = 80.0;
int samples = 1408;//1408;//+2; //+2 if you want to see   WARNING: This MUST be divisible by 32!
int overshoot = (32*7); //bits of overshoot (continue the table past the end so we don't have to keep checking to make sure it's ok)
void SynthTables::WriteSignal( double LumaValue, double ChromaValue, double Boundary, double ChromaShift, int debug_output, uint32_t * raw_output, uint32_t mask ){
	double t = 0;
	int i;
	int bitplace = 0;
	int byteplace = 0;

	for( i = 0; i < samples+overshoot; i++ )
	{
		double ModV = sin( ( MODULATION_Frequency * t ) * PI2 / 1000.0 + eps );
#ifdef DIRECT_CHROMA_CARRIER_SYNTHESIS
		double ChromaNV = sin( ((NTSC_Frequency+MODULATION_Frequency) * t ) * PI2 / 1000.0 + eps  + ChromaShift * PI2  );
		double Signal = ModV * LumaValue + ChromaNV * ChromaValue + Boundary;
#else
		double ChromaV = sin( (NTSC_Frequency * t ) * PI2 / 1000.0 + eps  + ChromaShift * PI2  );
		double Signal = ModV * (ChromaV*(ChromaValue)+LumaValue) + Boundary;
#endif

		if( Signal > 0 )
			raw_output[byteplace] |= (1<<(31-bitplace)) & mask;
		bitplace++;
		if( bitplace == 32 ) { bitplace = 0; byteplace++; }
		
		if( debug_output )
		{
			fprintf( stderr, "%d\n", ((Signal)>0)?1:-1 );
			fprintf( stderr, "%d\n", ((Signal)>0)?1:-1 );
		}
		t += 1000.0 / BIT_Frequency;
	}
}