#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include "camp.h"
#include "fileselector.c"
#include "fileselector7bit.c"

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

extern char playsong;
extern struct configstruct config;
//extern struct playlistent *playlist;

int fl_buttonpos=1;
unsigned int fl_maxpos=0, current[50];
char depth=0, screenmark[50], cdir[256];

struct playlistent *getfiles(struct playlistent *playlist) {
struct filelistent *filelist = NULL;
int i = 0;
int ch = 0;
fd_set fds;
   
   for(i=0;i<50;i++) { current[i] = 0; screenmark[i] = 0; }
   
   if ( cdir[0] == 0 && config.startincwd && getcwd(cdir, 256) ) {
      for(i=0;i<strlen(cdir);i++) if ( cdir[i] == '/' ) depth++;
      cdir[i]   = '/';
      cdir[i+1] = 0;
   }   
   if ( cdir[0] == 0 ) strcpy(cdir, "/"); 
   
   filelist = loaddir( cdir );
   if ( filelist == NULL ) {
      l_status("DIR Open error.");
      sleep(1);
      l_status(NULL);
      fl_updatebuttons(0);
      return NULL;
   }
   if ( config.ttymode == 8 ) 
     printf("%s", fsscreen); else
     printf("%s", fsscreen7bit);
   fl_showents(0, filelist);
   fl_updatebuttons(0);
   ch = 0;
   
   while ( TRUE ) {

      FD_ZERO(&fds);
      FD_SET(0, &fds);
#ifdef USE_GPM_MOUSE
      if ( gpm_flag ) FD_SET(gpm_fd, &fds);
      if ( select(gpm_flag ? gpm_fd+1 : 1, &fds, NULL, NULL, NULL) > 0 ) {
	 if ( FD_ISSET(gpm_fd, &fds) && gpm_flag ) {
	    filelist = fl_domouse(filelist, playlist);
	    if ( fl_buttonpos == FL_MAXBUTTON+1 ) { fl_buttonpos = FL_MAXBUTTON; return playlist; }
	 }
#else
      if ( select(1, &fds, NULL, NULL, NULL) > 0 ) {
#endif
	 ch = 0;
	 if ( FD_ISSET(0, &fds) )
	   while ( ch != -1 ) {
	      ch = getchar();
	      if ( ch == 3 ) exit(0);
	      if ( ch == 32 ) 
		switch(file_seek(current[depth], filelist)->type) {
		 case 1: 
		   togglemark(filelist, TRUE);
		   fl_showents(current[depth]-screenmark[depth], filelist);
		   break;
		 case 2:
		   filelist = camp_chdir(filelist);
		}
	      
	      if ( ch == 27 ) {
	         ch = getchar();
		 if ( ch == -1 ) {
		    mykbhit(0, 250000);
		    ch = getchar();
		 }
		 if ( ch != '[' ) {
		    releasedir(filelist);
		    return playlist;
		 }
		 ch = getchar();
		 if ( ch == 'B' && current[depth] != fl_maxpos ) {
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
		    if ( current[depth] > fl_maxpos-11 ) { screenmark[depth] = fl_maxpos - current[depth]; current[depth] = fl_maxpos; } else
		      current[depth] += 11;
		    fl_showents(current[depth]-screenmark[depth], filelist);
		 }
		 if ( ch == '1' ) {
		    current[depth] = 0;
		    screenmark[depth] = 0;
		    fl_showents(0, filelist);
		 }
		 if ( ch == '4' ) {
		    current[depth] = fl_maxpos;
		    screenmark[depth] = 12;
		    fl_showents(current[depth]-screenmark[depth], filelist);
		 }
	      }
	      if ( ch == 13 ) { 
		 if ( fl_buttonpos == FL_MAXBUTTON ) return playlist; else
		   if ( fl_buttonpos == 1 && file_seek(current[depth], filelist)->type == 2 )
		     filelist = camp_chdir(filelist); else {
			playlist = fl_dofunction(filelist, playlist);
		     }
	      }
	   }
      }
   }
}

struct filelistent *camp_chdir(struct filelistent *filelist) {
struct filelistent *newlist = NULL;
char   origpath[256];
int    origdepth = depth;
   
   strcpy(origpath, cdir);
   
   if ( !strcmp(file_seek(current[depth], filelist)->name, "..") ) {
      cdir[strrchr(cdir, '/')-cdir] = '\0';
      cdir[strrchr(cdir, '/')-cdir+1] = '\0';
      depth--;
   } else {
      sprintf(cdir, "%s%s/", cdir, file_seek(current[depth], filelist)->name);
      depth++;
      current[depth] = 0;
      screenmark[depth] = 0;
   }
   newlist = loaddir( cdir );
   if ( newlist == NULL ) {
      strcpy(cdir, origpath);
      depth = origdepth;
      l_status("DIR Open error!");
      fl_maxpos = file_seek(65535, filelist)->number;
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


void togglemark( struct filelistent *filelist, char movedown ) {

   filelist = file_seek(current[depth], filelist);
   
   if ( filelist->type != 1 ) return;
   if ( filelist->tagged == FALSE ) filelist->tagged = TRUE; else
     filelist->tagged = FALSE;

   if ( movedown && current[depth] != fl_maxpos ) current[depth]++;
   
}

struct filelistent *file_seek( int pos, struct filelistent *filelist ) {

   if ( filelist == NULL ) return NULL;
   
   if ( filelist->number < pos )
     while ( filelist->number != pos && filelist->next != NULL ) filelist = filelist->next;
   
   if ( filelist->number > pos ) 
     while ( filelist->number != pos && filelist->prev != NULL ) filelist = filelist->prev;

return filelist;
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


struct playlistent *fl_dofunction( struct filelistent *filelist, struct playlistent *playlist ) {
struct playlistent *temp;
char name[31], artist[31], buf[256];
char j, cspos;
   
   switch( fl_buttonpos ) {
      
    case 1: /* add */
      filelist = file_seek(0, filelist);
      while ( filelist->next != NULL ) {
	 sprintf(buf, "%s%s", cdir, filelist->name);
	 if ( filelist->tagged ) {
	    playlist = addfiletolist(playlist, buf, NULL, 0, 0, 0, TRUE);
	    filelist->tagged = FALSE;
	 }
	 filelist = filelist->next;
      }
      fl_showents(current[depth]-screenmark[depth], filelist);
      break;
      
    case 2: /* autotag */
      while ( filelist->prev != NULL ) filelist = filelist->prev;
      while ( filelist->next != NULL ) {
	 if ( (strstr(lowercases(filelist->name), ".mp3") != NULL) && filelist->type == 1 ) filelist->tagged = TRUE;
	 filelist = filelist->next;
      }
      fl_showents(current[depth]-screenmark[depth], filelist);
      break;
   
    case 3: /* untag all */
      while ( filelist->prev != NULL ) filelist = filelist->prev;
      while ( filelist->next != NULL ) {
	 filelist->tagged = FALSE;
	 filelist = filelist->next;
      }
      fl_showents(current[depth]-screenmark[depth], filelist);
      break;

    case 4: /* load playlist */
      sprintf(buf, "%s%s", cdir, file_seek(current[depth], filelist)->name);
      playlist = loadplaylist(playlist, buf, TRUE);
      break;

    case 5: /* save playlist */
      saveplaylist(playlist, file_seek(current[depth], filelist)->name);
      break;
      
    case 6: /* clear playlist */
      playlist = clearplaylist(playlist);
      break;
   } /* switch */
   return(playlist);
}


struct filelistent *loaddir( char *dir ) {
char buf[256];
DIR *dirptr;
struct dirent *direntptr;
struct stat statf;
struct filelistent *filelist = (struct filelistent*)malloc(sizeof(struct filelistent));

   
   fl_maxpos = 0;
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
	 fl_maxpos++;
      }
   }
   
   closedir(dirptr);
   dirptr = opendir(dir);  
   direntptr = readdir(dirptr);
   fl_maxpos--;
   return sortfilelist(filelist);
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
	 filelist = filelist->next;
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


struct playlistent *loadplaylist(struct playlistent *playlist, char *filename, char filemanager) {
FILE *fd;
int  ch=0, cspos, j, cplid3 = FALSE;
char buf2[256], buf[256], charen[500];
unsigned int samplerate;
unsigned int bitrate;
unsigned char mode;
struct oneplaylistent getpl;
   
   fd = fopen(filename, "r");
   if ( fd == NULL ) return playlist;
      
   if ( filemanager ) l_status("Loading, please wait...");
   readln(fd, buf);
   if (!strncmp(buf, "CPL+ID3 1.3", 11)) {
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
	if ( filemanager ) l_status("This playlist is too old, please check the README"); else {
	   printf("This playlist is too old, please check the README\n");
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


void saveplaylist(struct playlistent *playlist, char *filename) {
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
     fputs("CPL+ID3 1.3\n", fd);
   
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

#ifdef USE_GPM_MOUSE
struct filelistent *fl_domouse(struct filelistent *filelist, struct playlistent *playlist) {
Gpm_Event event;
int i;
   
   Gpm_GetEvent(&event);
   
   /* middle button, any kind of click */
   if ( event.buttons & 2 && event.type & GPM_DOWN ) {
      if ( config.ttymode == 8 )
	printf("%s", fsscreen); else
	printf("%s", fsscreen7bit);
      fl_showents(current[depth]-screenmark[depth], filelist);
      fl_updatebuttons(0);
      }
   
   /* right button, any kind of click */
/*   if ( event.buttons & 1 && event.type & GPM_DOWN ) {
      screenmark[depth] = -1;
      return filelist;
   } */
   
   /* left button - any kind of click */
   if ( event.buttons & 4 && event.type & GPM_DOWN ) 
     if ( event.y == 4 && event.x > 4 && event.x < 74 ) {
	/* page up */
	if ( current[depth] < 13 ) { current[depth] = 0; screenmark[depth] = 0; } else
	  current[depth] -= 11;
	if ( current[depth] < screenmark[depth] )
	  current[depth] = screenmark[depth];
	fl_showents(current[depth]-screenmark[depth], filelist);
     } else	
     if ( event.y == 18 && event.x > 4 && event.x < 74 ) {
	/* page down */
	if ( current[depth] > fl_maxpos-11 ) { screenmark[depth] = fl_maxpos - current[depth]; current[depth] = fl_maxpos; } else
	  current[depth] += 11;
	fl_showents(current[depth]-screenmark[depth], filelist);
     } else
     if ( event.y == 19 ) {
	i = 255;
	if ( event.x > 7 && event.x < 16 ) fl_buttonpos = 1; else
	  if ( event.x > 17 && event.x < 25 ) fl_buttonpos = 2; else
	  if ( event.x > 26 && event.x < 34 ) fl_buttonpos = 3; else
	  if ( event.x > 35 && event.x < 43 ) fl_buttonpos = 4; else
	  if ( event.x > 44 && event.x < 52 ) fl_buttonpos = 5; else
	  if ( event.x > 53 && event.x < 61 ) { playlist = clearplaylist(playlist); fl_buttonpos = 6; } else
	  if ( event.x > 62 && event.x < 70 ) fl_buttonpos = FL_MAXBUTTON+1; else
	  i = 0;
	if ( i == 255 ) { 
	   fl_updatebuttons(0);
	   playlist = fl_dofunction(filelist, playlist);
	}
     } 
     
     /* left button - any click/move */
     if ( event.buttons & 4 && \
        ( event.type & GPM_DRAG || event.type & GPM_DOWN ) )
       if ( event.y > 4 && event.y < 18 && event.x > 4 && event.x < 74 && \
          current[depth]-screenmark[depth]+event.y-5 <= fl_maxpos ) {
	  /* Toggle mark (true)*/
	  current[depth] = current[depth]-screenmark[depth] + event.y - 5;
	  screenmark[depth] = event.y - 5;
	  if (!file_seek(current[depth]-screenmark[depth]+event.y-5, filelist)->tagged) togglemark(filelist, FALSE);
	  fl_showents(current[depth]-screenmark[depth], filelist);
       } 

   /* right button - any click/move */
   if ( event.buttons & 1 && \
      ( event.type & GPM_DRAG || event.type & GPM_DOWN ) )
     if ( event.y > 4 && event.y < 18 && event.x > 4 && event.x < 74 && \
        current[depth]-screenmark[depth]+event.y-5 <= fl_maxpos ) {
	/* Toggle mark (false)*/
	current[depth] = current[depth]-screenmark[depth] + event.y - 5;
	screenmark[depth] = event.y - 5;
	if (file_seek(current[depth]-screenmark[depth]+event.y-5, filelist)->tagged) togglemark(filelist, FALSE);
	fl_showents(current[depth]-screenmark[depth], filelist);
     }
   
   if ( !(event.type & GPM_DRAG) && event.buttons & 4 && event.type & GPM_DOUBLE ) 
     if ( event.y > 4 && event.y < 18 && event.x > 4 && event.x < 74 && \
        current[depth]-screenmark[depth]+event.y-5 <= fl_maxpos ) {
	/* Select as current "file" */
	current[depth] = current[depth]-screenmark[depth] + event.y - 5;
	screenmark[depth] = event.y - 5;
	fl_showents(current[depth]-screenmark[depth], filelist);
	/* Change dir */
	if ( file_seek(current[depth], filelist)->type == 2 )
	  filelist = camp_chdir(filelist);
	fl_showents(current[depth]-screenmark[depth], filelist);
     }
   
   /* right button, single click */
   if ( event.buttons & 1 && event.type & GPM_DOWN )
     if ( event.y == 4 && event.x > 4 && event.x < 74 ) {
	/* home */
	current[depth] = 0;
	screenmark[depth] = 0;
	fl_showents(0, filelist);
     } else
     if ( event.y == 18 && event.x > 4 && event.x < 74 ) {
	/* end */
	current[depth] = fl_maxpos;
	screenmark[depth] = 12;
	fl_showents(current[depth]-screenmark[depth], filelist);
     } 
   Gpm_GetSnapshot(&event);     
   if ( !event.y ) event.y++;
   if ( !event.x ) event.y++;
   GPM_DRAWPOINTER(&event);
   return filelist;
}
#endif
