#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "camp.h"
#include "chromansi.c"
#include "gui.c"
#include "gui7bit.c"

#ifdef USE_TERMIOS
# include <termios.h>
struct termios oldtermios, newtermios;
#else
# include <curses.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

#ifdef BSD
 const char keys[] = { 'H', 'F', 'I', 'G' };
#else
 const char keys[] = { '1', '4', '5', '6' };
#endif

/*                                                           
 *      Console Ansi Mpeg3 Player interface v1.2 by inm      
 *                                                           
 * If you improve this code, please send a copy, or a        
 * diff of it to inm@m1crosoft.com.                          
 * Latest version can be found at                            
 * http://linux.mccom.net/~inm                               
 *                                                           
 *                                                         */
     

char playsong=TRUE, quiet=FALSE, pausesong=FALSE, force=FALSE, quitmode=0, buttonpos=2, checkkill=FALSE;
struct playlistent *playlist = NULL;
struct oneplaylistent currentfile;
unsigned int slavepid=0, playlistents=0, filenumber=0;
struct configstruct config;

int main(int argc, char *argv[]) {
unsigned int j;
char   buf[100], c, dump=FALSE;
int    ch, cspos=0, i, stdinno = fileno(stdin);
FILE   *fd;
fd_set fds;
struct timeval currenttime;
#ifdef USE_GPM_MOUSE
Gpm_Connect mouse;
#endif   

   printf("Console Ansi Mpeg3 Player interface v%s.%d by inm (inm@m1crosoft.com)\n\n", CAMP_VERSION, BUILD);
   sprintf(buf, "%s/.camp", getenv("HOME"));
   printf("Loading config from %s...\n", buf);
   config = getconfig(buf);
   
   if ( argc != 1 ) {   
      printf("Scanning arguments, please wait..\n");
      for(i=1;i<argc;i++) {
	 if ( !strcasecmp(argv[i], "-d") || !strcasecmp(argv[i], "--dumpid3") )
	   dump = TRUE; else
	   if ( !strcasecmp(argv[i], "-f") || !strcasecmp(argv[i], "--force") )
	     force = TRUE; else
	   if ( !strcasecmp(argv[i], "-p") || !strcasecmp(argv[i], "--playlist") ) {
	      i++;
	      playlist = loadplaylist(playlist, argv[i], FALSE);
	   } else
	   if ( !strcasecmp(argv[i], "-s") || !strcasecmp(argv[i], "--steal") ) {
	      stealback();
	      checkkill = TRUE;
	   } else
	   if ( !strcasecmp(argv[i], "-k") || !strcasecmp(argv[i], "--kill") )
	     killcamp(); else
	   if ( !strcasecmp(argv[i], "-b") || !strcasecmp(argv[i], "--background") ) {
	      quitmode = 1; 
	      quiet = TRUE;
	   } else
	   if ( !strcasecmp(argv[i], "--help") || !strcasecmp(argv[i], "-h") ) {
	      printf("\nUsage: camp [OPTIONS] [FILES]\n\n");
	      printf("Options:\n");
	      printf("  -b,   --background   Forks camp into background directly\n");
	      printf("  -d,   --dumpid3      Dumps id3 tags to stderr\n");
	      printf("  -f,   --force        Force run, even if camp's running. (Won't write pidfile)\n");
	      printf("  -h,   --help         This help\n");
	      printf("  -k,   --kill         Kill running camp session\n");
	      printf("  -p X, --playlist X   Load playlist 'X'\n");
	      printf("  -s,   --steal        \"Steal\" another camp session\n");	      
	      printf("\nAny other parameter will be taken for a regular mp3 file.\n");
	      exit(0);
	   } else
	   playlist = addfiletolist(playlist, argv[i], NULL, 0, 0, 0, TRUE);
      }
   }   

   if ( dump ) {
      printf("Dumping ID3 tags to stdout..\n");
      if ( !playlist ) {
	 fprintf(stderr, "No files to dump tags from!\n");
	 exit(0);
      }
      playlist = pl_seek(0, playlist);
      while ( playlist->next ) {
	 fprintf(stderr, "%-5d %s - %s\n", playlist->number, playlist->showname);
	 playlist = playlist->next;
      }
      printf("Done!\n");
      exit(0);      
   }
   
   if ( !checkkill && exist(PID_FILE) ) {
      fd = fopen(PID_FILE, "r");
      fscanf(fd, "%u\n", &j);
      fclose(fd);
      if ( !kill(j, 0) ) {
	 if ( force )
	   printf("Another camp session is already running (--force specified, overriding kill).\n");
	 else { 
	    printf("Camp is already running!\nUse camp -s to steal running process, or camp -k to kill it.\n");
	    if ( playlist != NULL ) clearplaylist(playlist);
	    exit(0);
	 }
      }
   }
   
   if ( !playlist )
     playsong = FALSE;
   
   sleep(1);
   myinit();
#ifdef USE_GPM_MOUSE   
   my_Gpm_Init(&mouse);
#endif
   atexit(myexit);
   
   if ( playlist && playsong ) {
      playlistents = pl_count(playlist);
      if ( !slavepid ) {
	 if ( config.playmode == 2 ) filenumber = myrand(playlistents);
	 call_player(pl_seek(filenumber, playlist));
      } else {
	 signal(SIGCHLD, playnext);
	 updatedata();
      }       
   } else {
      updatebuttons(4);
      updatedata();
   }
   
   if ( quitmode == 1 ) disappear();
   
   while ( TRUE ) { /* main loop */
         
      if ( playsong && !pausesong )
	updatesongtime('u');
      
      if ( checkkill && kill(slavepid, 0) == -1 ) {
	 checkkill = FALSE;
	 playnext(0);
      }
            
      currenttime.tv_usec = 0;
      currenttime.tv_sec  = 1;
      FD_ZERO(&fds);
      FD_SET(stdinno, &fds);
#ifdef USE_GPM_MOUSE
      if ( gpm_flag ) FD_SET(gpm_fd, &fds);
      if ( select(gpm_flag ? gpm_fd+1 : stdinno+1, &fds, NULL, NULL, &currenttime) > 0 ) {
	 if ( FD_ISSET(gpm_fd, &fds) && gpm_flag ) main_domouse();
#else
      if ( select(stdinno+1, &fds, NULL, NULL, &currenttime) > 0 ) {
#endif
	 ch=0;
	 if ( FD_ISSET(stdinno, &fds) ) 
	   while ( ch != -1 ) {
	      ch = getchar();
	      if (ch == 27)   escfix(); else
		if (ch == 3)  exit(0);  else    /* ^C */
		if (ch == 26) disappear(); else /* ^Z */
		if (ch == 12) {                 /* ^L */
		   if ( config.ttymode == 8 )
		     printf("\e[0m\e[2J\e[1;1H%s", screendata); else
		     printf("\e[0m\e[2J\e[1;1H%s", screendata7bit);
		   updatedata();
		   updatebuttons(0);
		} else
		if (ch == 13 && dofunction(0)) call_player(pl_seek(filenumber, playlist));
	   } /* while ch != .. */
      } /* select */
   } /* while(endless) */   
   return 0;
} /* main() */

   
   //     Procedures & functions             //


void updatebuttons(int add) {

   buttonpos += add;
   if ( buttonpos == 8    && add == 7 )  buttonpos = 9; else 
     if ( buttonpos == 10 && add == 7 )  buttonpos = 9; else
     if ( buttonpos == 10 && add == -1 )  buttonpos = 9; else
     if ( buttonpos == 10 && add == 1 )  buttonpos = 11; else
     if ( buttonpos == 9 && add == -1 ) buttonpos = 8; else 
     if ( buttonpos == 8  && add == -1 ) buttonpos = 9; else
     if ( buttonpos == 8  && add == 1 ) buttonpos = 7; 
     /* phear me =) */
   
   printf("\e[12;14H");
   if (buttonpos == 1)	printf("\e[0;44;36;1m"); else
     printf("\e[0;36;44m");
   if ( config.ttymode == 8 )
     printf("  ®®  "); else
     printf("  <<  ");   
   printf("\e[12;22H");
   if (buttonpos == 2)	printf("\e[44;36;1m"); else
     printf("\e[44;34;1m");
   if ( config.ttymode == 8 )
     printf("  ¯¯  "); else
     printf("  >>  ");
   printf("\e[12;30H");
   if (buttonpos == 3)	printf("\e[44;36;1m"); else
     printf("\e[0;36;44m");
   if ( config.ttymode == 8 )
     printf("  ¯¯  "); else
     printf("  >>  ");     
   printf("\e[12;38H");
   if (buttonpos == 4)	printf("\e[44;36;1m"); else
     printf("\e[44;34;1m");
   if ( config.ttymode == 8 )
     printf("  þþ  "); else
     printf(" stop ");
   printf("\e[12;46H");
   if (buttonpos == 5)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   if ( config.ttymode == 8 )
     printf("  ®¯  "); else
     printf(" mode ");
   printf("\e[12;54H");
   if (buttonpos == 6)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  pl  ");
   printf("\e[12;62H");
   if (buttonpos == 7)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" %s ", CUSTOM_BUTTON);

   /* 2:nd row */
   
  /* printf("\e[12;14H");
   if (buttonpos == 8)	printf("\e[0;44;36;1m"); else
     printf("\e[0;36;44m");
   printf("  ®®  "); */
   printf("\e[14;22H");
   if (buttonpos == 9)	printf("\e[44;36;1m"); else
     printf("\e[44;34;1m");
   printf("  ||  ");
/*   printf("\e[14;30H");
   if (buttonpos == 10)	printf("\e[44;36;1m"); else
     printf("\e[0;36;44m");
   printf("  ¯¯  "); */
   printf("\e[14;38H");
   if (buttonpos == 11)	printf("\e[44;36;1m"); else
     printf("\e[44;34;1m");
   printf(" jump ");
   printf("\e[14;46H");
   if (buttonpos == 12)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" lock ");
   printf("\e[14;54H");
   if (buttonpos == 13)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" desc ");
   printf("\e[14;62H");
   if (buttonpos == 14)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" fork ");

   fflush(stdout);
}

void myinit(void) {
FILE *fd;
   if ( !force ) {
      fd = fopen(PID_FILE, "w");
      fprintf(fd, "%d\n", getpid());
      fclose(fd);
   }
   signal(SIGUSR1, sigusr1);
#ifdef USE_TERMIOS
   tcgetattr(fileno(stdin), &oldtermios);
   memcpy(&newtermios, &oldtermios, sizeof(struct termios));
   termios_raw(&newtermios);
   tcsetattr(fileno(stdin), TCSANOW, &newtermios);
   fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
#else
   savetty();
   noecho(); 
   nonl(); 
   cbreak();
#endif
   if ( !quiet ) {
      if ( config.ttymode == 8 )
	printf("\e[0m\e[2J\e(U\e[?25l%s", screendata); else
	printf("\e[0m\e[2J\e[?25l%s", screendata7bit);		   
      updatebuttons(0);
   }
}

void myexit(void) {
   if ( quitmode == 0 ) /* 0 = normal, 1 = forking, 2 = "forked-steal" quit  3 = "steal" quit*/
      if ( playsong ) killslave();  
   if ( playlist != NULL ) clearplaylist(playlist);
#ifdef USE_TERMIOS
   fcntl(fileno(stdin), F_SETFL, 0);
   tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
#else
   resetty();
#endif
   
#ifdef USE_GPM_MOUSE   
   while(Gpm_Close());
#endif

   if ( quitmode != 2 ) {
      printf("\e[0m\e[2J\e[1;1H%s", chromansi);
      printf("\e[0mConsole Ansi Mpeg3 Player interface %s.%d by inm (inm@m1crosoft.com)\n", CAMP_VERSION, BUILD);
      printf("\e[?25h");
   }
   
   switch ( quitmode ) {
    case 1: printf("Forked into the background.\n"); break;
    case 3: printf("Argh! Someone stole me!\n"); break;
    default: if ( !force ) unlink(PID_FILE); 
   }
}


void escfix(void) {
   
   int ch1, ch2;

   ch1 = getchar();
   if ( ch1 == -1 ) {
      mykbhit(0, 250000);
      ch1 = getchar();
   }
   ch2 = getchar();
   
   if ( ch1 == -1 || ch1 == 27 ) exit(0); else
     if ( ch1 == '[' )
       if (( ch2 == 'C' ) && ( buttonpos != MAXBUTTON )) updatebuttons(1);  else
     if (( ch2 == 'D' ) && ( buttonpos != MINBUTTON )) updatebuttons(-1); else
     if (( ch2 == 'A' ) && ( buttonpos > 7 )) updatebuttons(-7); else
     if (( ch2 == 'B' ) && ( buttonpos < 8 )) updatebuttons(7); 
}


int dofunction(unsigned char forcedbutton) {
static struct timeval pause_start, pause_end;
char pass[16], checkpass[16];
int mod, ec, oldfilenumber;
FILE *fd;
   
   switch( forcedbutton ? forcedbutton : buttonpos ) {
      
    case 1: /* skip back */
      if ( filenumber == 0 ) filenumber = playlistents-1; else
	filenumber--;
      if ( playsong == TRUE && slavepid != 0 ) killslave(); else
	if ( !playsong && config.timemode && playlist && !quiet)
	  printf("\e[1;34;46m\e[8;16H%02d:%02d", pl_seek(filenumber, playlist)->length/60, pl_seek(filenumber, playlist)->length%60 );
      return(1);
      
    case 2: /* play */
      if ( playlist == NULL ) return(0);
      if ( pausesong )  {
	 kill(slavepid, SIGCONT);
	 pausesong = FALSE;	 
	 updatesongtime('e');
	 return(0);
      }	    
      if ( config.playmode == 2 && playsong == TRUE ) filenumber = myrand(playlistents);
      if ( playsong == TRUE && slavepid != 0) killslave();
      playsong = TRUE;
      return(1);
      
    case 3: /* skip fwd */
      if ( filenumber >= playlistents-1 ) filenumber = 0; else
	filenumber++;
      if ( playsong == TRUE && slavepid != 0) killslave(); else
	if ( !playsong && config.timemode && playlist )
	  printf("\e[1;34;46m\e[8;16H%02d:%02d", pl_seek(filenumber, playlist)->length/60, pl_seek(filenumber, playlist)->length%60 );
      return(1);
      
    case 4: /* stop */
      if ( playsong && slavepid != 0) {
	 playsong = FALSE;
	 killslave();
	 if ( !config.timemode )
	   printf("\e[1;34;46m\e[8;16H00:00"); else
	   printf("\e[1;34;46m\e[8;16H%02d:%02d", currentfile.length/60, currentfile.length%60 );
	 fflush(stdout);
      }
      break;
      
    case 5: /*playmode */
      if ( config.playmode == 0 || config.playmode == 1 )
	config.playmode++; else config.playmode = 0;
      updatedata();
      break;

    case 6: /* playlist */
      quiet = TRUE;
      playlist = rplaylist(playlist, &filenumber);
      playlistents = pl_count(playlist);
      if ( config.ttymode == 8 )
	printf("\e[0m\e[2J%s", screendata); else
	printf("\e[0m\e[2J%s", screendata7bit); 
	fflush(stdout);
      updatedata();
      updatebuttons(0);
      quiet = FALSE;
      if ( !playsong && config.timemode && playlist && !quiet)
	printf("\e[1;34;46m\e[8;16H%02d:%02d", pl_seek(filenumber, playlist)->length/60, pl_seek(filenumber, playlist)->length%60 );
      break;
      
    case 7: /* custom button */
      signal(SIGCHLD, SIG_IGN);
      printf("\e[0m\e[2J\e[1;1H"); 
      fflush(stdout);
#ifdef USE_TERMIOS
      tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
      fcntl(fileno(stdin), F_SETFL, 0);
#else
      resetty();
#endif
      system(CUSTOM_RUN);
      myinit();
      signal(SIGCHLD, playnext);
      if ( kill(slavepid, 0) != 0 ) playnext(0); else
	if ( !playsong && config.timemode && playlist )
	  printf("\e[1;34;46m\e[8;16H%02d:%02d", currentfile.length/60, currentfile.length%60 );
      updatedata();
      updatebuttons(0);
      fflush(stdout);
      break;
   
    case 9: /* pause */
      if ( slavepid != 0 ) {
	 if ( !pausesong ) {
	    kill(slavepid, SIGSTOP);
	    pausesong = TRUE;
	    updatesongtime('p');
	 } else {
	    kill(slavepid, SIGCONT);
	    pausesong = FALSE;
	    updatesongtime('e');
	 }
      }
      break;

    case 11:
      oldfilenumber = filenumber;
      pass[0] = 0;
      do {
	 printf("\e[1;34;46m\e[9;16H                                                  \e[9;16HJump to song [1-%d]:\e[0;34;46m", playlistents);
	 mod = 27;
	 filenumber = atoi(readyxline(9, 38, pass, 4, &ec, &mod));
	 sprintf(pass, "%d", filenumber); 
	 if ( filenumber == 0 || ec == 27 || !mod ) { filenumber = -1; break; }
	 filenumber--;
      } while( filenumber < 0 || filenumber > playlistents-1 );
      if ( filenumber != -1 ) call_player(pl_seek(filenumber, playlist)); else {
	 filenumber = oldfilenumber;
	 updatedata();
      }
      break;
      
    case 12: /* lock */
      printf("\e[1;34;46m\e[12;14HEnter  password:                                      ");
      printf("\e[14;14HRetype password:                                      ");
      printf("\e[12;31H\e[0;46;34m");
      readpass(pass, 15);
      printf("\e[14;31H");
      readpass(checkpass, 15);
      printf("\e[1;34;46m\e[12;14HConsole locked                                       ");
      printf("\e[14;14HUnlock password:                                      ");
      fflush(stdout);
      if ( !strcmp(pass, checkpass) ) {
	 do {
	    while ( !mykbhit(1, 0) )
	      updatesongtime('u');
	    printf("\e[14;31H\e[0;46;34m");
	    readpass(checkpass, 15);
	    printf("\e[14;31H               \e[14;31H\e[0;46;34m"); fflush(stdout);
	 } while ( strcmp(pass, checkpass) );
      }
      printf("\e[1;34;46m\e[12;14H                                                     ");
      printf("\e[14;14H                                                     ");
      updatebuttons(0);
      if ( !playsong && config.timemode && playlist )
	printf("\e[1;34;46m\e[8;16H%02d:%02d", currentfile.length/60, currentfile.length%60 );
      break;
      
    case 13: /* desc edit */
      if ( playlist == NULL ) return;
      quiet = TRUE;
      id3edit(pl_seek(filenumber, playlist)->name, pl_seek(filenumber, playlist));
      if ( config.ttymode == 8 )
	printf("%s", screendata); else
	printf("%s", screendata7bit);
      updatedata();
      updatebuttons(0);
      if ( !playsong && config.timemode && playlist )
	printf("\e[1;34;46m\e[8;16H%02d:%02d", currentfile.length/60, currentfile.length%60 );
      quiet = FALSE;
      break;

    case 14:
      disappear();
      break;
      
   }   
   return(0);
}

#ifdef USE_GPM_MOUSE
void main_domouse(void) {
Gpm_Event event;
char newbuttonpos = 0;
   
   Gpm_GetEvent(&event);
   
   /* middle button */
   if ( event.buttons & 2 && event.type & GPM_DOWN ) {
      if ( config.ttymode == 8 )
	printf("\e[0m\e[2J\e[1;1H%s", screendata); else
	printf("\e[0m\e[2J\e[1;1H%s", screendata7bit);
      updatedata();
      updatebuttons(0);
      event.buttons = 255;
   } 
   /* left button */
   if ( event.buttons & 4 && event.type & GPM_DOWN ) {
      if ( event.y == 8 ) {
	 if ( event.x > 14 && event.x < 22 ) { 
	    if ( config.timemode ) 
	      config.timemode = FALSE; else
	      config.timemode = TRUE;
	    updatesongtime('f');
	 } else
	   if ( event.x > 56 && event.x < 61 ) dofunction(5); /*mode*/ 
      } else
	if ( event.y == 12 ) {
	   if ( event.x > 13 && event.x < 20 ) newbuttonpos = 1; else
	     if ( event.x > 21 && event.x < 28 ) newbuttonpos = 2; else
	     if ( event.x > 29 && event.x < 36 ) newbuttonpos = 3; else
	     if ( event.x > 37 && event.x < 44 ) newbuttonpos = 4; else
	     if ( event.x > 45 && event.x < 52 ) newbuttonpos = 5; else
	     if ( event.x > 53 && event.x < 60 ) newbuttonpos = 6; else
	     if ( event.x > 61 && event.x < 68 ) newbuttonpos = 7;
	} else
	if ( event.y == 14 ) {
	   /*	   if ( event.x > 13 && event.x < 20 ) newbuttonpos = 8; else */
	   if ( event.x > 21 && event.x < 28 ) newbuttonpos = 9; else
	     /*	     if ( event.x > 29 && event.x < 36 ) newbuttonpos = 10; else */
	     if ( event.x > 37 && event.x < 44 ) newbuttonpos = 11; else
	     if ( event.x > 45 && event.x < 52 ) newbuttonpos = 12; else
	     if ( event.x > 53 && event.x < 60 ) newbuttonpos = 13; else
	     if ( event.x > 61 && event.x < 68 ) dofunction(14);
	}
   }
   if ( newbuttonpos ) {
      buttonpos = newbuttonpos;
      updatebuttons(0);
      if ( dofunction(0) )
	call_player(pl_seek(filenumber, playlist));
   } else {
      Gpm_GetSnapshot(&event);
      if ( !event.y ) event.y++;
      if ( !event.x ) event.x++;
      GPM_DRAWPOINTER(&event);
   }
}
#endif   
   
void updatedata() {
struct timeval currenttime;

   if ( currentfile.showname[0] != 0 ) {
      if ( config.showtime == 3 || config.showtime == 1 ) printf("\e[1;34;46m\e[9;16H                                                  \e[9;16H[%02u:%02u] %s\n", currentfile.length / 60, currentfile.length % 60, currentfile.showname); else
	printf("\e[1;34;46m\e[9;16H                                                  \e[9;16H%s\n", currentfile.showname);
      printf("\e[8;25H\e[0;34;46m%4d/%-4d\e[8;38H%5d\e[8;46H%3d\e[8;63H%s", filenumber+1, playlistents, currentfile.samplerate, currentfile.bitrate, currentfile.mode ? "st" : "mo");
   }
   if ( playsong ) updatesongtime('u');
   printf("\e[8;57H\e[0;34;46m");
   switch ( config.playmode ) {
    case 0: printf("norm"); break;
    case 1: printf("loop"); break;
    case 2: printf("rand"); break;
   } fflush(stdout);
}

void updatesongtime(char ch) {
static struct timeval starttime, pause_start, pause_end, oldcurrenttime;
struct timeval currenttime;
int reverse_time;   
FILE *fd;
char buf[256];
   
   if ( quiet && ( ch == 'u' || ch == 'f' ) ) return;
   
   switch(ch) {
      
    case 'i':
      gettimeofday(&starttime, NULL);
      break;
      
    case 'u':
    case 'f':
      gettimeofday(&currenttime, NULL);
      if ( ch == 'u' && oldcurrenttime.tv_sec == currenttime.tv_sec ) return;
      /* guess this is the best way of doing this, since we don't want the player to type this,. */
      if ( config.timemode ) {
	 reverse_time = currentfile.length-(currenttime.tv_sec-starttime.tv_sec);
	 if ( reverse_time < 0 ) reverse_time = 0;
	 printf("\e[1;34;46m\e[8;16H%02d:%02d", reverse_time/60, reverse_time%60);
      } else
      printf("\e[1;34;46m\e[8;16H%02d:%02d", (currenttime.tv_sec-starttime.tv_sec)/60, (currenttime.tv_sec-starttime.tv_sec)%60);
      fflush(stdout);
      oldcurrenttime.tv_sec = currenttime.tv_sec;
      break;
   
    case 'e':
      gettimeofday(&pause_end, NULL);
      starttime.tv_sec += pause_end.tv_sec - pause_start.tv_sec;
      break;
      
    case 'p':
      gettimeofday(&pause_start, NULL);
      break;
      
    case 'w':
      sprintf(buf, "%s/time.camp", TMP_DIR);
      fd = fopen(buf, "w");
      fprintf(fd, "%u\n%u\n", starttime.tv_sec, starttime.tv_usec);
      fclose(fd);
      break;

    case 'r':
      sprintf(buf, "%s/time.camp", TMP_DIR);
      fd = fopen(buf, "r");
      if ( !fd ) break;
      fscanf(fd, "%u\n%u\n", &starttime.tv_sec, &starttime.tv_usec);
      fclose(fd);
      unlink(buf);
      break;
   }

}
