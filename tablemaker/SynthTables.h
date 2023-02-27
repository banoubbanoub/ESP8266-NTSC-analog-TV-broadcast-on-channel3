
#ifndef SynthTables_h
#define SynthTables_h
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <Arduino.h>
#define DIRECT_CHROMA_CARRIER_SYNTHESIS
#define PI2 (3.14159265359*2)
#define CHANNEL_2  55.22727272727  //Actually 55.25, but need to make it line up to the 1408 mark.
#define CHANNEL_3  61.25 //Channel 3 works out perfectly.
#define CHANNEL_4  67.272727273
#define CHANNEL_5  77.25
class SynthTables{
    public:
    SynthTables();
//ChromaValue, LumaValue = 0...??? 
//, ChromaShift, Luma are all 0..1
//Set boundary to a value to normalize Luma + Chroma.  Setting this to 0 will maximize both.
//Note that if ChromaValue == LumaValue, Chroma will not effect the signal.
//
//For all use of this function, I highly recommend checking the output at 160MHz (doubling)
//in an FFT, like http://sooeet.com/math/online-fft-calculator.php
//
//WARNING: These values are sort of inverted, as abscence of signal indicates value!
static void WriteSignal( double LumaValue, double ChromaValue, double Boundary, double ChromaShift, int debug_output, uint32_t * raw_output, uint32_t mask );

};
#endif