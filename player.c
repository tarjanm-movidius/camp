#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#ifndef USE_TERMIOS
# include <curses.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "camp.h"

extern struct playlistent *playlist;
extern unsigned int slavepid, filenumber, playlistents, fs, bitrate;
extern char showname[100];
extern char quiet, pausesong, playsong;
extern unsigned char mode;
extern struct configstruct config;

void playnext(int sig) {
   if ( playlist == NULL ) return;
   signal(SIGCHLD, playnext);
   if ( pausesong ) return;
   if ( playsong && slavepid && ( ( waitpid(-1, NULL, WNOHANG) > 0 ) || ( kill(slavepid, 0) != 0 ) ) ) {
      if ( config.playmode != 2 ) {
	 if ( filenumber+1 == playlistents && config.playmode == 1 ) filenumber = playlistents+2; else
	   if ( filenumber+1 == playlistents && config.playmode == 0 ) exit(0); 
	 filenumber++;
      } else
	filenumber = myrand(playlistents); 
      if ( filenumber+1 > playlistents ) filenumber = 0;
      slavepid = 0;
      call_player(pl_seek(filenumber, playlist));
   } // if ( !getampinfo()  && playsong == TRUE )
}


void killslave() {
int pid;
   if ( slavepid != 0 ) {
      pid = slavepid;
      slavepid = 0;
      if ( pausesong ) kill(pid, SIGCONT);
      pausesong = FALSE;
      kill(pid, SIGKILL);
      waitpid(-1, NULL, 0);
   }
}

void slave(char *filename) {
char i=0, buf[256];
FILE *fd;

   if ( !config.dontreopen ) {
      freopen("/dev/null", "w", stdout);
      freopen("/dev/null", "w", stderr);
   }
   for(i=0;i<15;i++)
     if (config.playerargv[i] == NULL) break;
   config.playerargv[i] = (char*)malloc(strlen(filename)+1);
   strcpy(config.playerargv[i], filename);
   sprintf(buf, "%s/%s", config.playerpath, config.playername);
   execve(buf, config.playerargv, NULL);
}


void call_player(struct playlistent *pl) {
int  j, cspos=0;
FILE *fd;
      
   if ( pl == NULL || pausesong ) return;
   
   strcpy(showname, pl->showname);
   bitrate = pl->bitrate;
   fs      = pl->samplerate;
   mode    = pl->mode;
   if ( strlen(showname) >= 51 ) showname[50] = '\0'; 
   
   updatesongtime('i');
   if ( !quiet ) updatedata();
   
   if ( playsong ) {
      if ( slavepid != 0 ) killslave();
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
	 break;
      } /* switch( fork() ); */
      //signal(SIGCHLD, playnext);
   }
/*   if (!quiet) printf("\e[0;34;46m\e[9;36H%-.5d\e[9;45H%-.3d\e[9;62H%s", fs, bitrate, mode); */
}
