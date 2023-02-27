#ifndef _3d_H
#define _3d_H
#include <Arduino.h>
#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
//#include "tablemaker/broadcast_tables.h"
//#include <user/ntsc_broadcast.h>
class d3d{
    public:
    d3d();
static int gframe;
static uint8_t * frontframe;
static int16_t ProjectionMatrix[16];
static int16_t ModelviewMatrix[16];
static int CNFGPenX, CNFGPenY;
static uint8_t CNFGBGColor;
static uint8_t CNFGLastColor ;
static uint8_t CNFGDialogColor; //background for boxes
//static uint16_t *CNFGTackPixel;
//d3d();
static void CNFGTackSegment( int x0, int y0, int x1, int y1 );
static int LABS( int x );

//#define CNFGTackPixelFAST( x, y ) { frontframe[(x+y*FBW)>>2] |= 2<<( (x&3)<<1 ); }  //Store in 4 bits per byte.
static void LocalToScreenspace( int16_t * coords_3v, int16_t * o1, int16_t * o2 );
static int16_t tdSIN( uint8_t iv );
static int16_t tdCOS( uint8_t iv );

/* Colors:
    0 .. 15 = standard-density colors
	16: Black, Double-Density
	17: White, Double-Density
*/
static void CNFGColor( uint8_t col ); 

static void ICACHE_FLASH_ATTR tdTranslate( int16_t * f, int16_t x, int16_t y, int16_t z );		//Operates ON f
static void ICACHE_FLASH_ATTR tdScale( int16_t * f, int16_t x, int16_t y, int16_t z );			//Operates ON f
static void ICACHE_FLASH_ATTR tdRotateEA( int16_t * f, int16_t x, int16_t y, int16_t z );		//Operates ON f

static void ICACHE_FLASH_ATTR CNFGDrawText( const char * text, int scale );
static void ICACHE_FLASH_ATTR CNFGDrawBox(  int x1, int y1, int x2, int y2 );
static void ICACHE_FLASH_ATTR CNFGTackRectangle( short x1, short y1, short x2, short y2 );
static void ICACHE_FLASH_ATTR tdMultiply( int16_t * fin1, int16_t * fin2, int16_t * fout );
static void ICACHE_FLASH_ATTR tdPTransform( int16_t * pin, int16_t * f, int16_t * pout );
static void  td4Transform( int16_t * pin, int16_t * f, int16_t * pout );
static void ICACHE_FLASH_ATTR MakeTranslate( int x, int y, int z, int16_t * out );
static void ICACHE_FLASH_ATTR Perspective( int fovx, int aspect, int zNear, int zFar, int16_t * out );
static void ICACHE_FLASH_ATTR tdIdentity( int16_t * matrix );
static void ICACHE_FLASH_ATTR MakeYRotationMatrix( uint8_t angle, int16_t * f );
static void ICACHE_FLASH_ATTR MakeXRotationMatrix( uint8_t angle, int16_t * f );
static void ICACHE_FLASH_ATTR DrawGeoSphere();
static void ICACHE_FLASH_ATTR Draw3DSegment( int16_t * c1, int16_t * c2 );

static int16_t ICACHE_FLASH_ATTR tdPerlin2D( int16_t x, int16_t y );
static int16_t ICACHE_FLASH_ATTR tdFLerp( int16_t a, int16_t b, int16_t t );
static int16_t ICACHE_FLASH_ATTR tdNoiseAt( int16_t x, int16_t y );
};
#endif

