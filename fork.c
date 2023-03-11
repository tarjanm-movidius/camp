#include <stdio.h>
#include <stdlib.h>
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

extern char quiet, quitmode, checkkill, currloc, playsong;
extern unsigned int slavepid, filenumber;
extern struct playlistent *playlist;
extern struct currentplaylistent currentfile;
extern struct configstruct config;
extern char use_lircd;

void disappear(void)
{
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

            if ( config.mpg123 )
                mpg123_control(NULL);

            usleep( config.rctime );
#ifdef LIRCD
            if ( use_lircd ) dolircd(0);
#endif
#ifdef RC_ENABLED
            if ( config.userc ) checkrc();
#endif
        }
        playnext(-1);

        while ( TRUE ) {

            if ( config.mpg123 )
                mpg123_control(NULL);

            usleep( config.rctime );
#ifdef RC_ENABLED
            if ( config.userc ) checkrc();
#endif
        }
        break;

    default:
        quitmode = 1;
        exit(0);

    }
}

void sigusr1(int signr)
{
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
    fprintf(fd, "%d\n%d\n%d\n", slavepid, filenumber, currentfile.frame);
    fclose(fd);

    updatesongtime('w');

    /*   if ( config.mpg123 ) {
        (void*)mpg123_control("QUIT\n");
        kill(slavepid, SIGKILL);
        sleep(1);
       } */

    if ( quitmode == 0 ) quitmode = 3;
    else
        quitmode = 2;
    exit(0);

}

void stealback(void)
{
    FILE *fd;
    char buf[256];
    unsigned int oldpid;
    struct stat statbuf;
#ifdef HAVE_UTMP_H
    struct utmp usearch;
    struct utmp *ureturn;
#endif

    fd = fopen(PID_FILE, "r");
    if ( !fd ) {
        printf("No session to steal!\n");
        exit(0);
    }
    fscanf(fd, "%d\n", &oldpid);
    fclose(fd);
    printf("Stealing (pid: %d)... ", oldpid);
    fflush(stdout);
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
        strcpy(usearch.ut_line, buf+5);
    else
        strcpy(usearch.ut_line, rindex(buf, '/')+1);

    ureturn = getutline((struct utmp*)&usearch);
    if ( ureturn ) {
        if ( ! ureturn->ut_host[0] )
            fprintf(fd, "%s at %s from local console.\n", ureturn->ut_user, ureturn->ut_line);
        else
            fprintf(fd, "%s at %s from %s.\n", ureturn->ut_user, ureturn->ut_line, ureturn->ut_host);
    } else
        fprintf(fd, "%s at %s.\n", getenv("USER"), ttyname(0));
    endutent();


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
        exit(0);
    }
    fscanf(fd, "%d\n%d\n%d\n", &slavepid, &filenumber, &currentfile.frame);
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
        if ( config.mpg123 ) {
            signal(SIGCHLD, playnext);
            playsong = TRUE;
            sprintf(buf, "LOAD %s\n", currentfile.name);
            (void*)mpg123_control(buf);
            if ( !config.mpg123fastjump ) (void*)mpg123_control("#@S");
            /* -- for versions of mpg123 earlier than 0.59s to resume song currectly */
            sprintf(buf, "JUMP +%d\n", currentfile.frame);
            (void*)mpg123_control(buf);
        }
    } else printf("done!\n");

    updatesongtime('r');
    scrollsongname(0);

}

void killcamp(void)
{
    FILE *fd;
    int oldpid;
    char buf[256];

    fd = fopen(PID_FILE, "r");
    if ( !fd ) {
        printf("No session to kill!\n");
        exit(0);
    }
    fscanf(fd, "%d\n", &oldpid);
    fclose(fd);
    printf("Killlig (pid: %d)... ", oldpid);
    fflush(stdout);
    kill(oldpid, SIGUSR1);
    while ( !kill(oldpid, 0) )
        usleep(250000);
    sprintf(buf, "%s/data.camp", TMP_DIR);
    fd = fopen(buf, "r");
    if ( !fd ) {
        printf("No session to kill!\n");
        exit(0);
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
    exit(0);
}
