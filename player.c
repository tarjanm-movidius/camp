#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#ifndef HAVE_TERMIOS_H
# include <curses.h>
#endif
#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "camp.h"

extern struct playlistent *playlist;
extern unsigned int slavepid, filenumber, playlistents;
extern struct oneplaylistent currentfile;
extern char quiet, pausesong, playsong, checkkill, currloc;
extern struct configstruct config;
extern int pl_current, pl_screenmark;

void playnext(int sig) {
int status = 0;

   if ( !playlist ) return;
   if ( !config.mpg123 || ( config.mpg123 && sig != -1 ) )
     if ( ( sig == -1 ) || ( playsong && !pausesong && slavepid && ( waitpid(slavepid, NULL, WNOHANG) == slavepid ) ) ) {
	if ( sig != -1 ) waitpid(slavepid, NULL, 0);
	if ( config.mpg123 && sig != -1 ) mpg123_control("#RESTART"); 

	if ( config.playmode != 2 ) {
	   if ( filenumber+1 == playlistents && config.playmode == 1 ) filenumber = playlistents+2; else
	     if ( filenumber+1 == playlistents && config.playmode == 0 ) exit(0);
	   filenumber++;
	} else
	  filenumber = myrand(playlistents); 
	if ( filenumber+1 > playlistents ) filenumber = 0;
	slavepid = 0;
	call_player(pl_seek(filenumber, &playlist));
     } else
     signal(SIGCHLD, playnext);
}


void killslave() {
   if ( !slavepid ) return;
   signal(SIGCHLD, SIG_IGN);
   if ( pausesong ) kill(slavepid, SIGCONT);
   pausesong = FALSE;
   if ( config.nicekill ) kill(slavepid, SIGTERM); else
   kill(slavepid, SIGKILL);
   if ( config.kill2pids ) kill(slavepid+1, SIGKILL); /* This is no good, but it (usually) works for multithreaded-buffering players ;) */
   if ( checkkill )
     while ( kill(slavepid, 0) != -1 ) usleep(1000); else
     waitpid(slavepid, NULL, 0);
   slavepid = 0;
   checkkill = FALSE;
}

void slave(char *filename) {
char i=0, buf[256];
FILE *fd;

#ifdef USE_GPM_MOUSE
   while ( Gpm_Close() != 0 ) ;
#endif
   if ( !config.dontreopen ) {
      freopen("/dev/null", "w", stdout);
      freopen("/dev/null", "w", stderr);
   }
   for(i=0;i<15;i++)
     if (config.playerargv[i] == NULL) break;
   config.playerargv[i] = (char*)malloc(strlen(filename)+1);
   strcpy(config.playerargv[i], filename);
   sprintf(buf, "%s/%s", config.playerpath, config.playername);
   if ( execve(buf, config.playerargv, NULL) == -1 ) {
      printf("Execution of player (%s) failed!\n", buf);
      perror("execve()");
      sleep(2);
      exit(-1);	    
   }
}


void call_player(struct playlistent *pl) {
int  j, cspos=0;
char name[31], artist[31], buf[500];
struct stat statf;
FILE *fd;
      
   if ( pl == NULL || pausesong ) return;

   memcpy((void*)&currentfile, (void*)pl, sizeof(struct oneplaylistent));
   
   if ( config.mpg123 ) {
      sprintf(buf, "load %s\n", currentfile.name);
      mpg123_control(buf);
      return;
   }
   
   if ( config.bufferdelay ) usleep(config.bufferdelay);
   
   if ( config.rescanid3 || ( !config.useid3 && !pl->length ) ) {
      if ( getmp3info(pl->name, &pl->mode, &pl->samplerate, &pl->bitrate, name, artist, NULL, NULL, NULL, 0 ) ) {
	 if ( artist[0] ) sprintf(pl->showname, "%s - %s", artist, name ); else
	   strcpy(pl->showname, name);
      }
      if ( pl->bitrate ) {
	 stat(pl->name, (struct stat*)&statf);
	 pl->length     = statf.st_size / (pl->bitrate * 125);
      }	 	 
   }

   memcpy((void*)&currentfile, (void*)pl, sizeof(struct oneplaylistent));
   
   scrollsongname(0);
   scrollsongname(1); /* Hrm */
   updatesongtime('i');
   updatesongtime('f');
   updatedata();
   if ( !quiet && currloc == CAMP_MAIN && config.skin.platmain ) {
      pl_current = filenumber;
      if ( pl_current-pl_screenmark < 0 )
	pl_screenmark = pl_current;
      pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
   }      
   
   if ( playsong ) {
      if ( slavepid != 0 ) killslave();
      if ( !exist(pl->name) && strncasecmp(pl->name, "http://", 7) ) {
	 playnext(-1);
	 return;
      }
      slavepid = fork();
      switch( slavepid ) {
       case -1: 
	 perror("fork");
	 exit(-1);
       case 0:
	 slave(pl->name);
	 break;
       default:
	 signal(SIGCHLD, playnext);
	 setpriority(PRIO_PROCESS, slavepid, config.playerprio);
	 break;
      } /* switch( fork() ); */
      //signal(SIGCHLD, playnext);
   }
}

char *mpg123_control(char *command) {
static int mpgrfd=-1, mpgwfd=-1;
int insocks[2], outsocks[2]; // 0 = read, 1 = write
fd_set rfds;
struct timeval tv;
char buf[500], ch[2];
int i;
   
   ch[1] = 0;

   if ( command && !strcmp(command+1, "RESTART") ) {
      /* Oh my god!! They've killed kenny! */
      close(mpgwfd);
      close(mpgrfd);
      mpgrfd = mpgwfd = -1; /* Yep, let's go back to scratch */
   }
   
   if ( mpgrfd == -1 && mpgwfd == -1 ) { /* start mpg123, setup control pipe */

      if ( pipe(insocks) == -1 ) {
	 printf("Cannot create pipes! Try set \"mpg123mode = false\" in ~/.camp/camprc!\n");
	 sleep(2);
	 exit(-1);
      }
      
      if ( pipe(outsocks) == -1 ) {
	 printf("Cannot create pipes! Try set \"mpg123mode = false\" in ~/.camp/camprc!\n");
	 sleep(2);
	 exit(-1);
      }
      
      
      slavepid = fork();
      switch ( slavepid ) {
       case -1:
	 printf("Cannot fork! Try restarting camp!\n");
	 sleep(2);
	 exit(-1);
       case 0:
	 dup2(outsocks[0], STDIN_FILENO);
	 dup2(insocks[1], STDOUT_FILENO);
	 dup2(insocks[1], STDERR_FILENO);
	 close(outsocks[0]);
	 close(outsocks[1]);
	 close(insocks[0]);
	 close(insocks[1]);

	 for(i=0;i<15;i++)
	   if (config.playerargv[i] == NULL) break;
	 config.playerargv[i] = (char*)malloc(3);
	 strcpy(config.playerargv[i], "-R"); i++;
	 config.playerargv[i] = (char*)malloc(1);
	 strcpy(config.playerargv[i], "");
	 sprintf(buf, "%s/%s", config.playerpath, config.playername);
	 printf("%s - ", buf);
	 for(i=0;i<15;i++)
	   printf("%s ", config.playerargv[i]);
	 printf("\n");
	 sleep(5);
	 if ( execve(buf, config.playerargv, NULL) == -1 ) {
	    printf("Execution of player (%s) failed!\n", buf);
	    perror("execve()");
	    sleep(2);
	    exit(-1);	    
	 }
      	 break;

       default: /* rock 'n' roll, we're up kicking */
	 mpgrfd = insocks[0];
	 mpgwfd = outsocks[1];
	 break;
	 
      } /* switch fork .. */
      
   } /* Init session end. */
   

   if ( command && command[0] != '#' ) { /* write .. */

      //      write(outsocks[1], command, strlen(command)+1);
      if ( write(mpgwfd, command, strlen(command)) != strlen(command) ) {
	 /* Write error! This is no good.. */
	 close(mpgwfd);
	 close(mpgrfd);
	 killslave();
	 mpgrfd = mpgwfd = -1;
	 return NULL;
      }
      
   } else { /* read .. */      
      
      i = 1;
      while ( i ) { /* As log as mpg123 has sonething to report.. */
      
	 FD_ZERO(&rfds);
	 FD_SET(mpgrfd, &rfds);
	 tv.tv_sec = tv.tv_usec = 0;
	 
	 if ( select(mpgrfd+1, &rfds, NULL, NULL, &tv) && FD_ISSET(mpgrfd, &rfds) ) {
	    /* Data availible in pipe */
	    
	    memset(buf, 0, 500);
	    while ( (read(mpgrfd, ch, 1) != 0) && ch[0] != '\n'  )
	      if ( ch[0] != '\r' && ch[0] != '\n' ) strncat(buf, ch, 1); 
	    /* There must be a better way to read single lines from files/piles eh?
	     * something like fgets .. please inform me if there is such a command :) */
	    
//	    printf("\e[1;1HEWP! GOT LINE: %s--\n", buf);

	    if ( command )
	      if ( !strncmp(buf, command+1, strlen(command)-1) ) i = 0;
	    
	    if ( !strcmp(buf, "@P 0") && playsong )
	      playnext(-1);

	 } else if ( !command || command[0] != '#' ) return NULL; /* pipe data end */
      } /* while 1 .. */
   } /* end read */
   return NULL;
}
   
