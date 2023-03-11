#include <stdio.h>
#include <string.h>
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
extern struct currentplaylistent currentfile;
extern char quiet, pausesong, playsong, checkkill, currloc;
extern struct configstruct config;
extern int pl_current, pl_screenmark;

void playnext(int sig)
{
// int status = 0;

    if ( !playlist ) return;
    if ( sig == SIGALRM ) {
        playsong = TRUE;
        sig = -1;
    }

    if ( ( sig == -1 ) || ( ( ( playsong  && !pausesong ) || config.mpg123 ) && slavepid && ( waitpid(slavepid, NULL, WNOHANG) == slavepid ) ) ) {
        if ( sig != -1 ) waitpid(slavepid, NULL, 0);
        if ( config.mpg123 && sig != -1 ) mpg123_control("#RESTART");

        if ( config.playmode != 2 ) {
            if ( filenumber+1 == playlistents && config.playmode == 1 ) filenumber = playlistents+2;
            else if ( filenumber+1 == playlistents && config.playmode == 0 ) exit(0);
            filenumber++;
        } else
            filenumber = myrand(playlistents);
        if ( filenumber+1 > playlistents ) filenumber = 0;
        if ( !config.mpg123 ) slavepid = 0;
        call_player(pl_seek(filenumber, &playlist));
    } else
        signal(SIGCHLD, playnext);
}

void killslave()
{
    if ( !slavepid ) return;
    signal(SIGCHLD, SIG_IGN);
    if ( pausesong ) kill(slavepid, SIGCONT);
    pausesong = FALSE;
    if ( config.nicekill )
        kill(slavepid, SIGTERM);
    else
        kill(slavepid, SIGKILL);
    if ( config.kill2pids ) kill(slavepid+1, SIGKILL); /* This is no good, but it (usually) works for multithreaded-buffering players ;) */
    if ( checkkill )
        while ( kill(slavepid, 0) != -1 ) usleep(1000);
    else
        waitpid(slavepid, NULL, 0);
    slavepid = 0;
    checkkill = FALSE;
}

void slave(char *filename)
{
    char buf[256];
    int i=0;
// FILE *fd;

#ifdef USE_GPM_MOUSE
    while ( Gpm_Close() != 0 ) ;
#endif
    if ( !config.dontreopen ) {
        if(freopen("/dev/null", "w", stdout) == NULL) perror("freopen(stdout)");
        if(freopen("/dev/null", "w", stderr) == NULL) perror("freopen(stderr)");
    }
    for(i=0; i<14; i++)
        if (config.playerargv[i] == NULL) break;
    config.playerargv[i] = (char*)malloc(strlen(filename)+1);
    strcpy(config.playerargv[i], filename);
    sprintf(buf, "%s/%s", config.playerpath, config.playername);
    if ( execve(buf, config.playerargv, NULL) == -1 ) {
        printf("Execution of player (%s) failed!\n", buf);
        perror("execve()");
        sleep(2);
        exit(-1);
    }
}


void call_player(struct playlistent *pl)
{
// int  j, cspos=0;
    char name[31], artist[31], buf[500];
    struct stat statf;
    FILE *fd;

    if ( pl == NULL || pausesong ) return;

    memcpy((void*)&currentfile, (void*)pl, sizeof(struct oneplaylistent));

    if ( config.rescanid3 || ( !config.useid3 && !pl->length ) ) {
        if ( getmp3info(pl->name, &pl->mode, &pl->samplerate, &pl->bitrate, name, artist, NULL, NULL, NULL, NULL ) ) {
            if ( artist[0] ) sprintf(pl->showname, "%s - %s", artist, name );
            else
                strcpy(pl->showname, name);
        }
        if ( pl->bitrate ) {
            stat(pl->name, (struct stat*)&statf);
            pl->length     = statf.st_size / (pl->bitrate * 125);
        }
    }

    memcpy((void*)&currentfile, (void*)pl, sizeof(struct oneplaylistent));

    scrollsongname(0);
    scrollsongname(1); /* Hrm */
    updatesongtime('i');
    updatesongtime('f');
    updatedata();
    if ( !quiet && currloc == CAMP_MAIN && config.skin.platmain ) {
        pl_current = filenumber;
        if ( pl_current-pl_screenmark < 0 )
            pl_screenmark = pl_current;
        pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
    }

    if ( playsong ) {

        /* drop info to file, for stupid things like irc-scripts  */
        if ( config.dropinfo ) {
            fd = fopen(config.dropfile, "w");
            if ( fd ) {
                fprintf(fd, "camp\n%s\n%s\n%s\n%u\n%u\n", CAMP_VERSION, currentfile.name, currentfile.showname, currentfile.bitrate, currentfile.samplerate);
                fclose(fd);
            }
        }

        if ( config.mpg123 ) { /* mpg123 jukebox */
            if ( !exist(pl->name) && strncasecmp(pl->name, "http://", 7) ) {
                playsong = FALSE;
                signal(SIGALRM, playnext);
                alarm(1);
                return;
            }
            sprintf(buf, "LOAD %s\n", currentfile.name);
            mpg123_control(buf);
            signal(SIGCHLD, playnext);
        } else { /* Normal inm-mode */

            if ( config.bufferdelay ) usleep(config.bufferdelay);

            if ( slavepid != 0 ) killslave();

            if ( !exist(pl->name) && strncasecmp(pl->name, "http://", 7) ) {
                playsong = FALSE;
                signal(SIGALRM, playnext);
                alarm(1);
                return;
            }

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
                setpriority(PRIO_PROCESS, slavepid, config.playerprio);
                break;
            } /* switch( fork() ); */
            //signal(SIGCHLD, playnext);
        } /* end normal-mode */
    }
    return;
}

char *mpg123_control(char *command)
{
    static int mpgrfd=-1, mpgwfd=-1;
    int insocks[2], outsocks[2]; // 0 = read, 1 = write
    fd_set rfds;
    struct timeval tv;
    static char buf[500];
    int i, j;

    if ( command && !strcmp(command+1, "RESTART") ) {
        /* Oh my god!! They've killed kenny! */
        close(mpgwfd);
        close(mpgrfd);
        mpgrfd = mpgwfd = -1; /* Yep, let's go back to scratch */
    } else if ( !playsong ) return NULL;

    if ( mpgrfd == -1 || mpgwfd == -1 ) { /* start mpg123, setup control pipe */

        if ( pipe(insocks) == -1 ) {
            printf("Cannot create pipes! Try set \"mpg123mode = false\" in ~/.camp/camprc!\n");
            sleep(2);
            exit(-1);
        }

        if ( pipe(outsocks) == -1 ) {
            printf("Cannot create pipes! Try set \"mpg123mode = false\" in ~/.camp/camprc!\n");
            sleep(2);
            exit(-1);
        }

        slavepid = fork();
        switch ( slavepid ) {
        case -1:
            printf("Cannot fork! Try restarting camp!\n");
            sleep(2);
            exit(-1);
        case 0:
            dup2(outsocks[0], STDIN_FILENO);
            dup2(insocks[1], STDOUT_FILENO);
            dup2(insocks[1], STDERR_FILENO);
            close(outsocks[0]);
            close(outsocks[1]);
            close(insocks[0]);
            close(insocks[1]);

            for(i=0; i<13; i++)
                if (config.playerargv[i] == NULL) break;

            config.playerargv[i] = (char*)malloc(3);
            strcpy(config.playerargv[i], "-R");
            i++;
            config.playerargv[i] = (char*)malloc(1);
            config.playerargv[i][0] = 0;
            sprintf(buf, "%s/%s", config.playerpath, config.playername);

            if ( execve(buf, config.playerargv, NULL) == -1 ) {
                printf("Execution of player (%s) failed!\n", buf);
                perror("execve()");
                sleep(2);
                exit(-1);
            }
            break;

        default: /* rock 'n' roll, we're up kicking */
            mpgrfd = insocks[0];
            mpgwfd = outsocks[1];
            setpriority(PRIO_PROCESS, slavepid, config.playerprio);
            break;

        } /* switch fork .. */

        if ( command && !strcmp(command+1, "RESTART") ) return NULL;
    } /* Init session end. */

    if ( command && command[0] != '#' ) { /* write .. */

        //      write(outsocks[1], command, strlen(command)+1);
        if ( write(mpgwfd, command, strlen(command)) != strlen(command) ) {
            /* Write error! This is no good.. */
            close(mpgwfd);
            close(mpgrfd);
            killslave();
            mpgrfd = mpgwfd = -1;
            return NULL;
        }

    } else { /* read .. */

        i = 1;
        while ( i ) { /* As long as mpg123 has something to report.. */

            FD_ZERO(&rfds);
            FD_SET(mpgrfd, &rfds);
            tv.tv_sec = tv.tv_usec = 0;

            if ( select(mpgrfd+1, &rfds, NULL, NULL, &tv) && FD_ISSET(mpgrfd, &rfds) ) {
                /* Data availible in pipe */

//                memset(buf, 0, 500);
//                while ( (read(mpgrfd, ch, 1) != 0) && ch[0] != '\n'  )
//                    if ( ch[0] != '\r' && ch[0] != '\n' ) strncat(buf, ch, 1);

                /* There must be a better way to read single lines from files/piles eh?
                 * something like fgets .. please inform me if there is such a command :)
                 * MT:  ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream); */

                for (j = 0; read(mpgrfd, &buf[j], 1) != 0 && buf[j] && !IS_CRLF(buf[j]); j++);
                buf[j] = 0;

                if ( !strncmp(buf, "@R ", 3) ) {
                    /* Get mpg123 version
                    Format: @R MPG123 0.59s-mh4
                       or.. @R MPG123  .. older
                       or in the 21st century:
                            @R MPG123 (ThOr) v7
                    Older version than 0.59s that cannot handle the quickjump
                    */

                    if ( buf[10] == '0' && strncmp(buf+10, "0.59s", 4) < 0 )
                        config.mpg123fastjump = FALSE;
                    else
                        config.mpg123fastjump = TRUE;
                }

                if ( command )
                    if ( !strncmp(buf, command+1, strlen(command)-1) ) i = 0;

                if ( !strncmp(buf, "@P 0", 4) && playsong )
                    playnext(-1);
                else if ( !strncmp(buf, "@F ", 3) && playsong ) {
                    strcpy(buf, (char*)strchr(buf, ' ')+1 ); /* @F */
                    currentfile.frame = atoi( buf );         /* Frames played */
                    strcpy(buf, (char*)strchr(buf, ' ')+1 );
                    strcpy(buf, (char*)strchr(buf, ' ')+1 ); /* Skip frames left */
                    currentfile.played = atoi( buf );        /* Seconds played */
                    strcpy(buf, (char*)strchr(buf, ' ')+1 );
                    currentfile.left = atoi( buf );	  /* Seconds left */
                    if ( strchr(buf, ' ') ) {                /* Bitrate appears only in 0.59s+ */
                        strcpy(buf, (char*)strchr(buf, ' ')+1 );
                        currentfile.bitrate = atoi( buf ) / 1000; /* Read current bitrate, VBR is nice.. */
                    }

                } else if ( !strncmp(buf, "@S ", 3) ) {
                    /* @S 1.0 3 44100 Joint-Stereo 0 417 2 1 0 0 128 0 --- Start-info */
                } else if ( !strncmp(buf, "@I ID3:", 7) ) {
                    // ID3 TAGS
                }

//	    else printf("\e[1;1HEWP! GOT LINE: %s--\n", buf);


            } else if ( !command || command[0] != '#' ) return NULL; /* pipe data end */
        } /* while 1 .. */
    } /* end read */
    return buf;
}

