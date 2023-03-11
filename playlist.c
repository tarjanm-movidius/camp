#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#ifndef USE_TERMIOS
# include <curses.h>
#endif
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include "camp.h"
#include "fileselector.c"

extern char playsong;
extern char playlistname[19];
extern struct configstruct config;

int maxpos=0, pl_maxpos=0, current[50], pl_current=0, pl_screenmark=0;
int screenmark[50], fl_buttonpos=1, pl_buttonpos=1;
char depth = 0;

struct playlistent *rplaylist(struct playlistent *playlist, unsigned int *filenumber) {
int    ch;
fd_set stdinfds;

   printf("%s", fsscreen);
   if ( playlist != NULL ) {
      pl_maxpos = pl_seek(65000, playlist)->number-1;
      pl_current = *filenumber;
   } else 
     pl_current = 0;
   pl_screenmark = 0;
   pl_showents(pl_current-pl_screenmark, playlist); 
   pl_updatebuttons(0);
   
   while ( TRUE ) {
      
      FD_ZERO(&stdinfds);
      FD_SET(0, &stdinfds);
      select(1, &stdinfds, NULL, NULL, NULL);
      ch = getchar(); 
      if ( ch == 3 ) exit(0); /* ^C */ else
	if ( ch == 13 )	if ( pl_buttonpos == PL_MAXBUTTON ) return(playlist); else
	playlist = pl_dofunction(playlist, filenumber); else
	if ( ch == 27 ) {
	   ch = getchar();
	   if ( ch != '[' ) return playlist;
	   ch = getchar();
	   if ( ch == 'D' && pl_buttonpos != PL_MINBUTTON ) { 
	      pl_updatebuttons(-1);
	   }
	   if ( ch == 'C' && pl_buttonpos != PL_MAXBUTTON ) { 
	      pl_updatebuttons(1);
	 }
	   if ( ch == 'B' && pl_current != pl_maxpos ) {
	      if ( pl_screenmark != 12 )
		pl_screenmark++;
	      pl_current++;
	      pl_showents(pl_current-pl_screenmark, playlist);
	   }
	   if ( ch == 'A' && pl_current != 0 ) {
	      if ( pl_screenmark != 0 ) 
		pl_screenmark--;
	    pl_current--;
	      pl_showents(pl_current-pl_screenmark, playlist);
	   }
	   if ( ch == '5' ) {
	      if ( pl_current < 13 ) { pl_current = 0; pl_screenmark = 0; } else
		pl_current -= 11;
	      if ( pl_current < pl_screenmark )
		pl_current = pl_screenmark;
	    pl_showents(pl_current-pl_screenmark, playlist);
	   }
	   if ( ch == '6' ) {
	      if ( pl_current > pl_maxpos-11 ) { pl_screenmark = pl_maxpos - pl_current; pl_current = pl_maxpos; } else
		pl_current += 11;
	      pl_showents(pl_current-pl_screenmark, playlist);
	   }
	   if ( ch == '1' ) {
	      pl_current = 0;
	      pl_screenmark = 0;
	      pl_showents(0, playlist);
	   }
	   if ( ch == '4' ) {
	      pl_current = pl_maxpos;
	      pl_screenmark = 12;
	      pl_showents(pl_current-pl_screenmark, playlist);
	   }
	}
   }
}

struct playlistent *getfiles(struct playlistent *playlist) {
struct filelistent *filelist = NULL;
char   cdir[256];
int    i = 0;
int    ch = 0;     
fd_set stdinfds;
   
   for(i=0;i<50;i++) { current[i] = 0; screenmark[i] = 0; }
   strcpy(cdir, "/");
   filelist = loaddir(cdir);
   if ( filelist == NULL ) {
      l_status("DIR Open error.");
      sleep(1);
      l_status(NULL);
      fl_updatebuttons(0);
      return NULL;
   }
   printf("%s", fsscreen); 
   fl_showents(0, filelist);
   fl_updatebuttons(0);
   ch = 0;
   
   while ( TRUE ) {

      FD_ZERO(&stdinfds);
      FD_SET(0, &stdinfds);
      select(1, &stdinfds, NULL, NULL, NULL);
      ch = getchar();
      if ( ch == 3 ) exit(0);
      if ( ch == 32 ) 
	switch(file_seek(current[depth], filelist)->type) {
	 case 1: 
	   togglemark(filelist);
	   fl_showents(current[depth]-screenmark[depth], filelist);
	   break;
	 case 2:
	   filelist = camp_chdir(filelist, cdir);
	}
      
      if ( ch == 27 ) {
	 ch = getchar();
	 if ( ch != '[' ) {
	    releasedir(filelist);
	    return playlist;
	 }
	 ch = getchar();
	 if ( ch == 'B' && current[depth] != maxpos ) {
	    if ( screenmark[depth] != 12 )
	      screenmark[depth]++;
	    current[depth]++;
	    fl_showents(current[depth]-screenmark[depth], filelist);
	 }
	 if ( ch == 'A' && current[depth] != 0 ) {
	    if ( screenmark[depth] != 0 ) 
	      screenmark[depth]--;
	    current[depth]--;
	    fl_showents(current[depth]-screenmark[depth], filelist);
	 }
	 if ( ch == 'D' && fl_buttonpos != FL_MINBUTTON ) { 
	    fl_updatebuttons(-1);
	 }
	 if ( ch == 'C' && fl_buttonpos != FL_MAXBUTTON ) { 
	    fl_updatebuttons(1);
	 }
	 if ( ch == '5' ) {
	    if ( current[depth] < 13 ) { current[depth] = 0; screenmark[depth] = 0; } else
	      current[depth] -= 11;
	    if ( current[depth] < screenmark[depth] )
	      current[depth] = screenmark[depth];
	    fl_showents(current[depth]-screenmark[depth], filelist);
	 }
	 if ( ch == '6' ) {
	    if ( current[depth] > maxpos-11 ) { screenmark[depth] = maxpos - current[depth]; current[depth] = maxpos; } else
	      current[depth] += 11;
	    fl_showents(current[depth]-screenmark[depth], filelist);
	 }
	 if ( ch == '1' ) {
	    current[depth] = 0;
	    screenmark[depth] = 0;
	    fl_showents(0, filelist);
	 }
	 if ( ch == '4' ) {
	    current[depth] = maxpos;
	    screenmark[depth] = 12;
	    fl_showents(current[depth]-screenmark[depth], filelist);
	 }
      }
      if ( ch == 13 ) { 
	 if ( fl_buttonpos == FL_MAXBUTTON ) return playlist; else
	   if ( fl_buttonpos == 1 && file_seek(current[depth], filelist)->type == 2 )
	     filelist = camp_chdir(filelist, cdir); else {
		playlist = fl_dofunction(filelist, playlist, cdir);
		fl_showents(current[depth]-screenmark[depth], filelist);
	     }
      }
   }   
}

struct filelistent *camp_chdir(struct filelistent *filelist, char *cdir) {
struct filelistent *newlist = NULL;
char   origpath[256];
int    origdepth = depth;
   
   strcpy(origpath, cdir);
   
   if ( !strcmp(file_seek(current[depth], filelist)->name, "..") ) {
      cdir[strrchr(cdir, '/')-cdir] = '\0';
      depth--;
   } else {
      sprintf(cdir, "%s/%s", cdir, file_seek(current[depth], filelist)->name);
      depth++;
      current[depth] = 0;
      screenmark[depth] = 0;
   }
   newlist = loaddir(cdir);
   if ( newlist == NULL ) {
      strcpy(cdir, origpath);
      depth = origdepth;
      l_status("DIR Open error!");
      maxpos = file_seek(65535, filelist)->number;
      sleep(1);
      l_status(NULL);
      fl_updatebuttons(0);
      return filelist;
   } 
   if ( filelist != NULL ) releasedir(filelist);
   fl_showents(current[depth]-screenmark[depth], newlist);
   return newlist;
}

void fl_showents( int startpos, struct filelistent *filelist ) {
int i=0, k=0;
char shortname[66];
   
   filelist = file_seek(startpos, filelist);
   
   while ( (filelist->next != NULL) ) {
      if ( filelist->number == current[depth] ) printf("\e[0;31;46m"); else
      if ( filelist->type == 2 ) printf("\e[1;36;46m"); else
	printf("\e[0;34;46m");
      if ( filelist->tagged ) printf("\e[1m");

      for (k=0;k<65;k++) if ( k < strlen(filelist->name) ) shortname[k] = filelist->name[k]; else
	shortname[k] = ' ';
      shortname[65] = '\0';
      printf("\e[%d;8H%s",5+i,shortname);
      i++;
      if (i>12) break;
      filelist = filelist->next;
   }
   if ( i != 13 ) 
      for(;i<13;i++) 
      printf("\e[%d;8H                                                                 ",5+i);
   fflush(stdout);
}

void pl_showents( int startpos, struct playlistent *playlist ) {
int i=0, k=0;
char shortname[61];
   
   playlist = pl_seek(startpos, playlist);
   printf("\e[0;46m");
   
   while ( playlist != NULL && playlist->next != NULL ) {
      if ( playlist->number == pl_current ) printf("\e[31m"); else
	printf("\e[34m");
      strncpy(shortname, playlist->showname, 60);
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

void togglemark( struct filelistent *filelist ) {

   filelist = file_seek(current[depth], filelist);
   
   if ( filelist->tagged == FALSE ) filelist->tagged = TRUE; else
     filelist->tagged = FALSE;

   if ( current[depth] != maxpos ) current[depth]++;
   
}

struct filelistent *file_seek( int pos, struct filelistent *filelist ) {

   if ( filelist == NULL ) return NULL;
   
   if ( filelist->number < pos )
     while ( filelist->number != pos && filelist->next != NULL ) filelist = filelist->next;
   
   if ( filelist->number > pos ) 
     while ( filelist->number != pos && filelist->prev != NULL ) filelist = filelist->prev;

return filelist;
}

struct playlistent *pl_seek( unsigned int pos, struct playlistent *playlist ) {

   if ( playlist == NULL ) return NULL;
   
   if ( playlist->number < pos )
     while ( (playlist->number != pos) && (playlist->next != NULL) ) playlist = playlist->next;
   
   if ( playlist->number > pos ) 
     while ( (playlist->number != pos) && (playlist->prev != NULL) ) playlist = playlist->prev;

return playlist;
}

int pl_count( struct playlistent *playlist ) {
int count=0;
   if ( playlist==NULL ) return 0;
   playlist = pl_seek(0, playlist);
   while( playlist->next != NULL ) {
      count++;
      playlist=playlist->next;
   }
   return count;
}



void fl_updatebuttons(int add) {

   fl_buttonpos = fl_buttonpos + add;
   printf("\e[19;9H");
   if (fl_buttonpos == 1) printf("\e[44;36;1m"); else
     printf("\e[34;44;1m");
   printf("  add  ");
   printf("\e[19;18H");
   if (fl_buttonpos == 2) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf("  tag  ");   
   printf("\e[19;27H");
   if (fl_buttonpos == 3) printf("\e[44;36m"); else
     printf("\e[34;44m");
   printf(" untag ");
   printf("\e[19;36H");
   if (fl_buttonpos == 4) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" loadp ");
   printf("\e[19;45H");
   if (fl_buttonpos == 5) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" savep ");
   printf("\e[19;54H");
   if (fl_buttonpos == 6) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" clear ");
   printf("\e[19;63H");
   if (fl_buttonpos == 7) printf("\e[44;36m"); else
     printf("\e[44;34m");
   printf(" back! ");   
   fflush(stdout);
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



struct playlistent *fl_dofunction( struct filelistent *filelist, struct playlistent *playlist, char *cdir ) {
struct playlistent *temp;
char name[31], artist[31], buf[256];
char j, cspos;
   
   switch( fl_buttonpos ) {
      
    case 1: /* add */
      filelist = file_seek(0, filelist);
      while ( filelist->next != NULL ) {
	 sprintf(buf, "%s/%s", cdir, filelist->name);
	 if ( filelist->tagged ) {
	    playlist = addfiletolist(playlist, buf, NULL, 0, 0, 0, TRUE);
	    filelist->tagged = FALSE;
	 }
	 filelist = filelist->next;
      }
      strcpy(playlistname, "misc. files loaded");
      break;
      
    case 2: /* autotag */
      while ( filelist->prev != NULL ) filelist = filelist->prev;
      while ( filelist->next != NULL ) {
	 if ( (strstr(lowercases(filelist->name), ".mp3") != NULL) && filelist->type == 1 ) filelist->tagged = TRUE;
	 filelist = filelist->next;
      }
      break;
   
    case 3: /* untag all */
      while ( filelist->prev != NULL ) filelist = filelist->prev;
      while ( filelist->next != NULL ) {
	 filelist->tagged = FALSE;
	 filelist = filelist->next;
      }
      break;

    case 4: /* load playlist */
      sprintf(buf, "%s/%s", cdir, file_seek(current[depth], filelist)->name);
      playlist = loadplaylist(playlist, buf, TRUE);
      break;

    case 5: /* save playlist */
      saveplaylist(playlist, cdir, file_seek(current[depth], filelist)->name);
      break;
      
    case 6: /* clear playlist */
      clearplaylist(playlist);
      strcpy(playlistname, "no playlist loaded");
      break;
   } /* switch */
   return(playlist);
}


struct playlistent *pl_dofunction(struct playlistent *playlist, unsigned int *filenumber) {
struct playlistent *temp;
int pid;   
   
   switch( pl_buttonpos ) {
      
    case 1: /* browse */
      playlist = getfiles(playlist);
      printf("%s", fsscreen);
      if ( playlist == NULL ) pl_maxpos = 0; else
	pl_maxpos = pl_seek(65000, playlist)->number-1;
      if (pl_current - pl_screenmark > pl_maxpos)
	pl_current = pl_screenmark = 0;
      pl_showents(pl_current, playlist); 
      pl_updatebuttons(0);
      return( playlist );
      
    case 2: /* play */ 
      if ( playlist == NULL ) return NULL;
      if ( playsong ) killslave();
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
      strcpy(playlistname, "misc. files loaded");
      return(playlist);
      
    case 4: /* new list (clear) */
      clearplaylist(playlist);
      pl_showents(0, playlist);
      pl_current = 0;
      pl_screenmark = 0;
      strcpy(playlistname, "no playlist loaded");
      return(playlist);

    case 5: /* id3 editor */
      id3edit(pl_seek(pl_current, playlist)->name, pl_seek(pl_current, playlist));
      printf("%s", fsscreen);
      pl_showents(pl_current-pl_screenmark, playlist); 
      pl_updatebuttons(0);
      return playlist;
      
    case 6: /* sort playlist */
      playlist = sortplaylist(playlist);
      pl_showents(pl_current-pl_screenmark, playlist);       
      return playlist;
   }
}

struct filelistent *loaddir(char *dir) {
char buf[256];
struct DIR *dirptr;
struct dirent *direntptr;
struct stat statf;
struct filelistent *filelist = (struct filelistent*)malloc(sizeof(struct filelistent));

   maxpos = 0;
   filelist->next = NULL;
   filelist->prev = NULL;
   filelist->number = 0;
   dirptr = opendir(dir);
   if ( dirptr == NULL ) return NULL;
   
   while ( ( direntptr = readdir(dirptr) ) != NULL ) {
      if (strcmp(direntptr->d_name, ".")) {
	 if ( !strcmp(direntptr->d_name, "..") && !strcmp(dir, "/") ) 
	   continue; else
	   if ( config.hidedot && direntptr->d_name[0] == '.' && strcmp(direntptr->d_name, "..") )
	     continue; 
	 sprintf(buf, "%s/%s", dir, direntptr->d_name);
	 stat(buf, (struct stat*)&statf);
	 if ( (statf.st_mode & S_IFMT) == S_IFDIR ) 
	   filelist->type = 2; else
	   filelist->type = 1;
	 strcpy(filelist->name, direntptr->d_name);
	 filelist->next         = (struct filelistent*)malloc(sizeof(struct filelistent));
	 filelist->next->prev   = filelist;
	 filelist->next->next   = NULL;
	 filelist->next->number = filelist->number +1;
	 filelist->tagged       = FALSE;
	 filelist               = filelist->next;
	 maxpos++;
      }
   }
   
   closedir(dirptr);
   dirptr = opendir(dir);  
   direntptr = readdir(dirptr);
   maxpos--;
   return sortfilelist(filelist);
}


char *lowercases(char *str) {
static char buf[256];
int i=0;
   for(;i<strlen(str)+1;i++) buf[i] = tolower(str[i]);
   return buf;
}

void releasedir(struct filelistent *filelist) {
struct filelistent *temp;

   while ( filelist->prev != NULL ) filelist = filelist->prev;
   while ( filelist != NULL ) {
      temp     = filelist;
      filelist = filelist->next;
      free(temp);
   }

}

struct playlistent *loadplaylist(struct playlistent *playlist, char *filename, char filemanager) {
FILE *fd;
int  ch=0, cspos, j, cplid3 = FALSE;
char buf2[256], buf[256], charen[500];
unsigned int samplerate;
int bitrate;
unsigned char mode;
struct oneplaylistent getpl;
   
   fd = fopen(filename, "r");
   if ( fd == NULL ) return playlist;

   if ( playlist != NULL ) strcpy(playlistname, "misc. files loaded"); else {
      strcpy(buf, filename);
      for(ch=0;ch<strlen(filename);ch++)
	if ( filename[ch] == '/' ) cspos=0; else {
	   buf[cspos] = filename[ch];
	   cspos++;
	}
      buf[cspos] = 0;
      strncpy(playlistname, buf, 19);
   }
   
   if ( filemanager ) l_status("Loading, please wait...");
   readln(fd, buf);
   if (!strncmp(buf, "CPL+ID3 1.2", 11)) {
      if ( playlist == NULL ) {
	 playlist = (struct playlistent*)malloc(sizeof(struct playlistent));
	 playlist->prev = NULL;
	 playlist->next = NULL;
	 playlist->number = 0;
      } else
	while ( playlist->next != NULL ) playlist = playlist->next;
      
      while ( fread((void*)&getpl, sizeof(struct oneplaylistent), 1, fd) != 0 ) {
	 memcpy((void*)playlist, (void*)&getpl, sizeof(struct oneplaylistent));
	 playlist->next = (struct playlistent*)malloc(sizeof(struct playlistent));
	 playlist->next->prev = playlist;
	 playlist->next->next = NULL;
	 playlist->next->number = playlist->number +1;
	 playlist = playlist->next;
      }
   } else 
     if (!strncmp(buf, "CPL+ID3", 7)) { /* wrong version playlist ;( */
	if ( filemanager ) l_status("This playlist is from an unsupported version of CAMP"); else {
	   printf("This playlist is from an unsupported version of CAMP\n");
	   exit(-1);
	}
	sleep(2);
	l_status(NULL);
     } else
     do { /* while readln != EOF */
	playlist = addfiletolist(playlist, buf, NULL, 0, 0, 0, TRUE);
     } while ( readln(fd, buf) != EOF );
   fclose(fd);
   if ( filemanager ) { 
      l_status(NULL);
      fl_updatebuttons(0);
   }
   return playlist;
}

void saveplaylist(struct playlistent *playlist, char *cdir, char *filename) {
FILE   *fd;
fd_set stdinfds;
char   cplid3 = TRUE, ch, buf[256], buf2[60];
int    exitchar = 0;
struct oneplaylistent getpl;
   
   memset((void*)&getpl, 0, sizeof(struct oneplaylistent));
   if ( playlist == NULL ) return;
   while ( playlist->prev != NULL ) playlist = playlist->prev;
   
   l_status("\e[36;46;1m\e[?25hSave as:\e[0;34;46m");
   strcpy(buf2, readyxline(19, 17, filename, 56, &exitchar, (int*)&ch));
   if ( exitchar == 27 ) {
      l_status("\e[?25l");
      fl_updatebuttons(0);
      return;
   } 
   
   if ( strchr(buf2, '/') == NULL )
     sprintf(buf, "%s/%s", cdir, buf2); else
     strcpy(buf, buf2);
     
   
   l_status("Save ID3 tags with playlist, for quicker loading? (Y/n)");
   FD_ZERO(&stdinfds);
   FD_SET(0, &stdinfds);
   select(1, &stdinfds, NULL, NULL, NULL);
   fd = fopen(buf, "w");
   if ( toupper(getchar()) == 'N' ) cplid3 = FALSE; else
     fputs("CPL+ID3 1.2\n", fd);
   
   l_status("Saving...\e[?25l");

   if ( cplid3 ) 
     do {
	memcpy((void*)&getpl, (void*)playlist, sizeof(struct oneplaylistent));
	fwrite((void*)&getpl, sizeof(struct oneplaylistent), 1, fd);
	if ( playlist->next != NULL ) playlist = playlist->next;
     } while (playlist->next != NULL);
   else 
     do {
	fputs(playlist->name, fd);
	fputc('\n', fd);
	if ( playlist->next != NULL ) playlist = playlist->next;
     } while ( playlist->next != NULL );
   fclose(fd);
   l_status(NULL);
   fl_updatebuttons(0);
}

struct playlistent *addfiletolist(struct playlistent *playlist, char *filename, char *showname, int bitrate, unsigned int samplerate, unsigned char mode, char scanid3 ) {
char name[31], artist[31];
int cspos=0, j;
   
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
   strcpy(playlist->name, filename);
   
   if ( showname == NULL ) {
      if ( scanid3 && getmp3info(filename, &mode, &samplerate, &bitrate, name, artist, NULL, NULL, NULL, 0) ) {
	 if ( artist[0] != 0 ) sprintf(playlist->showname, "%s - %s", artist, name); else
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
   if ( bitrate+samplerate != 0 ) {
      playlist->bitrate = bitrate;
      playlist->samplerate = samplerate;
      playlist->mode = mode;
   }
   
return playlist;
}


void clearplaylist(struct playlistent *playlist) {
struct playlistent *temp;
   
   if ( playlist != NULL ) {
      while ( playlist->prev != NULL ) playlist = playlist->prev;
      while ( playlist != NULL ) {
	 temp     = playlist;
	 playlist = playlist->next;
	 free(temp);
      }
   } 
}

struct playlistent *sortplaylist(struct playlistent *playlist) {
struct playlistent *sortedlist, *temp;
int i,j;
unsigned int playlistents = pl_seek(65535, playlist)->number;
   
   sortedlist = (struct playlistent*)malloc(sizeof(struct playlistent));
   sortedlist->prev = NULL;
   sortedlist->next = NULL;
   sortedlist->number = 0;
   
   for(i=0;i<playlistents;i++) {
      playlist = pl_seek(0, playlist);
      temp = playlist;
      while ( playlist->next != NULL ) {
	 playlist = playlist->next;
	 for(j=0;j<strlen(playlist->showname);j++)
	   
	   if( tolower(playlist->showname[j]) < tolower(temp->showname[j]) ) {
	      temp = playlist;
	      break;
	   } else
	   if ( tolower(playlist->showname[j]) != tolower(temp->showname[j]) )
	     break;
	 
      }
      sortedlist->next = (struct playlistent*)malloc(sizeof(struct playlistent));
      sortedlist->next->prev = sortedlist;
      sortedlist->next->next = NULL;
      sortedlist->next->number = sortedlist->number+1;
      strcpy(sortedlist->showname, temp->showname);
      strcpy(sortedlist->name, temp->name);
      sortedlist->samplerate = temp->samplerate;
      sortedlist->bitrate = temp->bitrate;
      sortedlist->mode = temp->mode;
      sortedlist = sortedlist->next;
      
      if ( temp->prev != NULL ) temp->prev->next = temp->next; else
	temp->next->prev = NULL;
      if ( temp->next != NULL ) temp->next->prev = temp->prev; else
	temp->prev->next = NULL;
      free(temp);
   }
   return(sortedlist);
}

struct filelistent *sortfilelist(struct filelistent *filelist) {
struct filelistent *sortedlist, *temp;
int i,j;
unsigned int filelistents = file_seek(65535, filelist)->number;
   
   sortedlist = (struct filelistent*)malloc(sizeof(struct filelistent));
   sortedlist->prev = NULL;
   sortedlist->next = NULL;
   sortedlist->number = 0;
   
   for(i=0;i<filelistents;i++) {
      filelist = file_seek(0, filelist);
      temp = filelist;
      while ( filelist->next != NULL ) {
	 filelist = filelist->next;
	 for(j=0;j<strlen(filelist->name);j++)
	   if( filelist->type == 2 && temp->type == 1 ) {
	      temp = filelist;
	      break;
	   } else
	   if ( filelist->type == 1 && temp->type == 2 )
	     break; else
	   if( filelist->name[j] < temp->name[j] ) {
	      temp = filelist;
	      break;
	   } else
	   if ( filelist->name[j] != temp->name[j] )
	     break;	 
      }
      sortedlist->next = (struct filelistent*)malloc(sizeof(struct filelistent));
      sortedlist->next->prev = sortedlist;
      sortedlist->next->next = NULL;
      sortedlist->next->number = sortedlist->number+1;
      strcpy(sortedlist->name, temp->name);
      sortedlist->tagged = temp->tagged;
      sortedlist->type = temp->type;
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
