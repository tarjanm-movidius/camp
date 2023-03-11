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
#ifdef USE_TERMIOS
# include <termios.h>
struct termios oldtermios, newtermios;
#else
# include <curses.h>
#endif


/*                                                           
 *      Console Ansi Mpeg3 Player interface v1.0 by inm      
 *                                                           
 * If you improve this code, please send a copy, or a        
 * diff of it to inm@sector7.kracked.com.                    
 * Latest version can always be found at                     
 * http://www.sector7.kracked.com                            
 *                                                           
 * 1998-04-16: v1.0.633 Released                             
 *                                                           
 * 1998-05-01: v1.0.700 Released                             
 * Bugs fixed.                                               
 * Added "jump" button                                       
 *                                                           
 * TODO:                                                     
 * Switch to disable 8bit chars.                             
 *                                                         */
     

/* wh0ps .. loads of globals. I'll optimize this shit later */
int  i, buttonpos=2;
char playsong=TRUE, quiet=FALSE, pausesong=FALSE;
char showname[100], playlistname[19];
struct playlistent *playlist = NULL;
unsigned int slavepid=0, playlistents=0, filenumber=0, fs=0, bitrate=0, mode=0;
struct configstruct config;

int main(int argc, char *argv[]) {
char   c;
int    j, ch, cspos=0;
FILE   *fd;
fd_set stdinfds;
struct timeval currenttime;
     
   printf("Console Ansi Mpeg3 Player interface v%s.%d by inm (inm@sector7.kracked.com)\n\n", CAMP_VERSION, BUILD);
   sprintf(showname, "%s/.camp", getenv("HOME"));
   printf("Loading config from %s...\n", showname);
   config = getconfig(showname);
   
   strcpy(playlistname, "misc. files loaded");

   if ( argc != 1 ) {   
      printf("Scanning arguments, please wait..\n");
      for(i=1;i<argc;i++) {
	 if ( !strcasecmp(argv[i], "-p") || !strcasecmp(argv[i], "--playlist") ) {
	    i++;
	    playlist = loadplaylist(playlist, argv[i], FALSE);
	 } else
	   if ( !strcasecmp(argv[i], "--help") || !strcasecmp(argv[i], "-h") ) {
	      printf("\nUsage: camp [OPTIONS] [FILES]\n\n");
	      printf("Options:\n");
	      printf("  -h,   --help         This help.\n");
	      printf("  -p X, --playlist X   Load playlist 'X'.\n\n");
	      printf("Any other filename will be taken for a regular mp3 file.\n");
	      exit(0);
	   } else
	   playlist = addfiletolist(playlist, argv[i], NULL, 0, 0, 0, TRUE);	 
      }
   }   
   
   if ( playlist == NULL ) {
      strcpy(playlistname, "no playlist loaded");
      playsong = FALSE;
   }
   
   sleep(1);
   myinit();
   atexit(myexit);
   
   if ( playlist != NULL && playsong == TRUE ) {
      playlistents = pl_seek(65000, playlist)->number;
      if ( config.playmode == 2 ) filenumber = myrand(playlistents);
      call_player(pl_seek(filenumber, playlist));
   } else
     updatebuttons(4);
   
   
   while ( TRUE ) {
      
      if ( playsong && !pausesong ) {
	 updatesongtime('u');
      }
      
      currenttime.tv_usec = 0;
      currenttime.tv_sec  = 1;
      FD_ZERO(&stdinfds);
      FD_SET(0, &stdinfds); 
      if ( select(1, &stdinfds, NULL, NULL, &currenttime) ) {
	 ch=0;
	 while ( ch != -1 ) {
	    ch = getchar();
	    if (ch == 27)   escfix(); else
	      if (ch == 3)  exit(0);  else /* ^C */
	      if (ch == 12) { /* ^L */
		 printf("\e[0m\e[2J\e[1;1H%s", screendata);
		 updatedata();
		 updatebuttons(0);
	      } else
	      if (ch == 13 && dofunction()) call_player(pl_seek(filenumber, playlist));
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
     if ( buttonpos == 8  && add == 1 ) buttonpos = 7; else
     if ( buttonpos == 14  && add == 7 ) buttonpos = 13; else
     if ( buttonpos == 14  && add == 1 ) buttonpos = 13; 
     /* phear me =) */
   
   printf("\e[12;14H");
   if (buttonpos == 1)	printf("\e[0;44;36;1m"); else
     printf("\e[0;36;44m");
   printf("  ®®  ");
   printf("\e[12;22H");
   if (buttonpos == 2)	printf("\e[44;36;1m"); else
     printf("\e[44;34;1m");
   printf("  ¯¯  ");
   printf("\e[12;30H");
   if (buttonpos == 3)	printf("\e[44;36;1m"); else
     printf("\e[0;36;44m");
   printf("  ¯¯  ");
   printf("\e[12;38H");
   if (buttonpos == 4)	printf("\e[44;36;1m"); else
     printf("\e[44;34;1m");
   printf("  þþ  ");
   printf("\e[12;46H");
   if (buttonpos == 5)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  ®¯  ");
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
/*   printf("\e[14;62H");
   if (buttonpos == 14)	printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" help ");
*/
   fflush(stdout);
}

void myinit(void) {
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
   printf("\e[0m\e[2J\e(U\e[?25l%s", screendata);
   updatebuttons(0);
}

void myexit(void) {
   
   if ( playlist != NULL ) clearplaylist(playlist);
   if ( playsong ) killslave();  
#ifdef USE_TERMIOS
   fcntl(fileno(stdin), F_SETFL, 0);
   tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
#else
   resetty();
#endif
   printf("\e[0m\e[2J\e[1;1H%s", chromansi);
   printf("\e[0mConsole Ansi Mpeg3 Player interface %s.%d by inm (inm@sector7.kracked.com)\n", CAMP_VERSION, BUILD);
   printf("Compiled for (%s@%s) %s-%s %s\n", COMPILEUSER, HOSTNAME, OS, ARCH, RELEASE);
   printf("\e[?25h");
}


void escfix(void) {
   
   int ch1, ch2;

   ch1 = getchar();
   ch2 = getchar();
   
   if ( ch1 == -1 ) exit(0); else
     if ( ch1 == '[' )
       if (( ch2 == 'C' ) && ( buttonpos != MAXBUTTON )) updatebuttons(1);  else
     if (( ch2 == 'D' ) && ( buttonpos != MINBUTTON )) updatebuttons(-1); else
     if (( ch2 == 'A' ) && ( buttonpos > 7 )) updatebuttons(-7); else
     if (( ch2 == 'B' ) && ( buttonpos < 8 )) updatebuttons(7); 
}


int dofunction(void) {
static struct timeval pause_start, pause_end;
char pass[16], checkpass[16];
int mod, ec, oldfilenumber;
FILE *fd;
   
   switch( buttonpos ) {
      
    case 1: /* skip back */
      if ( filenumber == 0 ) filenumber = playlistents-1; else
	filenumber--;
      if ( playsong == TRUE && slavepid != 0 ) killslave();
      return(1);
      
    case 2: /* play */
      if ( playlist == NULL ) return(0);
      if ( pausesong )  {
	 kill(slavepid, SIGCONT);
	 pausesong = FALSE;
	 updatesongtime('e');
	 return(0);
      }	    
      if ( playsong == TRUE && slavepid != 0) killslave();
      if ( config.playmode == 2 && playsong == TRUE ) filenumber = myrand(playlistents);
      playsong = TRUE;
      return(1);
      
    case 3: /* skip fwd */
      if ( filenumber >= playlistents-1 ) filenumber = 0; else
	filenumber++;
      if ( playsong == TRUE && slavepid != 0) killslave();
      return(1);
      
    case 4: /* stop */
      if ( playsong && slavepid != 0) {
	 playsong = FALSE;
	 killslave();
	 printf("\e[1;34;46m\e[8;16H00:00"); fflush(stdout);
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
      printf("\e[0m\e[2J%s", screendata); fflush(stdout);
      updatedata();
      updatebuttons(0);
      quiet = FALSE;
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
      if ( kill(slavepid, SIGCHLD) == -1 ) kill(getpid(), SIGCHLD);
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
	 /* updatebuttons(0); */
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
	    while ( !mykbhit(1) )
	      updatesongtime('u');
	    printf("\e[14;31H\e[0;46;34m");
	    readpass(checkpass, 15);
	    printf("\e[14;31H               \e[14;31H\e[0;46;34m"); fflush(stdout);
	 } while ( strcmp(pass, checkpass) );
      }
      printf("\e[1;34;46m\e[12;14H                                                     ");
      printf("\e[14;14H                                                     ");
      updatebuttons(0);
      break;
      
    case 13: /* desc edit */
      quiet = TRUE;
      id3edit(pl_seek(filenumber, playlist)->name, pl_seek(filenumber, playlist));
      printf("%s", screendata);
      updatedata();
      updatebuttons(0);
      quiet = FALSE;
      break;
      
   }   
   return(0);
}


void updatedata() {
struct timeval currenttime;
   printf("\e[1;34;46m\e[9;16H                                                  \e[9;16H%s\n", showname);
   printf("\e[8;25H\e[0;34;46m%4d/%-4d\e[8;38H%5d\e[8;46H%3d\e[8;63H%s", filenumber+1, playlistents, fs, bitrate, mode ? "st" : "mo");
   if ( playsong ) updatesongtime('u');
   printf("\e[8;57H\e[0;34;46m");
   switch ( config.playmode ) {
    case 0: printf("norm"); break;
    case 1: printf("loop"); break;
    case 2: printf("rand"); break;
   } fflush(stdout);
}

void updatesongtime(char ch) {
static struct timeval starttime, pause_start, pause_end;
       struct timeval currenttime;
   
   switch(ch) {
      
    case 'i':
      gettimeofday(&starttime, NULL);
      break;
      
    case 'u':
      gettimeofday(&currenttime, NULL);
      /* guess this is the best way of doing this, since we don't want the player to type this,. */
      printf("\e[1;34;46m\e[8;16H%02d:%02d", (currenttime.tv_sec-starttime.tv_sec)/60, (currenttime.tv_sec-starttime.tv_sec)%60); fflush(stdout);
      break;
   
    case 'e':
      gettimeofday(&pause_end, NULL);
      starttime.tv_sec += pause_end.tv_sec - pause_start.tv_sec;
      break;
      
    case 'p':
      gettimeofday(&pause_start, NULL);
      break;
      
   }
   
}

void termios_raw(struct termios *termios_p) {
   termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
   termios_p->c_oflag &= ~OPOST;
   termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
   termios_p->c_cflag &= ~(CSIZE|PARENB);
   termios_p->c_cflag |= CS8;
}
