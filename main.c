#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/vt.h>
#include "camp.h"
#include "chromansi.c"

#ifdef HAVE_TERMIOS_H
# include <fcntl.h>
# include <termios.h>
struct termios oldtermios, newtermios;
#else
# include <curses.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

#ifdef LIRCD
#include "lirc_client.h"
#endif

/*                                                      
 *   Console Ansi Mpeg3 Player interface v1.3 by inm    
 *                                                      
 * If you improve this code, please send a copy, or a   
 * diff of it to inm@m1crosoft.com.                     
 * Latest version can be found at                       
 * http://camp.goes.berzerk.nu                          
 *                                                      
 *                                                    */

#define KM_UP     1
#define KM_DOWN   2
#define KM_LEFT   3
#define KM_RIGHT  4
#define KM_NONE   0

 const char keys[] = { '1', '4', '5', '6' };
/* const char keys[] = { 'H', 'F', 'I', 'G' }; */

char playsong=TRUE, quiet=FALSE, pausesong=FALSE, force=FALSE, checkkill=FALSE, quitmode=0, buttonpos, muted=FALSE, use_lircd=TRUE, alwayslocked=FALSE;
char passwd[15];
struct playlistent *playlist = NULL;
struct oneplaylistent currentfile;
unsigned int slavepid=0, playlistents=0, filenumber=0;
int selval;
struct configstruct config;
#ifdef USE_GPM_MOUSE
Gpm_Connect mouse;
#endif   

#ifdef LIRCD
char *progname = "camp";
extern int lirc_lircd;
struct lirc_config *lircd_config = NULL;
#endif

int main(int argc, char *argv[]) {
unsigned int j;
char   buf[100], c, dump=FALSE;
int    ch, cspos=0, i, stdinno = fileno(stdin);
FILE   *fd;
fd_set fds;
struct timeval currenttime;
int left, right;   
   
   printf("Console Ansi Mpeg3 Player interface v%s.%d by inm (inm@m1crosoft.com)\n\n", CAMP_VERSION, BUILD);
   if ( argc > 1 && !strcasecmp(argv[1], "--version") ) exit(0);
   sprintf(buf, "%s/.camp/camprc", getenv("HOME"));
   printf("Loading config from %s...\n", buf);
   config = getconfig(buf);

#ifdef USE_GPM_MOUSE   
   my_Gpm_Init(&mouse);
   atexit(close_gpm);
#endif
   
   if ( argc != 1 ) {   
      printf("Scanning arguments, please wait.. "); 
      for(i=1;i<argc;i++) {
	 printf("\e[s[%d/%d]\e[K\e[u", i-1, argc-1); fflush(stdout);
	 if ( !strcasecmp(argv[i], "-r") || !strcasecmp(argv[i], "--renameid3") )
	   dump = 2; else
	   if ( !strcasecmp(argv[i], "-d") || !strcasecmp(argv[i], "--dumpid3") )
	     dump = TRUE; else
	   if ( !strcasecmp(argv[i], "-f") || !strcasecmp(argv[i], "--force") )
	     force = TRUE; else
	   if ( !strcasecmp(argv[i], "-l") || !strcasecmp(argv[i], "--lock") )
	     alwayslocked = TRUE; else
	   if ( !strcasecmp(argv[i], "-p") || !strcasecmp(argv[i], "--playlist") ) {
	      i++;
	      loadplaylist(&playlist, argv[i], FALSE);
	   } else
	   if ( !strcasecmp(argv[i], "-s") || !strcasecmp(argv[i], "--steal") ) {
	      stealback();
	      checkkill = TRUE;
	   } else
	   if ( !strcasecmp(argv[i], "-k") || !strcasecmp(argv[i], "--kill") )
	     killcamp(); else
	   if ( !strcasecmp(argv[i], "--skin") ) {
	      i++;
	      unloadskin(&config.skin);
	      loadskin(argv[i], &config);	      
	     } else
	   if ( !strcasecmp(argv[i], "-b") || !strcasecmp(argv[i], "--background") ) {
	      quitmode = 1; 
	      quiet = TRUE;
	   } else
#ifdef RC_ENABLED
	   if ( !strcasecmp(argv[i], "--norc") )
	     config.userc = FALSE; else
#endif	   
	   if ( !strcasecmp(argv[i], "--help") || !strcasecmp(argv[i], "-h") ) {
	      printf("\nUsage: camp [OPTIONS] [FILES]\n\n");
	      printf("Options:\n");
	      printf("  -b,   --background   Forks camp into background directly\n");
	      printf("  -d,   --dumpid3      Dumps id3 tags to stderr\n");
	      printf("  -f,   --force        Force run, even if camp's running. (Won't write pidfile)\n");
	      printf("  -h,   --help         This help\n");
	      printf("  -k,   --kill         Kill running camp session\n");
	      printf("  -l,   --lock         Lock computer to camp console while running\n");
/*	      printf("        --skin X       Use skin 'X' (override config)\n"); */
/*	      printf("  -r,   --renameid3    Rename files to id3.artist - id3.songname.mp3\n"); */
	      printf("  -p X, --playlist X   Load playlist 'X'\n");
	      printf("  -s,   --steal        \"Steal\" another camp session\n");	      
#ifdef RC_ENABLED
	      printf("        --norc         Don't try to use the remote control\n");
#endif
	      printf("\nAny other parameter will be taken for a regular mp3 file.\n");
	      exit(0);
	   } else
	   addfiletolist(&playlist, argv[i], NULL, 0, 0, 0, config.useid3);
      }
      printf("\e[s[%d/%d]\e[K\e[u", i-1, argc-1); // fflush(stdout);
      printf("done!      \n");
   }
   
   buttonpos = config.skin.msb;
   
   if ( dump ) {
      printf("Dumping ID3 tags to stderr..\n");
      if ( !playlist ) {
	 fprintf(stderr, "No files to dump tags from!\n");
	 exit(0);
      }
      pl_seek(0, &playlist);
      while ( playlist->next ) {
	 fprintf(stderr, "%-5d %s\n", playlist->number, playlist->showname); fflush(stderr);
	 if ( dump == 2 ) {
	    strcpy(buf, playlist->showname);
	    strcat(buf, ".mp3");
	    rename(playlist->name, buf);
	 }
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
	    if ( playlist != NULL ) clearplaylist(&playlist);
	    exit(0);
	 }
      }
   }
   
   if ( !playlist )
     playsong = FALSE;
   
#ifdef RC_ENABLED
   if ( config.userc ) {
      printf("Initializing LPT remote control... ");
      if ( ioperm(config.rc.port+1,1,1) == -1 ) {
	 printf("error, needs root! disabled.\n");
	 config.userc = FALSE;
      } else {
	 printf("ok!\n");
	 checkrc();
      }
   }
#endif
   
#ifdef LIRCD
   printf("Initializing LIRCD remote control ...\n");
   if ( lirc_init("camp") == -1 ) {
      printf("LIRCD Failed!\n");
      use_lircd = FALSE;
   } else
     if ( lirc_readconfig(NULL, &lircd_config, NULL) != 0 ) {
	printf("Error reading LIRCD config file %s!\n", LIRCCFGFILE); 
	use_lircd = FALSE;
     } else 
     printf("LIRCD Initialized succesfully!\n");
#endif
   
   sleep(1);
   myinit();

   atexit(myexit);
   
   if ( playlist && playsong ) {
      playlistents = pl_count(playlist);
      if ( !slavepid ) {
	 if ( config.playmode == 2 ) filenumber = myrand(playlistents);
	 call_player(pl_seek(filenumber, &playlist));
      } else {
	 signal(SIGCHLD, playnext);
      }       
   }

   if ( quitmode == 1 ) disappear();

   updatedata();

   if ( alwayslocked ) {
      quiet = TRUE;
      printf("\e[?25h"); /* cursor on */
      buf[0] == 1; passwd[0] == 0;
      while ( strcmp(passwd, buf) ) {
	 printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[0]);
	 readpass(passwd, 15);
	 printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[1]);
	 readpass(buf, 15);
      }
      
      printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
      
      printf("\e[?25l"); /* cursor on */
      i = open("/dev/console",O_WRONLY);	       
      if ( i == -1 ) alwayslocked = FALSE; else ioctl(i, VT_LOCKSWITCH);
      close(i);
      quiet = FALSE;
   }
   
   selval = stdinno + 1;
   
#ifdef USE_GPM_MOUSE
   if ( gpm_flag > 0 ) selval = gpm_fd + 1;
#endif
   
#ifdef LIRCD
   if ( use_lircd ) selval = lirc_lircd + 1;
#endif
   
   while ( TRUE ) { /* main loop */
         
      updatesongtime('u');
      
      if ( checkkill && kill(slavepid, 0) == -1 ) {
	 checkkill = FALSE;
	 playnext(-1);
      }

#ifdef RC_ENABLED
      if ( config.userc ) checkrc();
#endif

//      get_volume(config.voldev, &left, &right);
//      printf("\e[1;1H%d - %d   \n", left, right);
      
      currenttime.tv_usec = config.rctime;
      currenttime.tv_sec  = 0;
      FD_ZERO(&fds);
      FD_SET(stdinno, &fds);
            
#ifdef USE_GPM_MOUSE
      if ( gpm_flag > 0 ) FD_SET(gpm_fd, &fds);
#endif
#ifdef LIRCD
      if ( use_lircd ) FD_SET(lirc_lircd, &fds);
#endif
	
      if ( select(selval, &fds, NULL, NULL, &currenttime) > 0 ) {
#ifdef USE_GPM_MOUSE
	 if ( gpm_flag > 0 && FD_ISSET(gpm_fd, &fds) ) main_domouse();
#endif
#ifdef LIRCD	 	 
	 if ( use_lircd && FD_ISSET(lirc_lircd, &fds) ) dolircd(1);
#endif
	 
	 ch=0;
	 if ( FD_ISSET(stdinno, &fds) ) {
	    ch = getchar();
	    if   ( ch == 27 )   escfix(); else
	      if ( ch == 3  && canexit() )  exit(0);  else    /* ^C */
	      if ( ch == 26 && canexit() ) disappear(); else /* ^Z */
#ifdef HAVE_SYS_SOUNDCARD_H
	      if ( ch == '+' ) set_volume(config.voldev, config.volstep); else
	      if ( ch == '-' ) set_volume(config.voldev, -config.volstep); else
#endif
	      if ( ch == 12 ) {                 /* ^L */
		 printf("\e[0m\e[2J\e[1;1H%s", config.skin.main);
		 updatedata();
		 updatesongtime('f');
		 updatebuttons(0);
	      } else
	      if (ch == 13 && dofunction(-1)) call_player(pl_seek(filenumber, &playlist)); else
	      for(i=0;i<15;i++)
		if ( ch == config.skin.mh[i] ) {
		     if ( config.skin.ma[i] != NULL && config.skin.mi[i] != NULL ) {
			buttonpos = i;
			updatebuttons(0);
		     }
		     if ( dofunction(i) ) call_player(pl_seek(filenumber, &playlist));
		  }
	 } /* FD_ISSET */
      } /* select */
   } /* while(endless) */   
   return 0; /* Uhm, just wasting bytes, this is not possible */
} /* main() */

   
   //     Procedures & functions             //


void updatebuttons(char motion) {
int i;
   
   if ( motion != KM_NONE )
     if ( motion == KM_UP    && config.skin.mju[buttonpos] != -1 ) buttonpos = config.skin.mju[buttonpos]; else
     if ( motion == KM_DOWN  && config.skin.mjd[buttonpos] != -1 ) buttonpos = config.skin.mjd[buttonpos]; else
     if ( motion == KM_LEFT  && config.skin.mjl[buttonpos] != -1 ) buttonpos = config.skin.mjl[buttonpos]; else
     if ( motion == KM_RIGHT && config.skin.mjr[buttonpos] != -1 ) buttonpos = config.skin.mjr[buttonpos];
 
   for (i=0;i<14;i++) {
      if ( !config.skin.my[i] && !config.skin.mx[i] ) continue;
      printf("\e[%d;%dH",config.skin.my[i], config.skin.mx[i]);
      if (buttonpos == i) printf("%s", config.skin.ma[i]); else
	printf("%s", config.skin.mi[i]);
   }
   
   // fflush(stdout);
}

void myinit(void) {
FILE *fd;
long flags;
   
   if ( !force ) {
      fd = fopen(PID_FILE, "w");
      fprintf(fd, "%d\n", getpid());
      fclose(fd);
   }
   signal(SIGUSR1, sigusr1);
#ifdef HAVE_TERMIOS_H
   setvbuf(stdout, (char *)NULL, _IONBF, 0);
   tcgetattr(fileno(stdin), &oldtermios);
   memcpy(&newtermios, &oldtermios, sizeof(struct termios));
   termios_raw(&newtermios);
   tcsetattr(fileno(stdin), TCSANOW, &newtermios);
#else
   initscr();
   intrflush(stdscr, FALSE);
   savetty();
   noecho(); 
   nl(); 
   cbreak();
#endif
   if ( !quiet ) {
      printf("\e[0m\e[2J\e[?25l\e[1;1H\e(U%s", config.skin.main);
      updatebuttons(0);
   }
}

#ifdef USE_GPM_MOUSE   
void close_gpm(void) {
   while ( Gpm_Close() );   
}
#endif
   
void myexit(void) {
int i;
   
/*#ifdef LIRCD
   if ( lircd_config ) lirc_freeconfig(lircd_config);
   if ( use_lircd ) lirc_deinit();
#endif
*/   
   if ( quitmode == 0 )  /* 0 = normal, 1 = forking, 2 = "forked-steal" quit  3 = "steal" quit*/
      if ( playsong ) killslave();
   
   if ( alwayslocked && quitmode != 2 ) {
      i = open("/dev/console",O_WRONLY);	       
      if ( i == -1 ) {
	 printf("FAILED TO UNLOCK CONSOLE!!\n");
      } else {
	 ioctl(i, VT_UNLOCKSWITCH);
      }
      close(i);
   }
   

   /* clean up some shit */
   unloadskin(&config.skin);
   for(i=0;i<15;i++) if (config.playerargv[i]) free(config.playerargv[i]);
   if ( playlist != NULL ) clearplaylist(&playlist);

#ifdef HAVE_TERMIOS_H
   tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
#else
   resetty();
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

void unloadskin(struct skinconfig *skin) {
int i;
   for(i=0;i<6;i++)   if (skin->flistmsg[i]) free(skin->flistmsg[i]);
   for(i=0;i<4;i++)   if (skin->mainmsg[i]) free(skin->mainmsg[i]);
   for(i=0;i<7;i++)   if (skin->fi[i]) free(skin->fi[i]);
   for(i=0;i<7;i++)   if (skin->fa[i]) free(skin->fa[i]);
   for(i=0;i<7;i++)   if (skin->pi[i]) free(skin->pi[i]);
   for(i=0;i<7;i++)   if (skin->pa[i]) free(skin->pa[i]);
   for(i=0;i<14;i++)  if (skin->mi[i]) free(skin->mi[i]);
   for(i=0;i<14;i++)  if (skin->ma[i]) free(skin->ma[i]);
   for(i=0;i<3;i++)   if (skin->modetext[i]) free(skin->modetext[i]);
   for(i=0;i<2;i++)   if (skin->stereotext[i]) free(skin->stereotext[i]);
   
   if (skin->mtextc)      free(skin->mtextc);
   if (skin->textc)       free(skin->textc);
   if (skin->songnamec)   free(skin->songnamec);
   if (skin->sampleratec) free(skin->sampleratec);
   if (skin->bitratec)    free(skin->bitratec);
   if (skin->timec)       free(skin->timec);
   if (skin->stereoc)     free(skin->stereoc);
   if (skin->modetextc)   free(skin->modetextc);
   if (skin->songnumberc) free(skin->songnumberc);
   if (skin->flistcdi)    free(skin->flistcdi);
   if (skin->flistcda)    free(skin->flistcda);
   if (skin->flistcfi)    free(skin->flistcfi);
   if (skin->flistcfa)    free(skin->flistcfa);
   if (skin->flistcta)    free(skin->flistcta);
   if (skin->flistcti)    free(skin->flistcti);
   if (skin->plistci)     free(skin->plistci);
   if (skin->plistca)     free(skin->plistca);
   if (skin->id3st)       free(skin->id3st);
   if (skin->id3fnc)      free(skin->id3fnc);
   if (skin->id3tc)       free(skin->id3tc);
   if (skin->id3sc)       free(skin->id3sc);

   free(skin->main);
   free(skin->playlist);
   free(skin->filelist);
   free(skin->id3);      
}

void escfix(void) {
int ch1, ch2, flags;
         
#ifdef HAVE_TERMIOS_H
   fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
#endif
   
   ch1 = getchar();
   if ( ch1 == -1 ) {
      mykbhit(0, 250000);
      ch1 = getchar();
   }
   ch2 = getchar();   

#ifdef HAVE_TERMIOS_H
   fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#endif
   
   if ( ( ch1 == -1 || ch1 == 27 ) && canexit() ) exit(0); else
     
     if ( ch1 == '[' )
       if ( ch2 == 'C' ) updatebuttons(KM_RIGHT);  else
     if ( ch2 == 'D' ) updatebuttons(KM_LEFT); else
     if ( ch2 == 'A' ) updatebuttons(KM_UP); else
     if ( ch2 == 'B' ) updatebuttons(KM_DOWN); 
}


int dofunction(char forcedbutton) {
static struct timeval pause_start, pause_end;
char pass[16], checkpass[16];
int mod, ec, oldfilenumber;
char *buf, *buf2;
FILE *fd;
   
   if ( forcedbutton == -1 ) forcedbutton = buttonpos;
   
   switch( forcedbutton ) {

    case 0: /* skip back */
      if ( !playlist || pausesong ) return(0);
      if ( filenumber == 0 ) filenumber = playlistents-1; else
	filenumber--;
      if ( playsong == TRUE && !slavepid ) killslave(); else
      return(1);
      break;
      
    case 1: /* play */
      if ( !playlist ) return(0);
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
      
    case 2: /* skip fwd */
      if ( !playlist || pausesong ) return(0);
      if ( filenumber >= playlistents-1 ) filenumber = 0; else
	filenumber++;
      if ( playsong == TRUE && !slavepid) killslave(); else
      return(1);
      break;
      
    case 3: /* stop */
      if ( playsong && slavepid ) {
	 playsong = FALSE;
	 killslave();	 
      }
      updatedata();
      break;
      
    case 4: /*playmode */
      if ( config.playmode != 2 )
	config.playmode++; else config.playmode = 0;
      updatedata();
      break;

    case 5: /* playlist */
      quiet = TRUE;
      rplaylist(&playlist, &filenumber);
#ifdef LIRCD
      clear_lirc();
#endif
      playlistents = pl_count(playlist);
      printf("\e[0m\e[2J\e[1;1H%s", config.skin.main);
      // fflush(stdout);
      quiet = FALSE;
      scrollsongname(0);
      updatedata();
      updatebuttons(0);
      break;
      
    case 6: /* custom button */
      printf("\e[0m\e[2J\e[1;1H"); 
      // fflush(stdout);
#ifdef HAVE_TERMIOS_H
      tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
#else
      resetty();
#endif

#ifdef USE_GPM_MOUSE   
   while( Gpm_Close() );
#endif
      quiet = TRUE;
      my_system(CUSTOM_RUN);
      quiet = FALSE;
      
#ifdef USE_GPM_MOUSE   
   my_Gpm_Init(&mouse);
#endif
      
#ifdef HAVE_TERMIOS_H
      tcsetattr(fileno(stdin), TCSANOW, &newtermios);
      fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#else
      savetty();
      noecho(); 
      nonl(); 
      cbreak();
#endif
      scrollsongname(0);
      if ( !quiet ) {
	 printf("\e[?25l\e[0m\e[2J\e[1;1H%s", config.skin.main);	 
	 updatebuttons(0);
      }
      updatedata();
      // fflush(stdout);
      break;
   
    case 8: /* pause */
      if ( playsong ) {
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
      return 0;

    case 10: /* jump */
      oldfilenumber = filenumber;
      pass[0] = 0;
      buf  = (char*)malloc(256);
      buf2 = (char*)malloc(256);
      printf("\e[?25h"); /* cursor on */
      sprintf(buf, "\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[3]);
      sprintf(buf2, "%s%d%d", config.skin.mainmsg[3], 0, playlistents);
      do {
	 if ( !playlistents )
	   printf(buf, 0, playlistents); else
	   printf(buf, 1, playlistents);
	 mod = 27;
	 filenumber = atoi(readyxline(config.skin.mtexty, config.skin.mtextx+(ansi_strlen(buf2)-4), pass, 4, &ec, &mod));
	 sprintf(pass, "%d", filenumber); 
	 if ( filenumber == 0 || ec == 27 || !mod ) { filenumber = -1; break; }
	 filenumber--;
      } while( filenumber < 0 || filenumber > playlistents-1 );
      free(buf);
      free(buf2);
      printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
      printf("\e[?25l"); /* cursor off */
      if ( filenumber != -1 ) call_player(pl_seek(filenumber, &playlist)); else {
	 filenumber = oldfilenumber;
      }
      updatedata();
      updatebuttons(0);
      break;
      
    case 11: /* lock */
      printf("\e[?25h"); /* cursor on */
      printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[0]);
      readpass(pass, 15);
      printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[1]);
      readpass(checkpass, 15);
      // fflush(stdout);
      if ( !strcmp(pass, checkpass) ) {	
	 printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
	 updatedata();
	 if ( config.lockvt && ! alwayslocked ) {
	    ec = open("/dev/console",O_WRONLY);	       
	    ioctl(ec, VT_LOCKSWITCH);
	    close(ec);
	 }
	 do {
	    printf("\e[?25l"); /* cursor off */
	    while ( !mykbhit(1, 0) )
	      updatesongtime('u');
	    checkpass[0] = getchar();
	    printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[2]);
	    printf("\e[?25h"); /* cursor on */
	    // fflush(stdout);
	    memset(checkpass, 0, 15);
	    readpass(checkpass, 15);
	    printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
	    updatedata();
	 } while ( strcmp(pass, checkpass) );
      }
      if ( config.lockvt && !alwayslocked ) {
	 ec = open("/dev/console",O_WRONLY);	       
	 ioctl(ec, VT_UNLOCKSWITCH);
	 close(ec);
      }
      printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
      updatebuttons(0);
      updatedata();
      printf("\e[?25l"); /* cursor off */
      break;
      
    case 12: /* desc edit */
      if ( !playlist ) return;
      quiet = TRUE;
      id3edit(pl_seek(filenumber, &playlist)->name, pl_seek(filenumber, &playlist));
#ifdef LIRCD
      clear_lirc();
#endif
      quiet = FALSE;
      printf("\e[0m\e[2J\e[1;1H%s", config.skin.main);
      scrollsongname(0);
      updatedata();
      updatebuttons(0);
      break;

    case 13: /* fork() */
      if ( canexit() ) disappear();
      break;

    case 14: /* mute, new in 101 */
      if ( !muted ) muted=TRUE; else
	muted=FALSE;
      mute(muted);
      break;
      
   }   
   return(0);
}

#ifdef USE_GPM_MOUSE
void main_domouse(void) {
Gpm_Event event;
char i,y = FALSE;
   
   Gpm_GetEvent(&event);
   
   /* middle button */
   if ( event.buttons & 2 && event.type & GPM_DOWN ) {
      printf("\e[0m\e[2J\e[1;1H%s", config.skin.main);
      updatedata(); 
      updatesongtime('f');
      updatebuttons(0);
      event.buttons = 255;
   } else 

   /* left button */
   if ( event.buttons & 4 && event.type & GPM_DOWN ) {
      if ( event.y == config.skin.timey && event.x >= config.skin.timex && event.x < (config.skin.timex+5) ) {
	 if ( config.timemode ) 
	   config.timemode = FALSE; else
	   config.timemode = TRUE;
	 updatesongtime('f');
      } else 
	if ( event.y == config.skin.modetexty && event.x >= config.skin.modetextx && event.x < (config.skin.modetextx+config.skin.modetextw) ) {
	   if ( config.playmode != 2 )
	     config.playmode++; else config.playmode = 0;
	   updatedata();
	} else
	for(i=0;i<MAXBUTTON;i++)
	  if ( event.y == config.skin.my[i] && event.x >= (config.skin.mx[i]-config.skin.mouseexpand) && event.x < (config.skin.mx[i]+config.skin.mw[i]+config.skin.mouseexpand) ) { buttonpos = i; y=TRUE; break; }
   }   
   if ( y ) {
      updatebuttons(0);
      if ( dofunction(-1) )
	call_player(pl_seek(filenumber, &playlist));
   }
   
}
#endif   

#ifdef LIRCD
void dolircd(char atmain) {
char *ir, *c;
   
   ir = lirc_nextir();
   
   while ( ir && (c=lirc_ir2char(lircd_config,ir)) != NULL ) {
      if ( !strcasecmp(c, "skip-") )    if ( dofunction(0) ) call_player(pl_seek(filenumber, &playlist)); 
      if ( !strcasecmp(c, "play") )     if ( dofunction(1) ) call_player(pl_seek(filenumber, &playlist));
      if ( !strcasecmp(c, "skip+") )    if ( dofunction(2) ) call_player(pl_seek(filenumber, &playlist));
      if ( !strcasecmp(c, "stop") )     (void*)dofunction(3); 
      
      if ( atmain ) {
	 if ( !strcasecmp(c, "playmode") ) (void*)dofunction(4); 
	 if ( !strcasecmp(c, "playlist") ) { 
	    (void*)dofunction(5);
	    return; 
	 }
	 if ( !strcasecmp(c, "custom") ) {
	    (void*)dofunction(6);
	    clear_lirc();
	    return;
	 }
	 if ( !strcasecmp(c, "pause") )    (void*)dofunction(8);
	 if ( !strcasecmp(c, "jump") ) {
	    (void*)dofunction(10);
	    return;
	 }
	 if ( !strcasecmp(c, "fork") )     (void*)dofunction(13);
      }
      
      if ( !strcasecmp(c, "mute") )     (void*)dofunction(14);
# ifdef HAVE_SYS_SOUNDCARD_H
      if ( !strcasecmp(c, "vol+") ) set_volume(config.voldev, config.volstep);
      if ( !strcasecmp(c, "vol-") ) set_volume(config.voldev, -config.volstep);
# endif
   }  
   if ( ir ) free(ir);
}

void clear_lirc(void) {
fd_set fds;
struct timeval currenttime;
char *ir;
   
   if ( !use_lircd ) return;
      
   currenttime.tv_usec = currenttime.tv_sec  = 0;
   FD_ZERO(&fds);
   FD_SET(lirc_lircd, &fds);
   
   while ( select(lirc_lircd+1, &fds, NULL, NULL, &currenttime) > 0 ) {
      ir = lirc_nextir();
      if ( ir ) free(ir);
   }
     
      
}
#endif

void updatedata() {
struct timeval currenttime;
   
   if ( quiet ) return;
   
   if ( currentfile.showname[0] != 0 ) {
      printf("\e[%sm\e[%d;%dH%5d", config.skin.sampleratec, config.skin.sampleratey, config.skin.sampleratex, currentfile.samplerate);
      printf("\e[%sm\e[%d;%dH%3d", config.skin.bitratec, config.skin.bitratey, config.skin.bitratex, currentfile.bitrate);
      printf("\e[%sm\e[%d;%dH%s", config.skin.stereoc, config.skin.stereoy, config.skin.stereox, config.skin.stereotext[currentfile.mode]);
      printf("\e[%sm\e[%d;%dH%4d\e[%d;%dH%-4d", config.skin.songnumberc, config.skin.songnumbery, config.skin.songnumberx, filenumber+1, config.skin.songnumbery, config.skin.songnumberx+5, playlistents);
   }
   updatesongtime('u');
   printf("\e[%sm\e[%d;%dH%s", config.skin.modetextc, config.skin.modetexty, config.skin.modetextx, config.skin.modetext[config.playmode]);
   // fflush(stdout);
   
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
      oldcurrenttime.tv_sec = currenttime.tv_sec;
      
      if ( !playsong ) {
	 if ( !config.timemode )
	   printf("\e[%sm\e[%d;%dH00\e[%d;%d00", config.skin.timec, config.skin.timey, config.skin.timex, config.skin.timey, config.skin.timex+3); else
	   printf("\e[%sm\e[%d;%dH%02d\e[%d;%dH%02d", config.skin.timec, config.skin.timey, config.skin.timex, currentfile.length/60, config.skin.timey, config.skin.timex+3, currentfile.length%60);
	 if ( ( config.showtime == 3 || config.showtime == 1 ) && strncasecmp(playlist->showname, "http://", 7) )
	   printf("\e[%sm\e[%d;%dH%s\e[%d;%dH[%02u:%02u] %s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, currentfile.length / 60, currentfile.length % 60, scrollsongname(1)); else
	   printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, scrollsongname(1));
	 // fflush(stdout);
	 return;
      }
      
      if ( ( config.showtime == 3 || config.showtime == 1 ) && strncasecmp(playlist->showname, "http://", 7) ) printf("\e[%sm\e[%d;%dH%s\e[%d;%dH[%02u:%02u] %s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, currentfile.length / 60, currentfile.length % 60, scrollsongname(1)); else
	printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, scrollsongname(1));
      /* guess this is the best way of doing this, since we don't want the player to type this,. */
      if ( pausesong ) return;
      if ( config.timemode ) {
	 reverse_time = currentfile.length-(currenttime.tv_sec-starttime.tv_sec);
	 if ( reverse_time < 0 ) { 
	    reverse_time = 0;
	    printf("\e[%sm\e[%d;%dH--\e[%d;%dH--", config.skin.timec, config.skin.timey, config.skin.timex, config.skin.timey, config.skin.timex+3);
	 } else
	    printf("\e[%sm\e[%d;%dH%02d\e[%d;%dH%02d", config.skin.timec, config.skin.timey, config.skin.timex, reverse_time/60, config.skin.timey, config.skin.timex+3, reverse_time%60);
	   
      } else
	printf("\e[%sm\e[%d;%dH%02d\e[%d;%dH%02d", config.skin.timec, config.skin.timey, config.skin.timex, (currenttime.tv_sec-starttime.tv_sec)/60, config.skin.timey, config.skin.timex+3, (currenttime.tv_sec-starttime.tv_sec)%60);
      // fflush(stdout);
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


char *scrollsongname(char action) {   
static char scrolledname[100];
static int i, wichway;
      
   if ( action == 0 ) {
      i = -1;
      wichway = 1; /* -> */
      return NULL;
   }
      
   if ( action == 2 ) { /* scroll back to beginning, then return NULL (unused) */
      if ( i == 0 ) return NULL;
      i--;
      strncpy(scrolledname, currentfile.showname+i, config.skin.songnamew);
      return scrolledname;
   }
   
   if ( config.scrollsn && strlen(currentfile.showname) > (config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8) && action == 1 ) {
      if ( i+ (config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8) == strlen(currentfile.showname) && wichway == 1 )
	wichway = -1; else /* <- */
	if ( i == 0 && wichway == -1 ) wichway = 1; /* -> */
      i += wichway;
   } else
     i = 0;
   
   strncpy(scrolledname, currentfile.showname+i, config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8 );

   return scrolledname;
}
   
int canexit(void) {
char checkpasswd[16];
   
   if ( !alwayslocked ) return TRUE;
   quiet = TRUE;
   printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[2]);
   printf("\e[?25h"); /* cursor on */
   // fflush(stdout);
   memset(checkpasswd, 0, 16);
   readpass(checkpasswd, 15);
   printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
   printf("\e[?25l"); /* cursor off */
   quiet = FALSE;
   if ( !strcmp(passwd, checkpasswd) ) return TRUE; else
     return FALSE;
   
}
