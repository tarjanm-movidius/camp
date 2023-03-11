#include <stdio.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "camp.h"

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

extern char quiet, quitmode;
extern unsigned int slavepid, filenumber;
extern struct playlistent *playlist;
extern struct oneplaylistent currentfile;

void disappear(void) {
FILE *fd;
   
   switch (fork()) {
    case -1:
      perror("\e[1;1Hfork(): \a\a");
      return;
      
    case 0:
      sleep(1);
      fd = fopen(PID_FILE, "w");
      fprintf(fd, "%d\n", getpid());
      fclose(fd);
      quiet = TRUE;
      quitmode = 2;
      while ( !kill(slavepid, 0) )
	sleep(1);
      playnext(0);
#ifdef USE_GPM_MOUSE      
      while ( Gpm_Close() ); /* No need for the mouse anymore */
#endif
      while ( TRUE ) sleep(9999);
      break;
      
    default:
      quitmode = 1;
      exit();      
   }
   
}

void sigusr1(int signr) {
FILE *fd;
char buf[256];
struct oneplaylistent getpl;
   
   if ( playlist ) {
      playlist = pl_seek(0, playlist);
      sprintf(buf, "%s/playlist.camp", TMP_DIR);
      fd = fopen(buf, "w");
      fprintf(fd, "CPL+ID3 1.3\n");
      
      /* save playlist.. */
      
      do {
	 memcpy((void*)&getpl, (void*)playlist, sizeof(struct oneplaylistent));
	 fwrite((void*)&getpl, sizeof(struct oneplaylistent), 1, fd);
	 if ( playlist->next != NULL ) playlist = playlist->next;
      } while (playlist->next != NULL);
      
      fclose(fd);
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
   
   fd = fopen(PID_FILE, "r");
   if ( !fd ) {
      printf("No session to steal!\n");
      exit();
   }
   fscanf(fd, "%d\n", &oldpid);
   fclose(fd);
   printf("Stealing (pid: %d)... ", oldpid); fflush(stdout);
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
      playlist = loadplaylist(playlist, buf, FALSE);
      playlist = pl_seek(filenumber, playlist);
      unlink(buf);
      printf("done!\nplayer pid: %d, playing \"%s\" (%d)\n", slavepid, playlist->showname, filenumber+1);
      memcpy((void*)&currentfile, playlist, sizeof(struct oneplaylistent));
      if ( strlen(currentfile.showname) >= 51 ) currentfile.showname[50] = '\0';
   } else printf("done!\n");
      
   updatesongtime('r');
   
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
