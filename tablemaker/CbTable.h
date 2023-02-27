#ifndef CbTable_h
#define CbTable_h
#include <c_types.h>
#include <Arduino.h>
#define FT_STA_d 0
#define FT_STB_d 1
#define FT_B_d 2
#define FT_SRA_d 3
#define FT_SRB_d 4
#define FT_LIN_d 5
#define FT_CLOSE 6
#define FT_MAX_d 7
#define NTSC_LINES 525
class CbTable{
    public:
   
   static uint8_t CbLookup[263];
};
#endif

