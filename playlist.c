#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include "camp.h"

#ifndef USE_TERMIOS
# include <curses.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

extern char playsong;
extern struct configstruct config;
extern char fsscreen[], fsscreen7bit[];
extern const char keys[];
extern unsigned int slavepid;

unsigned int maxpos=0, pl_maxpos=0, pl_current=0;
int  pl_buttonpos=1, pl_screenmark=0;

struct playlistent *rplaylist(struct playlistent *playlist, unsigned int *filenumber) {
int    ch;
fd_set fds;

   if ( config.ttymode == 8 ) 
     printf("%s", fsscreen); else
     printf("%s", fsscreen7bit);
   if ( playlist != NULL ) {
      pl_maxpos = pl_count(playlist)-1;
      pl_current = *filenumber;
   } else 
     pl_current = 0;
   pl_screenmark = 0;
   pl_showents(pl_current-pl_screenmark, playlist); 
   pl_updatebuttons(0);
   
   while ( TRUE ) {
      
      FD_ZERO(&fds);
      FD_SET(0, &fds);
#ifdef USE_GPM_MOUSE
      if ( gpm_flag ) FD_SET(gpm_fd, &fds);
      if ( select(gpm_flag ? gpm_fd+1 : 1, &fds, NULL, NULL, NULL) > 0 ) {
	 if ( FD_ISSET(gpm_fd, &fds) ) playlist = pl_domouse(playlist, filenumber);
	 if ( pl_screenmark == -1 ) return(playlist);
#else
      if ( select(1, &fds, NULL, NULL, NULL) > 0 ) {
#endif	 
	 ch = 0;
	 if ( FD_ISSET(0, &fds) )
	   while ( ch != -1 ) {
	      ch = getchar();
	      if ( ch == 3 ) exit(0); /* ^C */ else
		if ( ch == 13 )	if ( pl_buttonpos == PL_MAXBUTTON ) return(playlist); else
		playlist = pl_dofunction(playlist, filenumber); else
		if ( ch == 27 ) {
		   ch = getchar();
		   if ( ch == -1 ) {
		      mykbhit(0, 250000);
		      ch = getchar();
		   }		   
		   if ( ch != '[' ) return playlist;
		   ch = getchar();
		   if ( ch == 'D' && pl_buttonpos != PL_MINBUTTON ) {
		      pl_updatebuttons(-1); /* left arrow */
		   }
		   if ( ch == 'C' && pl_buttonpos != PL_MAXBUTTON ) { 
		      pl_updatebuttons(1); /* right arrow */
		   }
		   if ( ch == 'B' && pl_current != pl_maxpos ) {
		      if ( pl_screenmark != 12 ) /* down arrow */
			pl_screenmark++;
		      pl_current++;
		      pl_showents(pl_current-pl_screenmark, playlist);
		   }
		   if ( ch == 'A' && pl_current != 0 ) {
		      if ( pl_screenmark != 0 ) /* up arrow */
			pl_screenmark--;
		      pl_current--;
		      pl_showents(pl_current-pl_screenmark, playlist);
		   }
		   if ( ch == keys[KEY_PGUP] ) { /* page up */
		      if ( pl_current < 13 ) { pl_current = 0; pl_screenmark = 0; } else
			pl_current -= 11;
		      if ( pl_current < pl_screenmark )
			pl_current = pl_screenmark;
		      pl_showents(pl_current-pl_screenmark, playlist);
		   }
		   if ( ch == keys[KEY_PGDN] ) { /* page down */
		      if ( pl_current > pl_maxpos-11 ) { pl_screenmark = pl_maxpos - pl_current; pl_current = pl_maxpos; } else
			pl_current += 11;
		      pl_showents(pl_current-pl_screenmark, playlist);
		   }
		   if ( ch == keys[KEY_HOME] ) { /* home */
		      pl_current = 0;
		      pl_screenmark = 0;
		      pl_showents(0, playlist);
		   }
		   if ( ch == keys[KEY_END] ) { /* end */
		      pl_current = pl_maxpos;
		      pl_screenmark = 12;
		      pl_showents(pl_current-pl_screenmark, playlist);
		   }
		}
	   }
      }
   }
}

   
void pl_showents( int startpos, struct playlistent *playlist ) {
int i=0, k=0;
char shortname[61];
char buf[70];
   
   playlist = pl_seek(startpos, playlist);
   printf("\e[0;46m");
   
   while ( playlist != NULL && playlist->next != NULL ) {
      if ( playlist->number == pl_current ) printf("\e[31m"); else
	printf("\e[34m");
      memset(buf, 0, 70);
      memset(shortname, 0, 60);
      strncpy(buf, playlist->showname, 52);
      
      if ( config.showtime > 1 ) 
	if ( playlist->length != 0 ) sprintf(shortname, "[%02u:%02u] %s", playlist->length / 60, playlist->length % 60, buf ); else
	sprintf(shortname, "[error] %s", playlist->showname, 59); else
	strncpy(shortname, playlist->showname, 59);      
      for(k=strlen(shortname);k<60;k++) shortname[k] = ' ';
      shortname[60] = 0;
      printf("\e[%d;8H%-4.d %s",5+i,playlist->number+1, shortname);
      i++;
      if (i>12) break;
      playlist = playlist->next;
   }
   if ( i != 13 ) 
     for(;i<13;i++) 
       printf("\e[%d;8H                                                                 ",5+i);
   fflush(stdout);
}


char *xys(unsigned char number, char ch) {
static char buf[256];
   memset(buf, ch, number);
   buf[number+1] = '\000';
   return buf;
}


struct playlistent *pl_seek( unsigned int pos, struct playlistent *playlist ) {

   if ( playlist == NULL ) return NULL;
   
   if ( playlist->number < pos )
     while ( (playlist->number != pos) && (playlist->next != NULL) ) playlist = playlist->next;
   
   if ( playlist->number > pos ) 
     while ( (playlist->number != pos) && (playlist->prev != NULL) ) playlist = playlist->prev;

return playlist;
}

unsigned int pl_count( struct playlistent *playlist ) {
unsigned int count=0;
   if ( playlist==NULL ) return 0;
   playlist = pl_seek(0, playlist);
   while( playlist->next != NULL ) {
      count++;
      playlist=playlist->next;
   }
   return count;
}


void pl_updatebuttons(int add) {

   pl_buttonpos = pl_buttonpos + add;
   printf("\e[19;9H");
   if (pl_buttonpos == 1) printf("\e[44;36;1m"); else
     printf("\e[34;44;1m");
   printf(" browse ");
   printf("\e[19;18H");
   if (pl_buttonpos == 2) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  play  ");
   printf("\e[19;27H");
   if (pl_buttonpos == 3) printf("\e[44;36m"); else
     printf("\e[34;44m");
   printf(" remove ");
   printf("\e[19;36H");
   if (pl_buttonpos == 4) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" newlst ");
   printf("\e[19;45H");
   if (pl_buttonpos == 5) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  desc  ");
   printf("\e[19;54H");
   if (pl_buttonpos == 6) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  sort  ");
   printf("\e[19;63H");
   if (pl_buttonpos == 7) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  back  ");
   fflush(stdout);
}


struct playlistent *pl_dofunction(struct playlistent *playlist, unsigned int *filenumber) {
struct playlistent *temp;
int pid;   
   
   switch( pl_buttonpos ) {
      
    case 1: /* browse */
      playlist = getfiles(playlist);
      if ( config.ttymode == 8 ) 
	printf("%s", fsscreen); else
	printf("%s", fsscreen7bit);
      if ( playlist == NULL ) pl_maxpos = 0; else
	pl_maxpos = pl_count(playlist)-1;
      if (pl_current - pl_screenmark > pl_maxpos)
	pl_current = pl_screenmark = 0;
      pl_showents(pl_current, playlist); 
      pl_updatebuttons(0);
      return( playlist );
      
    case 2: /* play */ 
      if ( playlist == NULL ) return NULL;
      if ( playsong && !slavepid ) killslave();
      *filenumber = pl_current;
      playsong = TRUE;
      call_player(pl_seek(pl_current, playlist));
      return(playlist);
      
    case 3: /* remove current entry */  
      if ( playlist == NULL ) return NULL;
      playlist = pl_seek(pl_current, playlist);
      temp = playlist;
      if ( pl_current != 0 )
	playlist->prev->next = playlist->next;
      playlist->next->prev = playlist->prev;
      while ( playlist->next != NULL ) {
	 playlist = playlist->next;
	 playlist->number--;
      }
      if ( pl_current == pl_maxpos ) pl_current--;
      pl_maxpos--;
      free(temp);
      pl_showents(pl_current-pl_screenmark, playlist); 
      return(playlist);
      
    case 4: /* new list (clear) */
      playlist = clearplaylist(playlist);
      pl_showents(0, playlist);
      pl_current = 0;
      pl_screenmark = 0;
      return(playlist);

    case 5: /* id3 editor */
      if ( playlist == NULL ) return playlist;
      id3edit(pl_seek(pl_current, playlist)->name, pl_seek(pl_current, playlist));
      if ( config.ttymode == 8 ) 
	printf("%s", fsscreen); else
	printf("%s", fsscreen7bit);
      pl_showents(pl_current-pl_screenmark, playlist); 
      pl_updatebuttons(0);
      return playlist;
      
    case 6: /* sort playlist */
      playlist = sortplaylist(playlist);
      pl_showents(pl_current-pl_screenmark, playlist);       
      return playlist;
   }
}

#ifdef USE_GPM_MOUSE
struct playlistent *pl_domouse(struct playlistent *playlist, unsigned int *filenumber) {
Gpm_Event event;
int i;
   
   Gpm_GetEvent(&event);
   /* middle button, any kind of click */
   if ( event.buttons & 2 && event.type & GPM_DOWN ) {
      if ( config.ttymode == 8 ) 
	printf("%s", fsscreen); else
	printf("%s", fsscreen7bit);
      pl_showents(pl_current-pl_screenmark, playlist); 
      pl_updatebuttons(0);
   }
   
   /* right button, any kind of click */
/*
   if ( event.buttons & 1 && event.type & GPM_DOWN ) {
      pl_screenmark = -1;
      return playlist;
   }
 */
   
   /* left button - any kind of click */
   if ( event.buttons & 4 && event.type & GPM_DOWN ) 
     if ( event.y == 4 && event.x > 4 && event.x < 74 ) {
	/* page up */
	if ( pl_current < 13 ) { pl_current = 0; pl_screenmark = 0; } else
	  pl_current -= 11;
	if ( pl_current < pl_screenmark )
	  pl_current = pl_screenmark;
	pl_showents(pl_current-pl_screenmark, playlist);
     } else	
     if ( event.y == 18 && event.x > 4 && event.x < 74 ) {
	/* page down */
	if ( pl_current > pl_maxpos-11 ) { pl_screenmark = pl_maxpos - pl_current; pl_current = pl_maxpos; } else
	  pl_current += 11;
	pl_showents(pl_current-pl_screenmark, playlist);
     } else
     if ( event.y == 19 ) {
	i = 255;
	if ( event.x > 8 && event.x < 17 ) pl_buttonpos = 1; else
	  if ( event.x > 17 && event.x < 26 ) pl_buttonpos = 2; else
	  if ( event.x > 26 && event.x < 35 ) pl_buttonpos = 3; else
	  if ( event.x > 35 && event.x < 44 ) pl_buttonpos = 4; else
	  if ( event.x > 44 && event.x < 53 ) pl_buttonpos = 5; else
	  if ( event.x > 53 && event.x < 62 ) pl_buttonpos = 6; else
	  if ( event.x > 62 && event.x < 71 ) {
	     pl_buttonpos  = PL_MAXBUTTON;
	     pl_screenmark = -1;
	     return playlist;
	  } else
	  i = 0;
	if ( i == 255 ) { 
	   pl_updatebuttons(0);
	   playlist = pl_dofunction(playlist, filenumber);
	}
     }
   
   /* left button - single click */
   if ( event.buttons & 4 && event.type & GPM_SINGLE )
     if ( event.y > 4 && event.y < 18 && event.x > 4 && event.x < 74 && \
        (pl_current-pl_screenmark+event.y-5) <= pl_maxpos ) {
	/* Select the selected file :) */
	pl_current = pl_current-pl_screenmark + event.y - 5;
	pl_screenmark = event.y - 5;
	pl_showents(pl_current-pl_screenmark, playlist);
     } 

   /* left button - double click */	  
   if ( event.buttons & 4 && event.type & GPM_DOUBLE )
     if ( event.y > 4 && event.y < 18 && event.x > 4 && event.x < 74 && \
        (pl_current-pl_screenmark+event.y-5) <= pl_maxpos ) {
	/* Play the selected file */
	pl_current = pl_current-pl_screenmark + event.y - 5;
	pl_screenmark = event.y - 5;
	pl_showents(pl_current-pl_screenmark, playlist);
	i = pl_buttonpos;
	pl_buttonpos = 2;
	playlist = pl_dofunction(playlist, filenumber);
	pl_buttonpos = i;
     }
   
   /* right button, any kind of click */
   if ( event.buttons & 1 && event.type & GPM_DOWN ) 
     if ( event.y == 4 && event.x > 4 && event.x < 74 ) {
	/* home */
	pl_current = 0;
	pl_screenmark = 0;
	pl_showents(0, playlist);
     } else
     if ( event.y == 18 && event.x > 4 && event.x < 74 ) {
	/* end */
	pl_current = pl_maxpos;
	pl_screenmark = 12;
	pl_showents(pl_current-pl_screenmark, playlist);
     }
   Gpm_GetSnapshot(&event);
   if ( !event.y ) event.y++;
   if ( !event.x ) event.y++;
   GPM_DRAWPOINTER(&event);
   return playlist;
}
#endif
      

char *lowercases(char *str) {
static char buf[256];
int i=0;
   for(;i<strlen(str)+1;i++) buf[i] = tolower(str[i]);
   return buf;
}


struct playlistent *addfiletolist(struct playlistent *playlist, char *filename, char *showname, unsigned int bitrate, unsigned int samplerate, unsigned char mode, char scanid3 ) {
char   name[31], artist[31], cwd[256];
int    cspos=0, j;
struct stat statf;
size_t filesize;
   
   if ( !exist(filename) ) return playlist;
   
   if ( playlist == NULL ) {
      playlist = (struct playlistent*)malloc(sizeof(struct playlistent));
      playlist->prev = NULL;
      playlist->next = NULL;
      playlist->number = 0;
   } else
     while ( playlist->next != NULL ) playlist = playlist->next;
   
   playlist->next = (struct playlistent*)malloc(sizeof(struct playlistent));
   playlist->next->prev = playlist;
   playlist->next->next = NULL;
   playlist->next->number = playlist->number +1;
   playlist->samplerate = samplerate;
   playlist->bitrate = bitrate;
   if ( filename[0] != '/' ) {
      getcwd(cwd, 256);
      sprintf(playlist->name, "%s/%s", cwd, filename);
   } else  
     strcpy(playlist->name, filename);

   if ( showname == NULL ) {
      if ( scanid3 && getmp3info(filename, &mode, &samplerate, &bitrate, name, artist, NULL, NULL, NULL, 0) ) {
	 if ( artist[0] != 0 ) sprintf(playlist->showname, "%s - %s", artist, name ); else
	   strcpy(playlist->showname, name);
      } else
	for(j=0;j<strlen(filename);j++) {
	   playlist->showname[cspos] = tolower(filename[j]);
#ifdef NICE_NAMES
	      if ( filename[j] == '.' || filename[j] == '_' ) playlist->showname[cspos] = 32; else
	     if ( filename[j] == '-' ) {
		playlist->showname[cspos]    = ' ';
		playlist->showname[cspos+1]  = '-';
		playlist->showname[cspos+2]  = ' ';
		cspos += 2;
	     } 
#endif
	   if ( filename[j] == '/' ) cspos = 0; else
	     cspos++;
	}
      playlist->showname[cspos - 4] = 0;
   } else
     strcpy(playlist->showname, showname);
   
   stat(filename, (struct stat*)&statf);

   if ( bitrate+samplerate != 0 ) {
      playlist->bitrate    = bitrate;
      playlist->samplerate = samplerate;
      playlist->mode       = mode;
      playlist->length     = statf.st_size / (bitrate * 125);
   }
   
return playlist;
}


struct playlistent *clearplaylist(struct playlistent *playlist) {
struct playlistent *temp;
   
   if ( playlist != NULL ) {
      while ( playlist->prev != NULL ) playlist = playlist->prev;
      while ( playlist != NULL ) {
	 temp     = playlist;
	 playlist = playlist->next;
	 free(temp);
      }
   } 
   return playlist;
}


struct playlistent *sortplaylist(struct playlistent *playlist) {
struct playlistent *sortedlist, *temp;
int i,j;
unsigned int playlistents = pl_count(playlist);
   
   if ( playlist == NULL ) return playlist;
   sortedlist = (struct playlistent*)malloc(sizeof(struct playlistent));
   sortedlist->prev = NULL;
   sortedlist->next = NULL;
   sortedlist->number = 0;
   
   for(i=0;i<playlistents;i++) {
      playlist = pl_seek(0, playlist);
      temp = playlist;
      while ( playlist->next != NULL && playlist != NULL ) {
	 
	 for(j=0;j<strlen(playlist->showname);j++)
	   
	   if( tolower(playlist->showname[j]) < tolower(temp->showname[j]) ) {
	      temp = playlist;
	      break;
	   } else
	   if ( tolower(playlist->showname[j]) != tolower(temp->showname[j]) )
	     break;	 
	 playlist = playlist->next;
      }
      sortedlist->next = (struct playlistent*)malloc(sizeof(struct playlistent));
      sortedlist->next->prev = sortedlist;
      sortedlist->next->next = NULL;
      sortedlist->next->number = sortedlist->number+1;
      memcpy((struct oneplaylistent*)sortedlist, temp, sizeof(struct oneplaylistent));
      sortedlist = sortedlist->next;
      
      if ( temp->prev != NULL ) temp->prev->next = temp->next; else
	temp->next->prev = NULL;
      if ( temp->next != NULL ) temp->next->prev = temp->prev; else
	temp->prev->next = NULL;
      free(temp);
   }
   return(sortedlist);
}

   
void l_status(char *text) {
   if ( text != NULL )
   printf("\e[19;8H\e[1;36;46m                                                              \e[19;8H%s", text); else
   printf("\e[19;8H\e[1;36;46m                                                              ");
   fflush(stdout);
}
