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
   
   /* 1 = play
    * 2 = skipR
    * 3 = skipL
    * 4 = stop
    * 5 = vol -
    * 6 = vol +
    * 7 = pause
    */
      
   if ( in == config.rc.play )
     return 1; else
     if ( in == config.rc.skipf )
       return 2; else
     if ( in == config.rc.skipb )
       return 3; else
     if ( in == config.rc.stop )
       return 4; else
     if ( in == config.rc.voldec )
       return 5; else
     if ( in == config.rc.volinc )
       return 6; else
     if ( in == config.rc.pause )
       return 7; else
     return 0;      
   
}

void checkrc(void) {
static unsigned char lastvalue = 0;
unsigned char currentvalue = rcpressed();
      
   if ( lastvalue == currentvalue && ( lastvalue < 5 || lastvalue > 6 ) ) return;
   lastvalue = currentvalue;
   
   switch ( currentvalue ) {
    case 0:
      break;
      /* 1 = play
       * 2 = skipF
       * 3 = skipB
       * 4 = stop
       * 5 = vol -
       * 6 = vol +
      */
    case 1:
      if ( dofunction(1) ) call_player(pl_seek(filenumber, &playlist));
      break;
    case 2:
      if ( dofunction(2) ) call_player(pl_seek(filenumber, &playlist));
      break;
    case 3:
      if ( dofunction(0) ) call_player(pl_seek(filenumber, &playlist));
      break;
    case 4:
      (void*)dofunction(3);
      break;
#ifdef HAVE_SYS_SOUNDCARD_H
    case 5:
      set_volume(config.voldev, -config.volstep);
      break;
    case 6:
      set_volume(config.voldev, config.volstep);
      break;
    case 7:
      if ( dofunction(8) ) call_player(pl_seek(filenumber, &playlist));
      break;
#endif
    default: ;
   }
}
   
