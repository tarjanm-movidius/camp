#include "camp.h"

extern unsigned int filenumber;
extern struct playlistent *playlist;
extern struct configstruct config;

int inb (short port)
{
        unsigned char val;
        __asm__ volatile ("inb %1,%0"
			  : "=a" (val)
			  : "d" ((unsigned short)port));
        return val;
}

unsigned char rcpressed(void) {
unsigned char in;

   in = inb(config.rc.port+1);
//   printf("\e[1;1H%d       \n", in);
   
   /* 1 = play
    * 2 = skipR
    * 3 = skipL
    * 4 = stop
    * 5 = vol -
    * 6 = vol +
    */
   
    if ( in == config.rc.play )
      return 1; else
    if ( in == config.rc.skipb )
      return 2; else
    if ( in == config.rc.skipf )
      return 3; else
    if ( in == config.rc.stop )
      return 4; else
    if ( in == config.rc.voldec )
      return 5; else
    if ( in == config.rc.volinc )
      return 6; else
      return 0;      

   /*
   if ( in == 127 ) 
     return 0; else   
   if ( in == 0x3f )
     return 5; else
   if ( in == 0x4f )
     return 1; else
   if ( in == 0x5f )
     return 2; else
   if ( in == 0x6f )
     return 3; else
   if ( in == 0x77 )
     return 6; else
   if ( in == 0xff )
     return 4; else   
   return 0; 
    */
}

void checkrc(void) {
static unsigned char lastvalue = 0;
unsigned char currentvalue = rcpressed();
   
   if ( !lastvalue ) lastvalue = currentvalue;
   
   if ( lastvalue == currentvalue ) return;
   lastvalue = currentvalue;
   
   switch ( currentvalue ) {
      /* 1 = play
       * 2 = skipR
       * 3 = skipL
       * 4 = stop
       * 5 = vol -
       * 6 = vol +
      */
    case 1:
      if ( dofunction(2) ) call_player(pl_seek(filenumber, &playlist));
      break;
    case 2:
      if ( dofunction(3) ) call_player(pl_seek(filenumber, &playlist));
      break;
    case 3:
      if ( dofunction(1) ) call_player(pl_seek(filenumber, &playlist));
      break;
    case 4:
      if ( dofunction(4) ) call_player(pl_seek(filenumber, &playlist));
      break;
#ifdef HAVE_SYS_SOUNDCARD_H
    case 5:
      set_volume(config.voldev, -config.volstep);
      break;
    case 6:
      set_volume(config.voldev, config.volstep);
      break;
#endif
    default: ;
   }
}
   
