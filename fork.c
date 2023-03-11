#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "camp.h"

#ifdef HAVE_UTMP_H
# include <utmp.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

extern char quiet, quitmode, checkkill, currloc;
extern unsigned int slavepid, filenumber;
extern struct playlistent *playlist;
extern struct oneplaylistent currentfile;
extern struct configstruct config;

void disappear(void) {
FILE *fd;
int  i;

#ifdef USE_GPM_MOUSE     
   while ( Gpm_Close() != 0 ); /* No need for the mouse anymore */
#endif

   
   switch ( fork() ) {
    case -1:
      perror("\e[1;1Hfork(): \a\a");
      return;
      
    case 0:
      signal(SIGCHLD, playnext);
      fd = fopen(PID_FILE, "w");
      fprintf(fd, "%d\n", getpid());
      fclose(fd);
#ifdef HAVE_SYS_IOCTL_H      
      i = open ("/dev/tty", O_RDWR); 
      ioctl (i, TIOCNOTTY, (char *)0); /* detach from tty */
      close(i);
#endif
#ifdef RC_ENABLED
      if ( config.userc ) ioperm(config.rc.port+1,1,1);
#endif
      quiet = TRUE;
      quitmode = 2;
      while ( !kill(slavepid, 0) ) {
	 usleep( config.rctime );
#ifdef RC_ENABLED
	 if ( config.userc ) checkrc();
#endif 
      }
      playnext(-1);
      
      while ( TRUE ) {
	 usleep( config.rctime );
#ifdef RC_ENABLED
	 if ( config.userc ) checkrc();
#endif 
      }
      break;

    default:
      quitmode = 1;
      exit();     
      
  }
}

void sigusr1(int signr) {
FILE *fd=NULL;
char buf[256];
struct oneplaylistent getpl;

   if ( playlist ) {
      pl_seek(0, &playlist);
      sprintf(buf, "%s/playlist.camp", TMP_DIR);
      saveplaylist(playlist, buf, TRUE);
   }
      
   sprintf(buf, "%s/data.camp", TMP_DIR);
   fd = fopen(buf, "w");
   fprintf(fd, "%d\n%d\n", slavepid, filenumber);
   fclose(fd);
   
   updatesongtime('w');
   
   if ( quitmode == 0 ) quitmode = 3; else
     quitmode = 2;
   exit();
   
}

void stealback(void) {
FILE *fd;
char buf[256];
unsigned int oldpid;
struct stat statbuf;
#ifdef HAVE_UTMP_H
 struct utmp entry;
#endif
   
   fd = fopen(PID_FILE, "r");
   if ( !fd ) {
      printf("No session to steal!\n");
      exit();
   }
   fscanf(fd, "%d\n", &oldpid);
   fclose(fd);
   printf("Stealing (pid: %d)... ", oldpid); fflush(stdout);
   sprintf(buf,"/proc/%d",oldpid);
   if (getuid()!=0) {
     if (stat (buf,&statbuf) == -1) {
        printf("\nNothing to steal... :-(\n");
        exit (-1);
      }
     if ( statbuf.st_uid != getuid()) {
        printf("\nCan't steal other's session, sorry.\n");
          exit (-1);
         }
     }

   
   sprintf(buf, "%s/info.camp", TMP_DIR);	
   fd = fopen(buf, "wb");

#ifdef HAVE_UTMP_H
   setutent();
   strcpy(buf, ttyname(0));
   if ( !strncmp(buf, "/dev/", 5) )
     strcpy(entry.ut_line, buf+5); else
     strcpy(entry.ut_line, rindex(buf, '/')+1);
   
   memcpy((char*)&entry, getutline((struct utmp*)&entry), sizeof(struct utmp));
   endutent();
   
   if ( ! entry.ut_host[0] ) strcpy(entry.ut_host, "local console");
   
   fprintf(fd, "%s at %s from %s.\n", entry.ut_user, entry.ut_line, entry.ut_host);
#else
   fprintf(fd, "%s at %s.\n", getenv("USER"), ttyname(0));
#endif
   fclose(fd);   

   kill(oldpid, SIGUSR1);
   while ( !kill(oldpid, 0) )
     usleep(250000);
   sprintf(buf, "%s/data.camp", TMP_DIR);
   fd = fopen(buf, "r");
   if ( !fd ) {
      printf("No data found, can't steal!\n");
      exit();
   }
   fscanf(fd, "%d\n%d\n", &slavepid, &filenumber);
   fclose(fd);
   sprintf(buf, "%s/data.camp", TMP_DIR);
   unlink(buf);
   
   sprintf(buf, "%s/playlist.camp", TMP_DIR);
   if ( exist(buf) ) {
      loadplaylist(&playlist, buf, FALSE);
      pl_seek(filenumber, &playlist);
      unlink(buf);
      printf("done!\nplayer pid: %d, playing \"%s\" (%d)\n", slavepid, playlist->showname, filenumber+1);
      memcpy((void*)&currentfile, playlist, sizeof(struct oneplaylistent));
   } else printf("done!\n");
      
   updatesongtime('r');
   scrollsongname(0);
   
}

void killcamp(void) {
FILE *fd;
int oldpid;
char buf[256];
   
   fd = fopen(PID_FILE, "r");
   if ( !fd ) {
      printf("No session to kill!\n");
      exit();
   }
   fscanf(fd, "%d\n", &oldpid);
   fclose(fd);
   printf("Killlig (pid: %d)... ", oldpid); fflush(stdout);
   kill(oldpid, SIGUSR1);   
   while ( !kill(oldpid, 0) )
     usleep(250000);
   sprintf(buf, "%s/data.camp", TMP_DIR);
   fd = fopen(buf, "r");
   if ( !fd ) {
      printf("No session to kill!\n");
      exit();
   }
   fscanf(fd, "%d\n", &oldpid);
   fclose(fd);
   kill(oldpid, SIGKILL);
   while ( !kill(oldpid, 0) )
     usleep(250000);   
   unlink(buf);
   sprintf(buf, "%s/playlist.camp", TMP_DIR);
   unlink(buf);
   sprintf(buf, "%s/time.camp", TMP_DIR);
   unlink(buf);
   printf("done!\n");
   exit();
}
