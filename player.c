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
extern char quiet, pausesong, playsong, checkkill;
extern struct configstruct config;

void playnext(int sig) {
int status = 0;

   if ( !playlist ) return;
   if ( ( sig == -1 ) || ( playsong && !pausesong && slavepid && ( waitpid(slavepid, NULL, WNOHANG) == slavepid ) ) ) {
      if ( sig != -1 ) waitpid(slavepid, NULL, 0);
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
   execve(buf, config.playerargv, NULL);

}

void mod_slave(char *filename) {
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
     if (config.modplayerargv[i] == NULL) break;
   config.modplayerargv[i] = (char*)malloc(strlen(filename)+1);
   strcpy(config.modplayerargv[i], filename);
   sprintf(buf, "%s/%s", config.modplayerpath, config.modplayername);
   execve(buf, config.modplayerargv, NULL);
   
}


void call_player(struct playlistent *pl) {
int  j, cspos=0;
char name[31], artist[31];
struct stat statf;
FILE *fd;
      
   if ( pl == NULL || pausesong ) return;
      
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

   scrollsongname(0);
   memcpy((void*)&currentfile, (void*)pl, sizeof(struct oneplaylistent));
   // if ( strlen(currentfile.showname) > config.skin.songnamew ) currentfile.showname[config.skin.songnamew] = '\0';
   
   updatesongtime('i');
   updatesongtime('f');
   if ( !quiet ) updatedata();
   
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
	 if( modcheck(pl->name) == 0 )
	   mod_slave(pl->name); else
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

int modcheck( char *name )
{
   int i,c;
   char buf[3];
   
   for( c = 0; c <= strlen(name); c++ )
     {
	if( name[c] == '.' )
	  {
	     for( i = 0; i < 4; i++ )
	       {
		  buf[i] = name[i+1+c];
	       }
	  }
     }

   if( strcmp( buf, "mod") == 0 )
     return 0;
   else if( strcmp( buf, "xm") == 0 )
     return 0;
   else
     return -1;
   
}
