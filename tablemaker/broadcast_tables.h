#ifndef BroadCastTables_h
#define BroadCastTables_h
#include <c_types.h>
#include <Arduino.h>
//#include <esp82xx/include/esp82xxutil.h>

#define PREMOD_ENTRIES 44
#define PREMOD_ENTRIES_WITH_SPILL 51
#define PREMOD_SIZE 18
#define SYNC_LEVEL 17
#define COLORBURST_LEVEL 16
#define BLACK_LEVEL 0
#define GRAY_LEVEL 1
#define WHITE_LEVEL 10
class broadcast_tables
{

public:
    broadcast_tables(/* args */);
    static uint32_t premodulated_table[918];
};
#endif




