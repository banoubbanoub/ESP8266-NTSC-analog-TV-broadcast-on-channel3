//MakeCbTables
#ifndef MakeCbTables_h
#define MakeCbTables_h
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>
#define NTSC_LINES 525
#define FT_STA_d 0
#define FT_STB_d 1
#define FT_B_d 2
#define FT_SRA_d 3
#define FT_SRB_d 4
#define FT_LIN_d 5
#define FT_CLOSE_d 6
#define FT_MAX_d 7
class MakeCbTables
{
    public:
    MakeCbTables();
  static  int MainMethod();
};
#endif