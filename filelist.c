#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "camp.h"

#ifdef HAVE_TERMIOS_H
# include <fcntl.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

#ifdef HAVE_LIBZ
# include <zlib.h>
#endif

extern char playsong, checkkill, use_lircd, currloc;
extern struct configstruct config;
extern unsigned int slavepid;
extern int lirc_lircd, selval;

int fl_buttonpos;
unsigned int fl_maxpos=0, current[50];
char depth=0, screenmark[50], cdir[256];


void getfiles(struct playlistent **playlist) {
struct filelistent *filelist = NULL;
struct timeval counter;
int i = 0;
int ch, ch2;
fd_set fds;
   
   currloc = CAMP_FL; /* Tell the world where we are */
   fl_buttonpos = config.skin.fsb;

   /* for(i=0;i<50;i++) { current[i] = 0; screenmark[i] = 0; } 
    * Eh.. was that lame or what? */
   
   memset(current, 0, sizeof(current));
   memset(screenmark, 0, sizeof(screenmark));
   
   if ( cdir[0] == 0 && config.startincwd && getcwd(cdir, 256) ) {
      for(i=0;i<strlen(cdir);i++) if ( cdir[i] == '/' ) depth++;
      cdir[i]   = '/';
      cdir[i+1] = 0;
   }   
   if ( cdir[0] == 0 ) strcpy(cdir, "/"); 
   
   filelist = loaddir( cdir );
   if ( filelist == NULL ) {
      l_status(config.skin.flistmsg[1]);
      sleep(1);
      l_status(NULL);
      fl_updatebuttons(0);
      return;
   }
   if ( config.skin.fclr ) printf("\e[0m\e[2J");
   printf("\e[0m\e[1;1H%s", config.skin.filelist);
   fl_showents(0, filelist);
   fl_updatebuttons(0);
   
   while ( TRUE ) {
      
      if ( config.mpg123 )
	mpg123_control(NULL);
      
      if ( checkkill && kill(slavepid, 0) == -1 ) {
         checkkill = FALSE;
         playnext(-1);
      }
      
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
      
      if ( select(selval, &fds, NULL, NULL, &counter) > 0 ) {
#ifdef USE_GPM_MOUSE
	 if ( gpm_flag > 0 && FD_ISSET(gpm_fd, &fds) ) {	    
	    if ( fl_domouse(&filelist, playlist) ) {
	       releasedir(filelist);
	       return;
	    }
	 }
#endif
#ifdef LIRCD
	 if ( use_lircd && FD_ISSET(lirc_lircd, &fds) ) dolircd(0);
#endif
	 ch = 0;
	 if ( FD_ISSET(0, &fds) ) {
	    ch = getchar();
	    if ( ch == 3 && canexit() ) { releasedir(filelist); exit(0); } else
	      if ( ch == 26 && canexit() ) { releasedir(filelist); disappear(); } else /* ^Z */
              if ( ch == 12 ) { /* ^L */
		 printf("\e[0m\e[2J\e[1;1H%s", config.skin.filelist);
		 fl_showents(current[depth]-screenmark[depth], filelist);
		 fl_updatebuttons(0);
	      } else	      
	      if ( ch == 32 ) 
		switch(file_seek(current[depth], filelist)->type) {
		 case 1: 
		   togglemark(filelist, TRUE);
		     fl_showents(current[depth]-screenmark[depth], filelist);
		   break;
		 case 2:
		   filelist = camp_chdir(filelist);
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
		   if ( ch == -1 || ch == 27 ) { 
		      releasedir(filelist);		      
		      return;
		   }
		   
		   if ( ch2 == 'B' && current[depth] != fl_maxpos ) {
		      if ( screenmark[depth] != config.skin.flistlines-1 )
			screenmark[depth]++;
		      current[depth]++;
		      fl_showents(current[depth]-screenmark[depth], filelist);
		   }
		   if ( ch2 == 'A' && current[depth] != 0 ) {
		      if ( screenmark[depth] != 0 ) 
			screenmark[depth]--;
		      current[depth]--;
		      fl_showents(current[depth]-screenmark[depth], filelist);
		   }
		   if ( ch2 == 'D' && fl_buttonpos != FL_MINBUTTON ) { 
		      fl_updatebuttons(-1);
		   }
		   if ( ch2 == 'C' && fl_buttonpos != FL_MAXBUTTON) { 
		      fl_updatebuttons(1);
		   }
		   if ( ch2 == '5' ) { /* page up */
		      if ( current[depth] < config.skin.flistlines ) { 
			 current[depth] = 0; 
			 screenmark[depth] = 0; 
		      } else
			current[depth] -= (config.skin.flistlines-1);
		      if ( current[depth] < screenmark[depth] )
			current[depth] = screenmark[depth];
		      fl_showents(current[depth]-screenmark[depth], filelist);
		   }
		   if ( ch2 == '6' ) { /* page down */
		      if ( current[depth]+(config.skin.flistlines-1) > fl_maxpos ) {
			 screenmark[depth] = 0;
			 current[depth] = fl_maxpos; 
		      } else
			current[depth] += (config.skin.flistlines-1);
		      fl_showents(current[depth]-screenmark[depth], filelist);
		   }
		   if ( ch2 == '1' ) { /* home */
		      current[depth] = 0;
		      screenmark[depth] = 0;
		      fl_showents(0, filelist);
		   }
		   if ( ch2 == '4' ) { /* end */
		      current[depth] = fl_maxpos;
		      screenmark[depth] = 0;
		      fl_showents(current[depth]-screenmark[depth], filelist);
		   }
		} else
	      if ( ch == 13 ) { 

//             if ( fl_buttonpos == config.skin.flistbo[FL_MAXBUTTON] ) { releasedir(filelist); return; } else
                if ( config.skin.flistbo[fl_buttonpos] == FL_BUTTONQUIT ) { releasedir(filelist); return; } else
  
		   if ( config.skin.flistbo[fl_buttonpos] == FL_MINBUTTON && file_seek(current[depth], filelist)->type == 2 )
		     filelist = camp_chdir(filelist); else {
			fl_dofunction(filelist, playlist);
		     }
	      } else
	      fl_search(ch, filelist);	      
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
      l_status(config.skin.flistmsg[1]);
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
char *shortname;
   
   shortname = malloc(config.skin.flistw+1);
   filelist = file_seek(startpos, filelist);
   
   while ( (filelist->next != NULL) ) {
      if ( filelist->number == current[depth] ) /* active */
	if ( filelist->type == 2 ) printf("\e[%sm", config.skin.flistcda); else
	if ( !filelist->tagged ) printf("\e[%sm", config.skin.flistcfa); else
	printf("\e[%sm", config.skin.flistcta); else /* inactive */
	if ( filelist->type == 2 ) printf("\e[%sm", config.skin.flistcdi); else
	if ( !filelist->tagged ) printf("\e[%sm", config.skin.flistcfi); else
	printf("\e[%sm", config.skin.flistcti);
      
      strncpy(shortname, filelist->name, config.skin.flistw);
      for (k=strlen(shortname);k<config.skin.flistw;k++) shortname[k] = ' ';
      shortname[config.skin.flistw] = '\0';
      
      printf("\e[%d;%dH%s",i+config.skin.flisty, config.skin.flistx, shortname);
      i++;
      if (i>=config.skin.flistlines) break;
      filelist = filelist->next;
   }
   printf("\e[%sm", config.skin.flistcfi);
   if ( i != config.skin.flistlines ) 
     for(;i<config.skin.flistlines;i++) 
       printf("\e[%d;%dH%s",i+config.skin.flisty, config.skin.flistx, xys(config.skin.flistw, ' '));
   fflush(stdout);
   free(shortname);
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
int i;
   if ( config.skin.flistbo[fl_buttonpos+add] == -1 ) return;
   
   fl_buttonpos = fl_buttonpos + add;
   for (i=FL_MINBUTTON;i<(FL_MAXBUTTON+1);i++) {
      if ( !config.skin.fy[i] && !config.skin.fx[i] ) continue;
      printf("\e[%d;%dH",config.skin.fy[i], config.skin.fx[i]);
      if (config.skin.flistbo[fl_buttonpos] == i) printf("%s", config.skin.fa[i]); else
	printf("%s", config.skin.fi[i]);
   }
   fflush(stdout);
}


void fl_dofunction( struct filelistent *filelist, struct playlistent **playlist ) {
struct playlistent *temp;
char name[31], artist[31], buf[256];
char j, cspos;
   
   switch( config.skin.flistbo[fl_buttonpos] ) {
      
    case 0: /* add */

      filelist = file_seek(0, filelist);
      while ( filelist->next != NULL ) {
	 sprintf(buf, "%s%s", cdir, filelist->name);
	 if ( filelist->tagged ) {
	    addfiletolist(playlist, buf, NULL, 0, 0, 0, config.useid3);
	    filelist->tagged = FALSE;
	 }
	 filelist = filelist->next;
      }
      fl_showents(current[depth]-screenmark[depth], filelist);
      break;
      
    case 1: /* autotag */
      while ( filelist->prev != NULL ) filelist = filelist->prev;
      while ( filelist->next != NULL ) {
	 if ( (strstr(lowercases(filelist->name), ".mp3") != NULL) && filelist->type == 1 ) filelist->tagged = TRUE;
	 filelist = filelist->next;
      }
      fl_showents(current[depth]-screenmark[depth], filelist);
      break;
   
    case 2: /* untag all */
      while ( filelist->prev != NULL ) filelist = filelist->prev;
      while ( filelist->next != NULL ) {
	 filelist->tagged = FALSE;
	 filelist = filelist->next;
      }
      fl_showents(current[depth]-screenmark[depth], filelist);
      break;

    case 3: /* load playlist */
      sprintf(buf, "%s%s", cdir, file_seek(current[depth], filelist)->name);
      loadplaylist(playlist, buf, TRUE);
      break;

    case 4: /* save playlist */
      saveplaylist(*playlist, file_seek(current[depth], filelist)->name, FALSE);
      break;
      
    case 5: /* clear playlist */
      clearplaylist(playlist);
      break;
/*
    case 6:  Quit 
      fl_buttonpos = FL_MAXBUTTON+1;
      break;
*/
    case 7: /* recurse playlist */
      j = fl_maxpos;
      recurseplaylist(filelist, playlist, cdir);
      fl_maxpos = j;
      break;

   } /* switch */
   return;
}


struct filelistent *loaddir( char *dir ) {
char *buf;
DIR *dirptr;
struct dirent *direntptr;
struct stat statf;
struct filelistent *filelist = (struct filelistent*)malloc(sizeof(struct filelistent));
   
   fl_maxpos = 0;
   filelist->next = NULL;
   filelist->prev = NULL;
   filelist->number = 0;
   dirptr = opendir(dir);
   if ( !dirptr ) {
      free(filelist);
      return NULL;
   }

   buf = (char*)malloc(500);
   
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
   free(buf);
   fl_maxpos--;
   return sortfilelist(filelist);
}


void releasedir(struct filelistent *filelist) {
struct filelistent *temp;

   if (filelist != NULL) {
	 	while ( filelist->prev != NULL ) filelist = filelist->prev;
		while ( filelist != NULL ) {
		      temp     = filelist;
		      filelist = filelist->next;
		      free(temp);
		}
   } 
}


struct filelistent *sortfilelist(struct filelistent *filelist) {
struct filelistent *sortedlist, *temp;
int i,j;
unsigned int filelistents = file_seek(65535, filelist)->number;
   
   if ( !filelist ) return NULL;
   
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
   if (filelist) free(filelist);
   return(sortedlist);
}


void loadplaylist(struct playlistent **playlist, char *filename, char filemanager) {
FILE *fd;
int  ch=0, cspos, j, cplid3 = FALSE;
char *buf;
unsigned int samplerate;
unsigned int bitrate;
unsigned char mode;
struct oneplaylistent getpl;

#ifdef HAVE_LIBZ
gzFile gzfd;
   gzfd = gzopen(filename, "rb");
   if ( !gzfd ) return;
   buf = malloc(257);
   gzread(gzfd, buf, 12);
#else
   fd = fopen(filename, "rb");
   if ( !fd ) return;   
   buf = malloc(257);
   fgets((char*)buf, 255, fd);
#endif
   
   if ( filemanager ) l_status(config.skin.flistmsg[0]);
   
   if (!strncmp(buf, "CPL+ID3 1.3\n", 12)) {
      if ( *playlist == NULL ) {
	 *playlist = (struct playlistent*)malloc(sizeof(struct playlistent));
	 (*playlist)->prev = NULL;
	 (*playlist)->next = NULL;
	 (*playlist)->number = 0;
      } else
	while ( (*playlist)->next != NULL ) *playlist = (*playlist)->next;
      
#ifdef HAVE_LIBZ
      while ( gzread(gzfd, (void*)&getpl, sizeof(struct oneplaylistent)) > 0 ) {
	 memcpy((void*)*playlist, (void*)&getpl, sizeof(struct oneplaylistent));
	 (*playlist)->next = (struct playlistent*)malloc(sizeof(struct playlistent));
	 (*playlist)->next->prev = *playlist;
	 (*playlist)->next->next = NULL;
	 (*playlist)->next->number = (*playlist)->number +1;
	 *playlist = (*playlist)->next;
      }
#else
      while ( fread((void*)&getpl, sizeof(struct oneplaylistent), 1, fd) != 0 ) {
	 memcpy((void*)*playlist, (void*)&getpl, sizeof(struct oneplaylistent));
	 (*playlist)->next = (struct playlistent*)malloc(sizeof(struct playlistent));
	 (*playlist)->next->prev = *playlist;
	 (*playlist)->next->next = NULL;
	 (*playlist)->next->number = (*playlist)->number +1;
	 *playlist = (*playlist)->next;
      }      
#endif
   } else /* not a CPL+ID3 1.3 playlist */
     if (!strncmp(buf, "CPL+ID3", 7)) { /* wrong version playlist ;( */
	if ( filemanager ) {
	   l_status(config.skin.flistmsg[2]);
	   sleep(2);
	   l_status(NULL); 
	} else {
	   printf("%s\n", config.skin.flistmsg[2]);
	   exit(-1);
	}
     } else /* text list w/ filenames? */
#ifdef HAVE_LIBZ
     {
	gzclose(gzfd);
	gzfd = 0;
	fd = fopen(filename, "rb");
#endif
	do { /* while readln != EOF */
	   buf = strtrim(buf, '\n'); 
	   addfiletolist(playlist, buf, NULL, 0, 0, 0, config.useid3);
	} while ( fgets((char*)buf, 255, fd) );	
#ifdef HAVE_LIBZ
	fclose(fd);
     }
   if ( gzfd ) gzclose(gzfd);
#else
   fclose(fd);
#endif
   
   if ( filemanager ) { 
      l_status(NULL);
      fl_updatebuttons(0);
   } else
     printf("done!\n");
   free(buf);
   return;
}


void saveplaylist(struct playlistent *playlist, char *filename, char directsave) {
FILE   *fd;
fd_set stdinfds;
char   cplid3 = TRUE, ch, *buf, *buf2;
int    exitchar = 0;
struct oneplaylistent getpl;
#ifdef HAVE_LIBZ
gzFile gzfd;
#endif
   
   memset((void*)&getpl, 0, sizeof(struct oneplaylistent));
   if ( playlist == NULL ) return;
   buf  = malloc(500);
   buf2 = malloc(500);
   while ( playlist->prev != NULL ) playlist = playlist->prev;
   
   if ( directsave ) strcpy(buf2, filename); else {
      sprintf(buf, "\e[?25h%s", config.skin.flistmsg[3]);
      l_status(buf);
      strcpy(buf2, filename);
      strcpy(buf2, readyxline(config.skin.texty, config.skin.textx+ansi_strlen(config.skin.flistmsg[3]), buf2, config.skin.textw-ansi_strlen(config.skin.flistmsg[3]), &exitchar, (int*)&ch));
      if ( exitchar == 27 || !buf2[0] ) {
	 l_status("\e[?25l");
	 fl_updatebuttons(0);
	 free(buf);
	 free(buf2);
	 return;
      }

      l_status(config.skin.flistmsg[4]); /* Save w/ ID3 tags and shiit ? y/n */
      FD_ZERO(&stdinfds);
      FD_SET(0, &stdinfds);
      select(1, &stdinfds, NULL, NULL, NULL);
      
   }

   if ( strchr(buf2, '/') == NULL )
     sprintf(buf, "%s/%s", cdir, buf2); else
     strcpy(buf, buf2);
   
   /* kinda messy, here's the getchar from 10 lines above.. eh, yeah .. it werks :) */
    if ( !directsave && toupper(getchar()) == 'N' ) {
      cplid3 = FALSE; 
      fd = fopen(buf, "w");
   } else
#ifdef HAVE_LIBZ
     if ( config.compresspl ) {
	gzfd = gzopen(buf, "wb");
	gzwrite(gzfd, "CPL+ID3 1.3\n", 12);
     } else {
	fd = fopen(buf, "wb");	   
	fputs("CPL+ID3 1.3\n", fd); 
     }
#else
     {
	fd = fopen(buf, "w");
	fputs("CPL+ID3 1.3\n", fd);
     }
#endif
   
   if ( !directsave ) {      
      sprintf(buf, "\e[?25l%s", config.skin.flistmsg[5]);
      l_status(buf);
   }
   
   if ( cplid3 ) 
     do {
	memcpy((void*)&getpl, (void*)playlist, sizeof(struct oneplaylistent));
#ifdef HAVE_LIBZ
	if ( config.compresspl )
	  gzwrite(gzfd, (void*)&getpl, sizeof(struct oneplaylistent)); else
	  fwrite((void*)&getpl, sizeof(struct oneplaylistent), 1, fd);
#else
	   fwrite((void*)&getpl, sizeof(struct oneplaylistent), 1, fd);
#endif
	if ( playlist->next != NULL ) playlist = playlist->next;
     } while (playlist->next != NULL);
   else 
     do {
	fprintf(fd, "%s\n", playlist->name);
	//	   fputs(playlist->name, fd);
	//	   fputc('\n', fd);
	if ( playlist->next != NULL ) playlist = playlist->next;
     } while ( playlist->next != NULL );
#ifdef HAVE_LIBZ
   if ( config.compresspl && cplid3 ) gzclose(gzfd); else
     fclose(fd);
#else
   fclose(fd);
#endif
   if ( !directsave ) {
      l_status(NULL);
      fl_updatebuttons(0);
   }
   free(buf);
   free(buf2);
   return;
}

void fl_search(char ch, struct filelistent *filelist)   { /* skippa ner till en fil med "ch" f0rst */
   
   if ( current[depth] != fl_maxpos ) filelist = file_seek(current[depth]+1, filelist);
   while ( filelist->next != NULL ) {
      if ( filelist->name[0] == ch ) {
	 current[depth] = filelist->number;
	 screenmark[depth] = 0;
	 fl_showents(current[depth]-screenmark[depth], filelist);
	 break;
      }
      filelist = filelist->next;
   }
   if ( filelist->next == NULL && filelist->name[0] != ch ) {
      filelist = file_seek(0, filelist);
      while ( filelist->number != current[depth] ) {
	 if ( filelist->name[0] == ch ) {
	    current[depth] = filelist->number;
	    screenmark[depth] = 0;
	    fl_showents(current[depth]-screenmark[depth], filelist);
	    break;
	 }
	 filelist = filelist->next;
      }
   }
}

void recurseplaylist (struct filelistent *filelist, struct playlistent **playlist, char *dir) {
int botonviejo;

  if (!filelist) return;

  while (filelist->prev != NULL)
	  filelist=filelist->prev;
  while (filelist->next != NULL) {
	if (strcmp(filelist->name,".") &&
	    strcmp(filelist->name,"..")) {
  	 if ( (strstr(lowercases(filelist->name), ".mp3") != NULL) &&
	     filelist->type == 1 ) 
	           filelist->tagged = TRUE;
	   if (filelist->type == 2) {
	          struct filelistent *newlist;
		  char olddir[256];
		  char newdir[256];
		  sprintf(newdir, "%s%s/",dir,filelist->name);
                  strcpy (olddir,cdir);
		  strcpy (cdir,newdir);
		  newlist=loaddir(newdir);
		  recurseplaylist (newlist, playlist,newdir);
		  strcpy (cdir,olddir);
		  releasedir (newlist);
	   }
	 }
         filelist=filelist->next;
  }
  botonviejo=config.skin.flistbo[fl_buttonpos];
  config.skin.flistbo[fl_buttonpos]=0;       // Agregar Archivos
  fl_dofunction(filelist, playlist);
  config.skin.flistbo[fl_buttonpos]=botonviejo;

 }


#ifdef USE_GPM_MOUSE
int fl_domouse(struct filelistent **filelist, struct playlistent **playlist) {
Gpm_Event event;
int i, j=0;
   
   Gpm_GetEvent(&event);
   
   /* middle button, any kind of click */
   if ( event.buttons & 2 && event.type & GPM_DOWN ) {      
      printf("\e[0m\e[2J\e[1;1H%s", config.skin.filelist);   
      fl_showents(current[depth]-screenmark[depth], *filelist);
      fl_updatebuttons(0);
      }
   
   
   /* left button - any kind of click */
   if ( event.buttons & 4 && event.type & GPM_DOWN && !(event.type & GPM_DRAG) )
     if ( event.y == config.skin.flisty-1 && event.x >= config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) ) {
	/* page up */
	if ( current[depth] < config.skin.flistlines ) { current[depth] = 0; screenmark[depth] = 0; } else
	  current[depth] -= (config.skin.flistlines-1);
	if ( current[depth] < screenmark[depth] )
	  current[depth] = screenmark[depth];
	fl_showents(current[depth]-screenmark[depth], *filelist);
     } else	
     if ( event.y == (config.skin.flisty+config.skin.flistlines) && event.x > config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) ) {
	/* page down */
	if ( current[depth]+(config.skin.flistlines-1) > fl_maxpos ) {
	   screenmark[depth] = 0;
	   current[depth] = fl_maxpos;
	} else
	  current[depth] += (config.skin.flistlines-1);
	fl_showents(current[depth]-screenmark[depth], *filelist);
     } else   
     
     for(i=FL_MINBUTTON;i<(FL_MAXBUTTON+1);i++)
       if ( event.y == config.skin.fy[i] && event.x >= (config.skin.fx[i]-config.skin.fmouseexpand) && event.x < (config.skin.fx[i]+config.skin.fw[i]+config.skin.fmouseexpand) ) {
          for(;j<(FL_MAXBUTTON+1);j++)
	    if ( config.skin.flistbo[j] == i )
	      fl_buttonpos = j;
	  if ( config.skin.flistbo[fl_buttonpos] == FL_BUTTONQUIT ) 
	    return 1;
	  fl_updatebuttons(0);
	  fl_dofunction(*filelist, playlist);
	  break; 
       }
   
   
     /* left button - any click/move */
     if ( event.buttons & 4 && \
        ( event.type & GPM_DRAG || event.type & GPM_DOWN ) )
       if ( event.y >= config.skin.flisty && event.y < (config.skin.flisty+config.skin.flistlines) && \
          event.x > config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) && \
          current[depth]-screenmark[depth]+event.y-(config.skin.flisty) <= fl_maxpos ) {
	  /* Toggle mark (true) */
	  current[depth] = current[depth]-screenmark[depth] + event.y - config.skin.flisty;
	  screenmark[depth] = event.y - config.skin.flisty;
	  if (!file_seek(current[depth]-screenmark[depth]+event.y-config.skin.flisty, *filelist)->tagged) togglemark(*filelist, FALSE);
	  fl_showents(current[depth]-screenmark[depth], *filelist);
       } 
   
   /* right button - any click/move */
   if ( event.buttons & 1 && \
      ( event.type & GPM_DRAG || event.type & GPM_DOWN ) )
     if ( event.y >= config.skin.flisty && event.y < (config.skin.flisty+config.skin.flistlines) && \
        event.x >= config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) && \
        current[depth]-screenmark[depth]+event.y-(config.skin.flisty) <= fl_maxpos ) {
	/* Toggle mark (false)*/
	current[depth] = current[depth]-screenmark[depth] + event.y - config.skin.flisty;
	screenmark[depth] = event.y - config.skin.flisty;
	if (file_seek(current[depth]-screenmark[depth]+event.y-config.skin.flisty, *filelist)->tagged) togglemark(*filelist, FALSE);
	fl_showents(current[depth]-screenmark[depth], *filelist);
     }

   if ( !(event.type & GPM_DRAG) && event.buttons & 4 && event.type & GPM_DOUBLE )
     if ( event.y >= config.skin.flisty && event.y < (config.skin.flisty+config.skin.flistlines) && \
        event.x >= config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) && \
        current[depth]-screenmark[depth]+event.y-config.skin.flisty <= fl_maxpos ) {
	/* Select as current "file" */
	current[depth] = current[depth]-screenmark[depth] + event.y - config.skin.flisty;
	screenmark[depth] = event.y - config.skin.flisty;
	fl_showents(current[depth]-screenmark[depth], *filelist);
	/* Change dir */
	if ( file_seek(current[depth], *filelist)->type == 2 )
	  *filelist = camp_chdir(*filelist);
	fl_showents(current[depth]-screenmark[depth], *filelist);
     }
   
   /* right button, single click */
   if ( event.buttons & 1 && event.type & GPM_DOWN && !(event.type & GPM_DRAG) )
     if ( event.y == config.skin.flisty-1 && event.x >= config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) ) {
	/* home */
	current[depth] = 0;
	screenmark[depth] = 0;
	fl_showents(0, *filelist);
     } else
     if ( event.y == (config.skin.flisty+config.skin.flistlines) && event.x >= config.skin.flistx && event.x < (config.skin.flistx+config.skin.flistw) ) {
	/* end */
	current[depth] = fl_maxpos;
	screenmark[depth] = 0;
	fl_showents(current[depth]-screenmark[depth], *filelist);
     } 
   return 0;
}
#endif
