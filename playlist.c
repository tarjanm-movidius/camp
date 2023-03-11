#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "camp.h"

#ifdef HAVE_TERMIOS_H
# include <fcntl.h>
#else
# include <curses.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

#ifdef LIRCD
#include "lirc_client.h"
#endif

extern char playsong, checkkill, use_lircd, currloc;
extern struct configstruct config;
extern const char keys[];
extern unsigned int slavepid;
extern int selval, lirc_lircd;
extern struct lirc_config *lircd_config;

int pl_maxpos=0, pl_current=0;
int pl_buttonpos, pl_screenmark=0;

void rplaylist(struct playlistent **playlist, unsigned int *filenumber) {
int    ch, ch2, prevsong=-1;
fd_set fds;
struct timeval counter;

   currloc = CAMP_PL;
   pl_buttonpos = config.skin.psb;
   if ( config.skin.pclr ) printf("\e[0m\e[2J");
   printf("\e[0m\e[1;1H%s", config.skin.playlist);
   if ( config.skin.mainatpl ) {
      updatedata();
      updatesongtime('f');
   }
   
   if ( *playlist != NULL ) {
      pl_maxpos = pl_count(*playlist)-1;
      if ( !config.skin.platmain ) pl_current = *filenumber;
   } else {
      pl_current = 0;
      pl_screenmark = 0;
   }
   //  if ( pl_current-pl_screenmark < 0 ) pl_screenmark = 0;
 
   pl_showents(pl_current-pl_screenmark, *playlist, filenumber);
   pl_updatebuttons(0);
   
   findit(NULL, 0);
   
   while ( TRUE ) {

      if ( config.mpg123 )
	mpg123_control(NULL);
            
      if ( checkkill && kill(slavepid, 0) == -1 ) {      
	 checkkill = FALSE;
	 playnext(-1);
      }
      
   if (prevsong != *filenumber)
	pl_showents(pl_current-pl_screenmark, *playlist, filenumber);
      
      prevsong=*filenumber;
      
      
#ifdef RC_ENABLED
      if ( config.userc ) checkrc();
#endif
      counter.tv_usec = config.rctime;
      counter.tv_sec  = 0;
      FD_ZERO(&fds);
      FD_SET(0, &fds);
      
#ifdef USE_GPM_MOUSE
      if ( gpm_flag > 0 ) FD_SET(gpm_fd, &fds);
#endif
#ifdef LIRCD
      if ( use_lircd ) FD_SET(lirc_lircd, &fds);
#endif
      
      if ( config.skin.mainatpl ) updatesongtime('u');
      
      if ( select(selval, &fds, NULL, NULL, &counter) > 0 ) {      
#ifdef USE_GPM_MOUSE
	 if ( gpm_flag > 0 && FD_ISSET(gpm_fd, &fds) ) {
            pl_domouse(playlist, filenumber, NULL);
	    if ( currloc == CAMP_RET ) return;
	 }
#endif
#ifdef LIRCD
	 if ( use_lircd && FD_ISSET(lirc_lircd, &fds) ) {
	    if ( pl_dolircd(playlist, filenumber) ) return;
	 }
#endif
	         
	 ch = 0;

	 if ( FD_ISSET(0, &fds) ) {
	    ch = getchar();
	    if ( ch == 3 && canexit() ) { clearplaylist(playlist); exit(0); } /* ^C */ else
              if ( ch == 26 && canexit() ) disappear(); else /* ^Z */	      
	      if ( ch == 12 ) { /* ^L */
		 printf("\e[0m\e[2J\e[1;1H%s", config.skin.playlist);
		    pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
		    pl_updatebuttons(0);
		 if ( config.skin.mainatpl ) {
		    updatedata();
		    updatesongtime('f');
		 }		 
	      } else				    
	      if ( ch == 13 ) {
		 pl_dofunction(playlist, filenumber, -1);
		 if ( currloc == CAMP_RET ) return;
	      } else
#ifdef HAVE_SYS_SOUNDCARD_H
	      if (ch == '+') set_volume(config.voldev, config.volstep);  else
	      if (ch == '-') set_volume(config.voldev, -config.volstep); else
#endif
	      if ( ch == 27 ) {
		   
#ifdef HAVE_TERMIOS_H
		   fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
#endif		   
		 ch = getchar();
		 if ( ch == -1 ) {
		    mykbhit(0, 250000);
		    ch = getchar();
		 } 		   
		 ch2 = getchar();
		 
#ifdef HAVE_TERMIOS_H
		 fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#endif		   
		 if ( ch == -1 || ch == 27 ) return; 
		   if ( ch2 == 'D' && pl_buttonpos != PL_MINBUTTON ) {
		      pl_updatebuttons(-1); /* left arrow */
		   } else
		   if ( ch2 == 'C' && pl_buttonpos != PL_MAXBUTTON ) { 
		      pl_updatebuttons(1); /* right arrow */
		   } else
		   if ( ch2 == 'B' && pl_current != pl_maxpos ) {
		      if ( pl_screenmark != (config.skin.plistlines-1) ) /* down arrow */
			pl_screenmark++;
		      pl_current++;
		      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
                      findit(NULL,0);
		   } else
		   if ( ch2 == 'A' && pl_current != 0 ) {
		      if ( pl_screenmark != 0 ) /* up arrow */
			pl_screenmark--;
		      pl_current--;
		      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
                      findit(NULL,0);
		   } else
		   if ( ch2 == keys[KEY_PGUP] && pl_current != 0  && *playlist ) { /* page up */
		      if ( pl_current < (config.skin.plistlines) ) {
			 pl_current = 0; 
			 pl_screenmark = 0; } else
			pl_current -= (config.skin.plistlines-1);
		      if ( pl_current < pl_screenmark )
			pl_current = pl_screenmark;
		      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
                      findit(NULL,0);
		   } else
		   if ( ch2 == keys[KEY_PGDN] && pl_current != pl_maxpos && *playlist ) { /* page down */
		      if ( pl_current+(config.skin.plistlines-1) > pl_maxpos ) {
			 pl_current = pl_maxpos;
			 pl_screenmark = 0;
		      } else
			pl_current += (config.skin.plistlines-1);
		      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
                      findit(NULL,0);
		   } else
		   if ( ch2 == keys[KEY_HOME] && pl_current != 0 && *playlist ) { /* home */
		      pl_current = 0;
			pl_screenmark = 0;
		      pl_showents(0, *playlist,filenumber);
                      findit(NULL,0);
		   } else
		   if ( ch2 == keys[KEY_END] && pl_current != pl_maxpos && *playlist ) { /* end */
		      pl_current = pl_maxpos;
		      pl_screenmark = 0;		     
		      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
                      findit(NULL,0);
		   }
	      } else
	      if ( ch > 31 /*&& ch < 128*/ ) 
	      pl_search(ch, *playlist, filenumber); /*- any other button */
      }
      }
   }
}


static int findit (struct playlistent *pl, char ch)
{
static char c[100];
static cont=0;
char tmp[100];
int i;
	
   if (pl == NULL) {
      switch (ch) {
       case 0:
	 cont=0;
	 break;	 
       case 127:
	 if (cont != 0)
	   cont--;
	 break;
       default:
	 if (cont != 100 && ch != 0x7e ) 
	   c[cont++]=tolower(ch);
	 break;
      }
      if ( config.skin.findbarw && config.skin.findbarc ) { /* If this skin supports showing the searchbar .. */
	 for (i=cont;i<99;i++)
	   c[i]=' ';
	 c[99]='\0';
	 memset(tmp, 0, 100);
	 strncpy(tmp, c, config.skin.findbarw);
	 printf("\e[%d;%dH\e[%sm%s",config.skin.findbary, config.skin.findbarx, config.skin.findbarc, tmp);
      }
      c[cont]='\0';
   }
   else  { // We do the actual search
      char *buftmp, *tmp;
      if (c[0]=='\0')
	return 0;
      buftmp=strdup(c);
      tmp=strtok(c," ");
      while (tmp != NULL) {
	 
	 if ( !strcasestr(pl->showname, tmp) ) {
	    strcpy(c,buftmp);
	    free(buftmp);
	    return 0;
	 }
	 tmp=strtok(NULL," ");
      }
      strcpy(c,buftmp);
      free(buftmp);
      return 1;
   }
}

void pl_search(char ch, struct playlistent *playlist,int *filenumber)	
{  
   int finish_playlist;
   
   if ( !playlist )
	   return;
   
   findit(NULL,ch);

   pl_seek(pl_current,&playlist);
  
   if (pl_current != 0)
     finish_playlist=pl_current-1;
   else
     finish_playlist=pl_maxpos+1;

/*   if ( pl_current != pl_maxpos ) 
	   pl_seek(pl_current+1, &playlist);
   else
	   pl_seek(0, &playlist);
*/ 
   while ( playlist->number != finish_playlist ) {

      if ( findit (playlist,0) ) {
	 pl_current = playlist->number;
	 pl_screenmark = 0;
	 pl_showents(pl_current-pl_screenmark, playlist,filenumber);
	 return;
      }
      
      if (playlist->next == NULL) 
	      pl_seek(0, &playlist);
      else
	      playlist= playlist->next;      
   }
}
   
   
void pl_showents( int startpos, struct playlistent *playlist, int *filenumber ) {
int i=0, k=0;
char shortname[100];
int count=0;
   
   if ( startpos < 0 ) startpos = 0;
   
   pl_seek(startpos, &playlist);
   printf("\e[%sm", config.skin.plistci);
   
   while ( playlist != NULL && playlist->next != NULL ) {
      if ( playlist->number == pl_current )
	 printf("\e[%sm", config.skin.plistca); else
      if (playlist->number == *filenumber && playsong && config.skin.plistcc )
	 printf("\e[%sm", config.skin.plistcc); else
	printf("\e[%sm", config.skin.plistci);
      
      memset(shortname, 0, 100);
      
      if ( config.showtime > 1 )
	if ( playlist->length ) sprintf(shortname, "[%02u:%02u] ", playlist->length / 60, playlist->length % 60 ); else
	if ( !strncasecmp(playlist->showname, "http://", 7) )
	  strcpy(shortname, "[strem] "); else
	strcpy(shortname, "[ n/a ] "); 
  	
      strncat(shortname, playlist->showname, config.skin.plistw-(5+strlen(shortname)));
      for (k=strlen(shortname);k<config.skin.plistw-5;k++) shortname[k] = ' ';
      
      printf("\e[%d;%dH%-4.d %s", config.skin.plisty+i, config.skin.plistx, playlist->number+1, shortname);
      i++;
      if (i>=config.skin.plistlines) break;
      playlist = playlist->next;
   }
   printf("\e[%sm", config.skin.plistci);

   if ( i != config.skin.plistlines )
     for(;i<config.skin.plistlines;i++)
       printf("\e[%d;%dH%s", config.skin.plisty+i, config.skin.plistx, xys(config.skin.plistw, ' '));
   fflush(stdout);

}


char *xys(unsigned char number, char ch) {
static char buf[256];
   memset(buf, ch, 256);
   buf[number] = '\000';
   return buf;
}


struct playlistent *pl_seek( unsigned int pos, struct playlistent **playlist ) {

   if ( *playlist == NULL ) return NULL;
   
   if ( (*playlist)->number < pos )
     while ( ((*playlist)->number != pos) && ((*playlist)->next != NULL) ) *playlist = (*playlist)->next;
   
   if ( (*playlist)->number > pos ) 
     while ( ((*playlist)->number != pos) && ((*playlist)->prev != NULL) ) *playlist = (*playlist)->prev;
   
return *playlist;
}

unsigned int pl_count( struct playlistent *playlist ) {
unsigned int count=0;
   if ( !playlist ) return 0;
   pl_seek(0, &playlist);
   while( playlist->next != NULL ) {
      count++;
      playlist=playlist->next;
   }
   return count;
}


void pl_updatebuttons(int add) {
int i;
   if ( config.skin.plistbo[pl_buttonpos+add] == -1 ) return;
   pl_buttonpos = pl_buttonpos + add;

   for (i=PL_MINBUTTON;i<(PL_MAXBUTTON+1);i++) {
      if ( !config.skin.py[i] && !config.skin.px[i] ) continue;
      printf("\e[%d;%dH",config.skin.py[i], config.skin.px[i]);
      if (config.skin.plistbo[pl_buttonpos] == i) printf("%s", config.skin.pa[i]); else
	printf("%s", config.skin.pi[i]);
   }
   fflush(stdout);
}


void pl_dofunction(struct playlistent **playlist, unsigned int *filenumber, char forcedbutton) {
struct playlistent *temp = NULL;
int length;   
   
   if ( forcedbutton == -1 ) forcedbutton = config.skin.plistbo[pl_buttonpos];
   
   switch( forcedbutton ) {
      
    case 0: /* browse */
      length = currloc;
      getfiles(playlist);
      currloc = length;
      if ( config.skin.fclr ) printf("\e[0m\e[2J");
      printf("\e[0m\e[1;1H%s", config.skin.playlist);
      if ( config.skin.mainatpl ) {
	 updatedata();
	 updatesongtime('f');
      }
      pl_maxpos = pl_count(*playlist)-1;
      if ( *playlist == NULL )
	pl_maxpos = pl_screenmark = pl_current = 0; else
	if (pl_current > pl_maxpos)
	  pl_current = pl_screenmark = 0;
      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
      findit(NULL,0);
      pl_updatebuttons(0);
      return;
      
    case 1: /* play */ 
      if ( *playlist == NULL ) return;
      findit(NULL,0);
      if ( playsong && !slavepid ) killslave();
      *filenumber = pl_current;
      playsong = TRUE;
      pl_seek(pl_current, playlist);
      length = (*playlist)->length;
      call_player(*playlist);
//      if ( length != (*playlist)->length )
      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
      return;
      
    case 2: /* remove current entry */  
      if ( !(*playlist) ) return;
      
      pl_seek(pl_current, playlist);
      if ( (*playlist)->next ) temp = (*playlist)->next; else
	if ( (*playlist)->prev ) temp = (*playlist)->prev; else
	temp = NULL; /* should not be possible? Uhm, no .. */
      
      if ( (*playlist)->next ) (*playlist)->next->prev = (*playlist)->prev;
      if ( (*playlist)->prev ) (*playlist)->prev->next = (*playlist)->next;
      
      free(*playlist);
      
      *playlist = temp;
      
      if ( *playlist ) (*playlist)->number--;
      while ( *playlist && (*playlist)->next ) {
	 *playlist = (*playlist)->next;
	 (*playlist)->number--;
      } 
      
      if ( pl_current == pl_maxpos ) {
	 pl_current--;
	 if ( (config.skin.plistlines-1) > pl_maxpos && pl_screenmark )
	   pl_screenmark--;
      }
      
      if ( !(*playlist)->next && !(*playlist)->prev ) {
	 free(*playlist); /* We did run out of entries, free "extra" entry */
	 *playlist = NULL;
      } else
	pl_maxpos--;
      
      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
      return;
      
    case 3: /* new list (clear) */

      findit(NULL,0);
      clearplaylist(playlist);
      pl_showents(0, *playlist,filenumber);
      pl_current = 0;
      pl_screenmark = 0;
      return;

    case 4: /* id3 editor */
      if ( *playlist == NULL ) return;
      id3edit(pl_seek(pl_current, playlist)->name, pl_seek(pl_current, playlist));
#ifdef LIRCD
      clear_lirc();
#endif
      findit(NULL,0);
      if ( config.skin.miclr ) printf("\e[0m\e[2J");
      if ( config.skin.pclr ) printf("\e[0m\e[2J");
      printf("\e[0m\e[1;1H%s", config.skin.playlist);
      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
      pl_updatebuttons(0);
      return;
      
    case 5: /* sort playlist */
      sortplaylist(playlist);
      pl_showents(pl_current-pl_screenmark, *playlist,filenumber);       
      return;

    case 6:
      currloc = CAMP_RET;
      return;
      
   }
   
}

#ifdef USE_GPM_MOUSE
void pl_domouse(struct playlistent **playlist, unsigned int *filenumber, Gpm_Event *event) {
int i,j=PL_MINBUTTON;
   
   if ( !event ) {
      event = malloc(sizeof(Gpm_Event));
      Gpm_GetEvent(event);
   }

   /* middle button, any kind of click */
   if ( event->buttons & 2 && event->type & GPM_DOWN ) {
      printf("\e[0m\e[2J\e[1;1H%s", config.skin.playlist);
      pl_showents(pl_current-pl_screenmark, *playlist,filenumber); 
      pl_updatebuttons(0);
   }
   
   /* right button, any kind of click */
/*
   if ( event.buttons & 1 && event.type & GPM_DOWN ) {
      currloc = CAMP_RET;
      return;
   }
 */
   
   /* left button - any kind of click */
   if ( event->buttons & 4 && event->type & GPM_DOWN && !(event->type & GPM_DRAG) ) 
     if ( event->y == config.skin.plisty-1 && event->x >= config.skin.plistx && event->x < (config.skin.plistx+config.skin.plistw) ) {
	/* page up */
	if ( pl_current < config.skin.plistlines ) { pl_current = 0; pl_screenmark = 0; } else
	  pl_current -= (config.skin.plistlines-1);
	if ( pl_current < pl_screenmark )
	  pl_current = pl_screenmark;
	pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
     } else	
     if ( event->y == config.skin.plisty+config.skin.plistlines && event->x >= config.skin.plistx && event->x < (config.skin.plistx+config.skin.plistw) ) {
	/* page down */
	if ( pl_current+(config.skin.plistlines-1) > pl_maxpos ) {	   
	   pl_current = pl_maxpos;
	   pl_screenmark = 0;
	} else
	  pl_current += (config.skin.plistlines-1);
	pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
     } else          
     for (i=PL_MINBUTTON;i<(PL_MAXBUTTON+1);i++)
       if ( event->y == config.skin.py[i] && event->x >= (config.skin.px[i]-config.skin.pmouseexpand) && event->x < (config.skin.px[i]+config.skin.pw[i]+config.skin.pmouseexpand) ) {
	  for(;j<(PL_MAXBUTTON+1);j++)
	    if ( config.skin.plistbo[j] == i )
	      pl_buttonpos = j;
	  pl_updatebuttons(0);
	  pl_dofunction(playlist, filenumber, -1);
	  break; 
       }
   
   /* left button - single click */
   if ( event->buttons & 4 && event->type & GPM_SINGLE )
     if ( event->y >= config.skin.plisty && event->y < (config.skin.plisty+config.skin.plistlines) && \
        event->x >= config.skin.plistx  && event->x < (config.skin.plistx+config.skin.plistw) && \
        (pl_current-pl_screenmark+event->y-config.skin.plisty) <= pl_maxpos ) {
	/* Select the selected file :) */
	pl_current = pl_current-pl_screenmark + event->y - config.skin.plisty;
	pl_screenmark = event->y - config.skin.plisty;
	pl_showents(pl_current-pl_screenmark, *playlist, filenumber);
     } 

   /* left button - double click */	  
   if ( event->buttons & 4 && event->type & GPM_DOUBLE )
     if ( event->y >= config.skin.plisty && event->y < (config.skin.plisty+config.skin.plistlines) && \
        event->x >= config.skin.plistx && event->x < (config.skin.plistx+config.skin.plistw) && \
        (pl_current-pl_screenmark+event->y-config.skin.plisty) <= pl_maxpos ) {
	/* Play the selected file */
	pl_current = pl_current-pl_screenmark + event->y - config.skin.plisty;
	pl_screenmark = event->y - config.skin.plisty;
	pl_showents(pl_current-pl_screenmark, *playlist, filenumber);

	pl_dofunction(playlist, filenumber, 1);
/*	^ - That instead ?
        i = pl_buttonpos;
	pl_buttonpos = 2;
	pl_dofunction(playlist, filenumber, -1);
	pl_buttonpos = i; */
     }
   
   /* right button, any kind of click */
   if ( event->buttons & 1 && event->type & GPM_DOWN && !(event->type & GPM_DRAG) )
     if ( event->y == (config.skin.plisty-1) && event->x >= config.skin.plistx && event->x < (config.skin.plistx+config.skin.plistw) ) {
	/* home */
	pl_current = 0;
	pl_screenmark = 0;
	pl_showents(0, *playlist, filenumber);
     } else
     if ( event->y == (config.skin.plisty+config.skin.plistlines) && event->x >= config.skin.plistx && event->x < (config.skin.plistx+config.skin.plistw) ) {
	/* end */
	pl_current = pl_maxpos;
	pl_screenmark = 0;
	pl_showents(pl_current-pl_screenmark, *playlist, filenumber);
     }

   if ( config.skin.mainatpl && currloc == CAMP_PL )
     main_domouse(event);
   
   if ( currloc == CAMP_PL && event ) free(event);
   return;
}
#endif
      

char *lowercases(char *str) {
static char buf[256];
int i=0;
   for(;i<strlen(str)+1;i++) buf[i] = tolower(str[i]);
   return buf;
}


void addfiletolist(struct playlistent **playlist, char *filename, char *showname, unsigned int bitrate, unsigned int samplerate, unsigned char mode, char scanid3 ) {
char   name[31], artist[31], cwd[256], *buf;
int    cspos, j, lastdot;
struct stat statf;
size_t filesize;
   
   if ( config.mpg123 )
     mpg123_control(NULL);
   
   if ( !exist(filename) && strncasecmp(filename, "http://", 7) ) return;
   buf = (char*)malloc(300);
   
   if ( *playlist == NULL ) {
      *playlist = (struct playlistent*)malloc(sizeof(struct playlistent));
      memset(*playlist, 0, sizeof(struct playlistent));
   } else
     while ( (*playlist)->next != NULL ) *playlist = (*playlist)->next;
   
   (*playlist)->next = (struct playlistent*)malloc(sizeof(struct playlistent));
   memset((*playlist)->next, 0, sizeof(struct playlistent));
   (*playlist)->next->prev = *playlist;
   (*playlist)->next->number = (*playlist)->number +1;
   if ( filename[0] != '/' && strncasecmp(filename, "http://", 7) ) {
      getcwd(cwd, 256);
      sprintf((*playlist)->name, "%s/%s", cwd, filename);
   } else  
     strcpy((*playlist)->name, filename);
   
   if ( !showname ) 
     if ( scanid3 && getmp3info(filename, &mode, &samplerate, &bitrate, name, artist, NULL, NULL, NULL, 0) && artist[0]+name[0] ) {
	if ( artist[0] && name[0] ) sprintf(buf, "%s - %s", artist, name ); else
	  if ( artist[0] ) sprintf(buf, "%s - (unknown)", artist); else
	  sprintf(buf, "(unknown) - %s", name);
	strncpy((*playlist)->showname, buf, 100);
     } else 
     if ( !strncasecmp(filename, "http://", 7) ) {
	strncpy( (*playlist)->showname, filename, 100 );
	(*playlist)->showname[99] = 0;
     } else
     { /* no id3/empty id3 */
	strncpy(buf, filename, 256);
	lastdot=0; cspos=0;
	for(j=0;(j<strlen(buf) && cspos<99) ;j++) {
#ifdef NICE_NAMES
	   if ( buf[j] == '.' ) lastdot = cspos;
	   if ( buf[j] == '.' || buf[j] == '_' ) (*playlist)->showname[cspos] = 32; else
	     if ( buf[j] == '-' && j != 0 && j+1 < strlen(buf) && buf[j-1] != ' ' && buf[j+1] != ' ' ) {
		(*playlist)->showname[cspos]    = ' ';
		(*playlist)->showname[cspos+1]  = '-';
		(*playlist)->showname[cspos+2]  = ' ';
		cspos += 2;
	     } else
#endif
	     (*playlist)->showname[cspos] = buf[j]; /* tolower removed */

	   if ( buf[j] == '/' ) cspos = 0; else
	     cspos++;
	}
	if ( lastdot && cspos != 99) (*playlist)->showname[lastdot] = 0; /* rip off extension */
     }
   
   if ( bitrate ) {
      stat(filename, (struct stat*)&statf);	 
      (*playlist)->bitrate    = bitrate;
      (*playlist)->samplerate = samplerate;
      (*playlist)->mode       = mode;
      (*playlist)->length     = statf.st_size / (bitrate * 125);
   }
   
   free(buf);
   return;
}

void clearplaylist(struct playlistent **playlist) {
struct playlistent *temp;
   
   if ( *playlist != NULL ) {
      while ( (*playlist)->prev != NULL ) *playlist = (*playlist)->prev;
      while ( *playlist != NULL ) {
	 temp     = *playlist;
	 *playlist = (*playlist)->next;
	 free(temp);
      }
   } 
   return;
}


void sortplaylist(struct playlistent **playlist) {
struct playlistent *sortedlist, *temp;
int i,j;
unsigned int playlistents = pl_count(*playlist);
   
   if ( *playlist == NULL ) return;
   sortedlist = (struct playlistent*)malloc(sizeof(struct playlistent));
   sortedlist->prev = NULL;
   sortedlist->next = NULL;
   sortedlist->number = 0;
   
   for(i=0;i<playlistents;i++) {
      if ( config.mpg123 ) (void*)mpg123_control(NULL); 
      pl_seek(0, playlist);
      temp = *playlist;
      while ( (*playlist)->next != NULL && *playlist != NULL ) {
	 
	 for(j=0;j<strlen((*playlist)->showname);j++)
	   
	   if( tolower((*playlist)->showname[j]) < tolower(temp->showname[j]) ) {
	      temp = *playlist;
	      break;
	   } else
	   if ( tolower((*playlist)->showname[j]) != tolower(temp->showname[j]) )
	     break;	 
	 *playlist = (*playlist)->next;
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
   if ( *playlist ) free(*playlist);
   *playlist = sortedlist;
   return; 
}

   
void l_status(char *text) {
   if ( text != NULL )
     printf("\e[%d;%dH\e[%sm%s\e[%d;%dH%s", config.skin.texty, config.skin.textx, config.skin.textc, xys(config.skin.textw, ' '), config.skin.texty, config.skin.textx, text); else
     printf("\e[%d;%dH\e[%sm%s", config.skin.texty, config.skin.textx, config.skin.textc, xys(config.skin.textw, ' '));
   fflush(stdout);
}

#ifdef LIRCD
unsigned char pl_dolircd(struct playlistent **playlist, unsigned int *filenumber) {
char *ir, *c, ret;
   
   while ( !lirc_nextcode(&ir) && ir ) {
      while  ( !(ret=lirc_code2char(lircd_config,ir,&c)) && c )  {

      if ( !strcasecmp(c, "skip+") && pl_current != pl_maxpos ) {
	 if ( pl_screenmark != (config.skin.plistlines-1) ) /* move down */
	   pl_screenmark++;
	 pl_current++;
	 pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
      } 

      if ( !strcasecmp(c, "skip-") && pl_current != 0 ) {
	if ( pl_screenmark != 0 ) /* move up */
	   pl_screenmark--;
	 pl_current--;
	 pl_showents(pl_current-pl_screenmark, *playlist,filenumber);
      } 
      		
      if ( !strcasecmp(c, "play") )     pl_dofunction(playlist, filenumber, 1);
      if ( !strcasecmp(c, "stop") )     (void*)dofunction(3);
      if ( !strcasecmp(c, "playlist") ) return 1;
      if ( !strcasecmp(c, "pause") )    (void*)dofunction(8);
      if ( !strcasecmp(c, "mute") )     (void*)dofunction(14);
# ifdef HAVE_SYS_SOUNDCARD_H
      if ( !strcasecmp(c, "vol+") ) set_volume(config.voldev, config.volstep);
      if ( !strcasecmp(c, "vol-") ) set_volume(config.voldev, -config.volstep);
# endif
   }
   free(ir);
   if ( ret == -1 ) break;
   }
   return 0;
}
#endif
