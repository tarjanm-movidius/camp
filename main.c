#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/vt.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include "camp.h"
#include "chromansi.c"

#ifdef HAVE_TERMIOS_H
# include <fcntl.h>
# include <termios.h>
struct termios oldtermios, newtermios;
#else
# include <curses.h>
#endif

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

#ifdef LIRCD
#include "lirc_client.h"
#endif

/*
 *   Console Ansi Mpeg3 Player interface v1.5 by inm
 *
 * If you improve this code, please send a copy, or a
 * diff of it to inm@sector7.nu.
 * Latest version can be found at
 * http://www.sector7.nu/camp
 *
 *                                                    */

#define KM_UP     1
#define KM_DOWN   2
#define KM_LEFT   3
#define KM_RIGHT  4
#define KM_NONE   0

const char keys[] = { '1', '4', '5', '6' };
/* const char keys[] = { 'H', 'F', 'I', 'G' }; */

/* Why did I really do this key-shit? this lame proggy still only support
 standard linux terminals and similar .. that's the only OS I run nowdays..
*/

/* ... and here goes all the nice global variables that I were planning on
 removing someday. Now I realised that I really don'r care about them..
 so what da heck. Feel like fixing it? You're welcome ..
*/

extern int pl_current, pl_screenmark, pl_maxpos;

char playsong=FALSE, quiet=TRUE, pausesong=FALSE, force=FALSE, checkkill=FALSE, quitmode=0, buttonpos, muted=FALSE,
     use_lircd=TRUE, alwayslocked=FALSE;
char nosteal = FALSE;
char currloc = CAMP_MAIN;/* We are here */
char passwd[15];
struct playlistent *playlist = NULL;
struct currentplaylistent currentfile;
unsigned int slavepid=0, playlistents=0, filenumber=0;
int selval;
struct configstruct config;
#ifdef USE_GPM_MOUSE
Gpm_Connect mouse;
#endif

#ifdef LIRCD
char *progname = "camp";
extern int lirc_lircd;
struct lirc_config *lircd_config = NULL;
#endif



int main(int argc, char *argv[])
{
    unsigned int j;
    char   buf[100], /*c,*/ dump=FALSE;
    int    ch, /*cspos=0,*/ i, stdinno = fileno(stdin);
    FILE   *fd;
    fd_set fds;
    struct timeval currenttime;
//int left, right;

    printf("Console Ansi Mpeg3 Player interface v%s.%d by inm (inm@sector7.nu)\n\n", CAMP_VERSION, BUILD);
    if ( argc > 1 && !strcasecmp(argv[1], "--version") ) exit(0);
    sprintf(buf, "%s/.camp/camprc", getenv("HOME"));
    printf("Loading config from %s...\n", buf);
    config = getconfig(buf);

    if ( config.showtip ) if ( !showtip() ) config.showtip = FALSE;

    signal(SIGTERM, sighandler);
//   signal(SIGTERM, SIG_IGN);
    signal(SIGUSR2, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGHUP,  sighandler);

#ifdef USE_GPM_MOUSE
    my_Gpm_Init(&mouse);
    atexit(close_gpm);
#endif

    if ( argc != 1 ) {
        printf("Scanning arguments, please wait.. ");
        for(i=1; i<argc; i++) {
            printf("\e[s[%d/%d]\e[K\e[u", i-1, argc-1);
            fflush(stdout);
            if ( !strcasecmp(argv[i], "-r") || !strcasecmp(argv[i], "--renameid3") )
                dump = 2;
            else if ( !strcasecmp(argv[i], "-d") || !strcasecmp(argv[i], "--dumpid3") )
                dump = TRUE;
            else if ( !strcasecmp(argv[i], "-f") || !strcasecmp(argv[i], "--force") )
                force = TRUE;
            else if ( !strcasecmp(argv[i], "-l") || !strcasecmp(argv[i], "--lock") )
                alwayslocked = TRUE;
            else if ( !strcasecmp(argv[i], "-n") || !strcasecmp(argv[i], "--nosteal") )
                nosteal = TRUE;
            else if ( !strcasecmp(argv[i], "-p") || !strcasecmp(argv[i], "--playlist") ) {
                i++;
                loadplaylist(&playlist, argv[i], FALSE);
            } else if ( !strcasecmp(argv[i], "-s") || !strcasecmp(argv[i], "--steal") ) {
                stealback();
                checkkill = TRUE;
            } else if ( !strcasecmp(argv[i], "-k") || !strcasecmp(argv[i], "--kill") )
                killcamp();
            else if ( !strcasecmp(argv[i], "--skin") ) {
                i++;
                unloadskin(&config.skin);
                loadskin(argv[i], &config);
            } else if ( !strcasecmp(argv[i], "-b") || !strcasecmp(argv[i], "--background") ) {
                quitmode = 1;
                quiet = TRUE;
            } else
#ifdef RC_ENABLED
                if ( !strcasecmp(argv[i], "--norc") )
                    config.userc = FALSE;
                else
#endif
                    if ( !strcasecmp(argv[i], "--help") || !strcasecmp(argv[i], "-h") ) {
                        printf("\nUsage: camp [OPTIONS] [FILES]\n\n");
                        printf("Options:\n");
                        printf("  -b,   --background   Forks camp into background directly\n");
                        printf("  -d,   --dumpid3      Dumps id3 tags to stderr\n");
                        printf("  -f,   --force        Force run, even if camp's running. (Won't write pidfile)\n");
                        printf("  -h,   --help         This help\n");
                        printf("  -k,   --kill         Kill running camp session\n");
                        printf("  -l,   --lock         Lock computer to camp console while running\n");
                        printf("        --skin X       Use skin 'X' (override config)\n");
                        /*	      printf("  -r,   --renameid3    Rename files to id3.artist - id3.songname.mp3\n"); */
                        printf("  -p X, --playlist X   Load playlist 'X'\n");
                        printf("  -s,   --steal        \"Steal\" another camp session\n");
#ifdef RC_ENABLED
                        printf("        --norc         Don't try to use the remote control\n");
#endif
                        printf("\nAny other parameter will be taken for a regular mp3 file.\n");
                        exit(0);
                    } else
                        addfiletolist(&playlist, argv[i], NULL, 0, 0, 0, config.useid3);
        }
        printf("\e[s[%d/%d]\e[K\e[u", i-1, argc-1); // fflush(stdout);
        printf("done!      \n");
    }

    buttonpos = config.skin.msb;

    if ( dump ) {
        printf("Dumping ID3 tags to stderr..\n");
        if ( !playlist ) {
            fprintf(stderr, "No files to dump tags from!\n");
            exit(0);
        }
        pl_seek(0, &playlist);
        while ( playlist->next ) {
            fprintf(stderr, "%-5u %s\n", playlist->number, playlist->showname);
            fflush(stderr);
            if ( dump == 2 ) {
                strcpy(buf, playlist->showname);
                strcat(buf, ".mp3");
                rename(playlist->name, buf);
            }
            playlist = playlist->next;

        }
        printf("Done!\n");
        exit(0);
    }

    if ( !checkkill && exist(PID_FILE) ) {
        fd = fopen(PID_FILE, "r");
        fscanf(fd, "%u\n", &j);
        fclose(fd);
        sprintf(buf, "/proc/%u", j);
        if ( !kill(j, 0) || exist(buf) ) {
            if ( force )
                printf("Another camp session is already running (--force specified, overriding kill).\n");
            else {
                printf("Camp is already running!\nUse camp -s to steal running process, camp -k to kill it, or camp -f to start anyway.\n");
                if ( playlist != NULL ) clearplaylist(&playlist);
                exit(0);
            }
        }
    }

    if ( !playlist && config.defpl ) {
        sprintf(buf, "%s/.camp/default.pl", getenv("HOME"));
        if ( exist(buf)) {
            loadplaylist(&playlist, buf, FALSE);
            playsong = TRUE;
        }
    } else if ( playlist ) playsong = TRUE;



    if ( config.skin.platmain )
        pl_maxpos = pl_count(playlist)-1;


#ifdef RC_ENABLED
    if ( config.userc ) {
        printf("Initializing LPT remote control... ");
        if ( ioperm(config.rc.port+1,1,1) == -1 ) {
            printf("error, requires root! disabled.\n");
            config.userc = FALSE;
        } else {
            printf("ok!\n");
            checkrc();
        }
    }
#endif

#ifdef LIRCD
    printf("Initializing LIRCD remote control ...\n");
    if ( lirc_init("camp", 1) == -1 ) {
        printf("LIRCD Failed!\n");
        use_lircd = FALSE;
        lirc_lircd = 0;
    } else if ( lirc_readconfig(NULL, &lircd_config, NULL) != 0 ) {
        printf("Error reading LIRCD config file %s!\n", LIRCCFGFILE);
        use_lircd = FALSE;
    } else  {
#ifdef HAVE_TERMIOS_H
        fcntl(lirc_lircd, F_SETFL, O_NONBLOCK);
#endif
        printf("LIRCD Initialized succesfully!\n");
    }
#endif

    if ( config.mpg123 ) {
        playsong = TRUE;
        (void*)mpg123_control(NULL);
//      printf("Awaiting mpg123 to become ready.."); fflush(stdout);
//      (void*)mpg123_control("#@"); /* R MPG123"); */
//      sleep(2);
//      mpg123_control("load /Mp3/12. Sy & Demo - Tears Run Cold.mp3\n");
//      printf("Rock 'n' roll!\n");
    }

    if ( config.showtip ) sleep(4);
    else usleep(250000);

    if ( playlist && playsong ) {
        playlistents = pl_count(playlist);
        if ( !slavepid || ( config.mpg123 && !checkkill ) ) {
            if ( config.playmode == 2 ) filenumber = myrand(playlistents);
            call_player(pl_seek(filenumber, &playlist));
        } else {
            signal(SIGCHLD, playnext);
        }
    }

    if ( config.mpg123 ) checkkill = FALSE;

    if ( quitmode == 1 ) disappear();

    quiet = FALSE;
    atexit(myexit);
    myinit();


    updatedata();
    if ( config.skin.platmain ) {
        pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
    }

    if ( alwayslocked ) {
        quiet = TRUE;
        printf("\e[?25h"); /* cursor on */
        buf[0] = 1;
        passwd[0] = 0;
        while ( strcmp(passwd, buf) ) {
            printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[0]);
            readpass(passwd, 15);
            printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[1]);
            readpass(buf, 15);
        }

        printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));

        printf("\e[?25l"); /* cursor on */
        i = open("/dev/console",O_WRONLY);
        if ( i == -1 ) alwayslocked = FALSE;
        else ioctl(i, VT_LOCKSWITCH);
        close(i);
        quiet = FALSE;
    }

    selval = stdinno + 1;

#ifdef USE_GPM_MOUSE
    if ( gpm_flag > 0 ) selval = gpm_fd + 1;
#endif

#ifdef LIRCD
    if ( use_lircd ) selval = lirc_lircd + 1;
#endif

    while ( TRUE ) { /* main loop */


        if ( config.mpg123 )
            mpg123_control(NULL);

        updatesongtime('u');

        if ( checkkill && kill(slavepid, 0) == -1 ) {
            checkkill = FALSE;
            playnext(-1);
        }

#ifdef RC_ENABLED
        if ( config.userc ) checkrc();
#endif

        currenttime.tv_usec = config.rctime;
        currenttime.tv_sec  = 0;
        FD_ZERO(&fds);
        FD_SET(stdinno, &fds);

#ifdef USE_GPM_MOUSE
        if ( gpm_flag > 0 ) FD_SET(gpm_fd, &fds);
#endif
#ifdef LIRCD
        if ( use_lircd ) FD_SET(lirc_lircd, &fds);
#endif

        if ( select(selval, &fds, NULL, NULL, &currenttime) > 0 ) {
#ifdef USE_GPM_MOUSE
            if ( gpm_flag > 0 && FD_ISSET(gpm_fd, &fds) ) main_domouse(NULL);
#endif
#ifdef LIRCD
            if ( use_lircd && FD_ISSET(lirc_lircd, &fds) ) dolircd(1);
#endif

            ch=0;
            if ( FD_ISSET(stdinno, &fds) ) {
                ch = getchar();
                if   ( ch == 27 )   escfix();
                else if ( ch == 3  && canexit() ) exit(0);
                else /* ^C */
                    if ( ch == 26 && canexit() ) disappear();
                    else /* ^Z */
                        if ( ch == 12 ) { /* ^L */
                            printf("\e[0m\e[2J\e[1;1H%s", config.skin.main);
                            updatedata();
                            updatesongtime('f');
                            updatebuttons(0);
                            if ( config.skin.platmain ) pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
                        } else if (ch == 13 && dofunction(-1)) call_player(pl_seek(filenumber, &playlist));
                        else
                            for(i=MINBUTTON; i<(MAXBUTTON+1); i++)
                                if ( ch == config.skin.mh[i] ) {
                                    if ( config.skin.ma[i] && config.skin.mi[i] ) {
                                        buttonpos = i;
                                        updatebuttons(0);
                                    }
                                    if ( dofunction(i) ) call_player(pl_seek(filenumber, &playlist));
                                    break;
                                }
            } /* FD_ISSET */
        } /* select */
    } /* while(endless) */
    return 0; /* Uhm, just wasting bytes, this is not possible */
} /* main() */


/*     Procedures & functions
 *     ----------------------
 *
 *     I hate this program! It has grown so f***ing big.
 *     I can hardly find out what stuff does what shit anymore ..
 *     (but it's still my favourite console player, heh)
 */


void updatebuttons(char motion)
{
    int i;

    if ( motion != KM_NONE ) {
        if ( motion == KM_UP    && config.skin.mju[(int)buttonpos] != -1 ) buttonpos = config.skin.mju[(int)buttonpos];
        else if ( motion == KM_DOWN  && config.skin.mjd[(int)buttonpos] != -1 ) buttonpos = config.skin.mjd[(int)buttonpos];
        else if ( motion == KM_LEFT  && config.skin.mjl[(int)buttonpos] != -1 ) buttonpos = config.skin.mjl[(int)buttonpos];
        else if ( motion == KM_RIGHT && config.skin.mjr[(int)buttonpos] != -1 ) buttonpos = config.skin.mjr[(int)buttonpos];
    }

    for (i=MINBUTTON; i<(MAXBUTTON+1); i++) {
        if ( !config.skin.my[i] + !config.skin.mx[i] ) continue;
        printf("\e[%d;%dH",config.skin.my[i], config.skin.mx[i]);
        if (buttonpos == i) printf("%s", config.skin.ma[i]);
        else
            printf("%s", config.skin.mi[i]);
    }

    // fflush(stdout);
}

void myinit(void)
{
    FILE *fd;
//long flags;

    if ( !force ) {
        fd = fopen(PID_FILE, "w");
        if ( !fd ) {
            printf("Uh, couldn't write pid file.. ");
            perror("fopen");
        } else {
            fprintf(fd, "%d\n", getpid());
            fclose(fd);
        }
    }
    signal(SIGUSR1, sigusr1);
#ifdef HAVE_TERMIOS_H
    setvbuf(stdout, (char *)NULL, _IONBF, 0);
    tcgetattr(fileno(stdin), &oldtermios);
    memcpy(&newtermios, &oldtermios, sizeof(struct termios));
    termios_raw(&newtermios);
    tcsetattr(fileno(stdin), TCSANOW, &newtermios);
#else
    initscr();
    intrflush(stdscr, FALSE);
    savetty();
    noecho();
    nl();
    cbreak();
#endif
    if ( !quiet ) {
        printf("\e[0m\e[2J\e[?25l\e[1;1H\e(U%s", config.skin.main);
        updatebuttons(0);
    }
}

#ifdef USE_GPM_MOUSE
void close_gpm(void)
{
    while ( Gpm_Close() );
}
#endif

void sighandler(int sig)
{

    switch (sig) {

    case SIGTERM:
        exit(1);
    case SIGHUP: /* Grrrr,.. some bastard killed out controlling TTY.. but since
		* we are so nice, fork shit and continue.. It's actually not that
		* fun when some wingay at work close their telnet when you have
		* loaded like 2000+ files and not saved them to a playlist.. */

        currloc = CAMP_UNDEF;  /* Yeah, right .. sucked into /dev/null */
        disappear();
        break;

    case SIGSEGV:
        printf("\e[0;1m***\n*mayday* *mayday* -- We got a incoming segfault!!\n");
        if ( config.forkseg ) {
            printf("fork() in 2 seconds..\e[0m\n");
            disappear();
            sleep(2);
        }
        sleep(2);
        exit(-1);
        break;
    }

    return;
}

void myexit(void)
{
    int i;
    FILE *fd=NULL;
    char buf[100], buf2[100];

#ifdef LIRCD
    if ( lircd_config ) lirc_freeconfig(lircd_config);
    if ( use_lircd ) lirc_deinit();
#endif


    signal(SIGCHLD, SIG_IGN);

    if ( quitmode == 0 ) {  /* 0 = normal, 1 = forking, 2 = "forked-steal" quit  3 = "steal" quit*/
        if ( playsong || config.mpg123 ) killslave();
        if ( config.defpl) {
            sprintf(buf, "%s/.camp/default.pl", getenv("HOME"));
            saveplaylist(playlist, buf, TRUE);
        }

    }

    if ( alwayslocked && quitmode != 2 ) {
        i = open("/dev/console",O_WRONLY);
        if ( i == -1 ) {
            printf("FAILED TO UNLOCK CONSOLE!!\n");
        } else {
            ioctl(i, VT_UNLOCKSWITCH);
        }
        close(i);
    }


    /* clean up some shit */
    unloadskin(&config.skin);
    for(i=0; i<15; i++) if (config.playerargv[i]) free(config.playerargv[i]);

    if ( playlist != NULL ) clearplaylist(&playlist);

#ifdef HAVE_TERMIOS_H
    tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
#else
    resetty();
#endif

    if ( quitmode != 2 && currloc != CAMP_UNDEF ) {
        printf("\e[0m\e[2J\e[1;1H%s", chromansi);
        printf("\e[0mConsole Ansi Mpeg3 Player interface %s.%d by inm (inm@sector7.nu)\n", CAMP_VERSION, BUILD);
        printf("\e[?25h");
    }

    switch ( quitmode ) {
    case 1:
        printf("Forked into the background.\n");
        break;
    case 3:
        sprintf(buf, "%s/info.camp", TMP_DIR);
        if ( exist(buf) ) {
            fd = fopen(buf, "rb");
            fgets(buf2, 100, fd);
            fclose(fd);
            printf("Session stolen by %s", buf2);
            unlink(buf);
        } else printf("Session stolen by unknown!\n");
        break;
    default:
        if ( !force ) unlink(PID_FILE);
        break;
    }
}

void unloadskin(struct skinconfig *skin)
{
    int i;

    for(i=0; i<6; i++)   if (skin->flistmsg[i]) free(skin->flistmsg[i]);
    for(i=0; i<4; i++)   if (skin->mainmsg[i]) free(skin->mainmsg[i]);
    for(i=0; i<3; i++)   if (skin->modetext[i]) free(skin->modetext[i]);
    for(i=0; i<2; i++)   if (skin->stereotext[i]) free(skin->stereotext[i]);

    for(i=FL_MINBUTTON; i<(FL_MAXBUTTON); i++)   if (skin->fi[i]) free(skin->fi[i]);
    for(i=FL_MINBUTTON; i<(FL_MAXBUTTON); i++)   if (skin->fa[i]) free(skin->fa[i]);
    for(i=PL_MINBUTTON; i<(PL_MAXBUTTON); i++)   if (skin->pi[i]) free(skin->pi[i]);
    for(i=PL_MINBUTTON; i<(PL_MAXBUTTON); i++)   if (skin->pa[i]) free(skin->pa[i]);
    for(i=MINBUTTON;   i<(MAXBUTTON); i++)      if (skin->mi[i]) free(skin->mi[i]);
    for(i=MINBUTTON;   i<(MAXBUTTON); i++)      if (skin->ma[i]) free(skin->ma[i]);

    if (skin->mtextc)      free(skin->mtextc);
    if (skin->textc)       free(skin->textc);
    if (skin->songnamec)   free(skin->songnamec);
    if (skin->sampleratec) free(skin->sampleratec);
    if (skin->bitratec)    free(skin->bitratec);
    if (skin->timec)       free(skin->timec);
    if (skin->stereoc)     free(skin->stereoc);
    if (skin->modetextc)   free(skin->modetextc);
    if (skin->songnumberc) free(skin->songnumberc);
    if (skin->flistcdi)    free(skin->flistcdi);
    if (skin->flistcda)    free(skin->flistcda);
    if (skin->flistcfi)    free(skin->flistcfi);
    if (skin->flistcfa)    free(skin->flistcfa);
    if (skin->flistcta)    free(skin->flistcta);
    if (skin->flistcti)    free(skin->flistcti);
    if (skin->plistci)     free(skin->plistci);
    if (skin->plistca)     free(skin->plistca);
    if (skin->plistcc)     free(skin->plistcc);
    if (skin->id3st)       free(skin->id3st);
    if (skin->id3fnc)      free(skin->id3fnc);
    if (skin->id3tc)       free(skin->id3tc);
    if (skin->id3sc)       free(skin->id3sc);
    if (skin->findbarc)    free(skin->findbarc);

    free(skin->main);
    free(skin->playlist);
    free(skin->filelist);
    free(skin->id3);
}

void escfix(void)
{
    int ch1, ch2;//, flags;

#ifdef HAVE_TERMIOS_H
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
#endif

    ch1 = getchar();
    if ( ch1 == -1 ) {
        mykbhit(0, 250000);
        ch1 = getchar();
    }
    ch2 = getchar();

#ifdef HAVE_TERMIOS_H
    fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#endif

    if ( ( ch1 == -1 || ch1 == 27 ) && canexit() ) exit(0);
    else if ( ch1 == '[' ) {
        if ( ch2 == 'C' ) updatebuttons(KM_RIGHT);
        else if ( ch2 == 'D' ) updatebuttons(KM_LEFT);
        else if ( ch2 == 'A' ) updatebuttons(KM_UP);
        else if ( ch2 == 'B' ) updatebuttons(KM_DOWN);
    }
}


uintptr_t dofunction(int forcedbutton)
{
//static struct timeval pause_start, pause_end;
    char pass[16], checkpass[16];
    int mod, ec, oldfilenumber;
    char *buf, *buf2;
//FILE *fd;

    if ( forcedbutton == -1 ) forcedbutton = buttonpos;

    switch( forcedbutton ) {

    case 0: /* skip back */
        if ( !playlist || pausesong ) return(0);
        if ( filenumber == 0 ) filenumber = playlistents-1;
        else
            filenumber--;
        if ( playsong == TRUE && !slavepid && !config.mpg123 ) killslave();
        else
            return(1);
        break;

    case 1: /* play */
        if ( !playlist ) return(0);
        if ( pausesong )  {
            kill(slavepid, SIGCONT);
            pausesong = FALSE;
            updatesongtime('e');
            return(0);
        }
        if ( config.playmode == 2 && playsong == TRUE ) filenumber = myrand(playlistents);
        if ( playsong == TRUE && slavepid != 0 && !config.mpg123 ) killslave();
        playsong = TRUE;
        return(1);

    case 2: /* skip fwd */
        if ( !playlist || pausesong ) return(0);
        if ( filenumber >= playlistents-1 ) filenumber = 0;
        else
            filenumber++;
        if ( playsong == TRUE && !slavepid && !config.mpg123 ) killslave();
        else
            return(1);
        break;

    case 3: /* stop */
        if ( playsong && slavepid ) {
            playsong = FALSE;
            if ( config.mpg123 )
                mpg123_control("STOP\n");
            else
                killslave();
        }
        updatedata();
        break;

    case 4: /*playmode */
        if ( config.playmode != 2 )
            config.playmode++;
        else config.playmode = 0;
        updatedata();
        break;

    case 5: /* playlist */
        if ( currloc == CAMP_PL ) break;
        if ( !config.skin.mainatpl ) quiet = TRUE;
        rplaylist(&playlist, &filenumber);
#ifdef LIRCD
        clear_lirc();
#endif
        playlistents = pl_count(playlist);
        if ( config.skin.mpclr ) printf("\e[0m\e[2J");
        printf("\e[1;1H%s", config.skin.main);
        if ( config.skin.platmain ) {
            pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
        }
        currloc = CAMP_MAIN;
        quiet = FALSE;
        updatesongtime('f');
        updatedata();
        updatebuttons(0);
        break;

    case 6: /* custom button */
        ec = currloc;
        currloc = CAMP_UNDEF;
        printf("\e[0m\e[2J\e[1;1H");
        // fflush(stdout);
#ifdef HAVE_TERMIOS_H
        tcsetattr(fileno(stdin), TCSANOW, &oldtermios);
#else
        resetty();
#endif

#ifdef USE_GPM_MOUSE
        while( Gpm_Close() );
#endif
        quiet = TRUE;
        my_system(CUSTOM_RUN);
        quiet = FALSE;

#ifdef USE_GPM_MOUSE
        my_Gpm_Init(&mouse);
#endif

#ifdef HAVE_TERMIOS_H
        tcsetattr(fileno(stdin), TCSANOW, &newtermios);
        fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#else
        savetty();
        noecho();
        nonl();
        cbreak();
#endif
        currloc = ec;
        printf("\e[?25l\e[0m\e[2J\e[1;1H%s", config.skin.main);
        updatebuttons(0);
        if ( config.skin.platmain ) {
            pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
        }
        updatedata();
        updatesongtime('f');
        fflush(stdout);
        break;

    case 7: /* rew (mpg123 mode only) */
        if ( config.mpg123 )
            (void*)mpg123_control("JUMP -150\n");
        updatesongtime('u');
        break;

    case 8: /* pause */
        if ( playsong ) {
            if ( !pausesong ) {
                kill(slavepid, SIGSTOP);
                pausesong = TRUE;
                updatesongtime('p');
            } else {
                kill(slavepid, SIGCONT);
                pausesong = FALSE;
                updatesongtime('e');
            }
        }
        return 0;

    case 9: /* ff (mpg123 mode only) */
        if ( config.mpg123 )
            (void*)mpg123_control("JUMP +150\n");
        updatesongtime('u');
        break;

    case 10: /* jump */
        if ( !playlistents ) break;
        quiet = TRUE;
        oldfilenumber = filenumber;
        pass[0] = 0;
        buf  = (char*)malloc(256);
        buf2 = (char*)malloc(256);
        printf("\e[?25h"); /* cursor on */
        sprintf(buf, "\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[3]);
        sprintf(buf2, "%s%d%u", config.skin.mainmsg[3], 0, playlistents);
        do {
            printf(buf, 1, playlistents);
            mod = 27;
            filenumber = atoi(readyxline(config.skin.mtexty, config.skin.mtextx+(ansi_strlen(buf2)-4), pass, 4, &ec, &mod));
            sprintf(pass, "%u", filenumber);
            if ( filenumber == 0 || ec == 27 || !mod ) {
                filenumber = -1;
                break;
            }
            filenumber--;
        } while( filenumber > playlistents-1 );
        free(buf);
        free(buf2);
        printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
        printf("\e[?25l"); /* cursor off */
        if ( filenumber != -1 ) call_player(pl_seek(filenumber, &playlist));
        else {
            filenumber = oldfilenumber;
        }
        quiet = FALSE;
        updatedata();
        updatebuttons(0);
        break;

    case 11: /* lock */
        printf("\e[?25h"); /* cursor on */
        printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[0]);
        readpass(pass, 15);
        if ( !pass[0] ) {
            printf("\e[?25l"); /* cursor off */
            break;
        }
        printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[1]);
        readpass(checkpass, 15);
        // fflush(stdout);
        if ( !strcmp(pass, checkpass) ) {
            printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
            updatedata();
            if ( config.lockvt && ! alwayslocked ) {
                ec = open("/dev/console",O_WRONLY);
                ioctl(ec, VT_LOCKSWITCH);
                close(ec);
            }
            do {
                printf("\e[?25l"); /* cursor off */
                while ( !mykbhit(1, 0) )
                    updatesongtime('u');
                checkpass[0] = getchar();
                printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[2]);
                printf("\e[?25h"); /* cursor on */
                // fflush(stdout);
                memset(checkpass, 0, 15);
                readpass(checkpass, 15);
                printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
                updatedata();
            } while ( strcmp(pass, checkpass) );
        }
        if ( config.lockvt && !alwayslocked ) {
            ec = open("/dev/console",O_WRONLY);
            ioctl(ec, VT_UNLOCKSWITCH);
            close(ec);
        }
        printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
        updatebuttons(0);
        updatedata();
        updatesongtime('f');
        printf("\e[?25l"); /* cursor off */
        break;

    case 12: /* desc edit */
        if ( !playlist ) return(0);
        quiet = TRUE;
        id3edit(pl_seek(filenumber, &playlist)->name, pl_seek(filenumber, &playlist));
#ifdef LIRCD
        clear_lirc();
#endif
        currloc = CAMP_MAIN;
        quiet = FALSE;
        if ( config.skin.miclr ) printf("\e[0m\e[2J");
        printf("\e[1;1H%s", config.skin.main);
        if ( config.skin.platmain ) pl_showents(pl_current-pl_screenmark, playlist, &filenumber);
        updatebuttons(0);
        updatedata();
        updatesongtime('f');
        break;

    case 13: /* fork() */
        if ( canexit() ) disappear();
        break;

    case 14: /* mute, new in 101 */
        if ( !muted ) muted=TRUE;
        else
            muted=FALSE;
        mute(muted);
        break;

#ifdef HAVE_SYS_SOUNDCARD_H
    case 15: /* vol - */
        set_volume(config.voldev, -config.volstep);
        break;

    case 16: /* vol + */
        set_volume(config.voldev, config.volstep);
        break;
#endif

    default:
        return(0);
    }
    return(0);
}

#ifdef USE_GPM_MOUSE
void main_domouse(Gpm_Event *event)
{
    int i;

    if ( !event ) {
        event = malloc(sizeof(Gpm_Event));
        Gpm_GetEvent(event);
    }

    /* middle button */
    if ( event->buttons & 2 && event->type & GPM_DOWN ) {
        printf("\e[0m\e[2J\e[1;1H%s", config.skin.main);
        updatedata();
        updatesongtime('f');
        updatebuttons(0);
        event->buttons = 255;
    } else

        /* left button */
        if ( event->buttons & 4 && event->type & GPM_DOWN ) {
            if ( event->y == config.skin.timey && event->x >= config.skin.timex && event->x < (config.skin.timex+5) ) {
                if ( config.timemode )
                    config.timemode = FALSE;
                else
                    config.timemode = TRUE;
                updatesongtime('f');
            } else if ( event->y == config.skin.modetexty && event->x >= config.skin.modetextx && event->x < (config.skin.modetextx+config.skin.modetextw) ) {
                if ( config.playmode != 2 )
                    config.playmode++;
                else config.playmode = 0;
                updatedata();
            } else
                for(i=MINBUTTON; i<(MAXBUTTON+1); i++)
                    if ( event->y == config.skin.my[i] && event->x >= (config.skin.mx[i]-config.skin.mouseexpand) && event->x < (config.skin.mx[i]+config.skin.mw[i]+config.skin.mouseexpand) ) {
                        buttonpos = (char)i;
                        updatebuttons(0);
                        if ( dofunction(-1) )
                            call_player(pl_seek(filenumber, &playlist));
                        break;
                    }
        }

    if ( config.skin.platmain && currloc == CAMP_MAIN )
        pl_domouse(&playlist, &filenumber, event);

    if ( currloc == CAMP_MAIN && event ) free(event);

}
#endif

#ifdef LIRCD
void dolircd(char atmain)
{
    char *ir=NULL, *c=NULL;
    int ret;


    while ( !lirc_nextcode(&ir) && ir ) {
        while  ( !(ret=lirc_code2char(lircd_config,ir,&c)) && c )  {
            if ( !strcasecmp(c, "skip-") )    if ( dofunction(0) ) call_player(pl_seek(filenumber, &playlist));
            if ( !strcasecmp(c, "play") )     if ( dofunction(1) ) call_player(pl_seek(filenumber, &playlist));
            if ( !strcasecmp(c, "skip+") )    if ( dofunction(2) ) call_player(pl_seek(filenumber, &playlist));

            if ( !strcasecmp(c, "stop") )     dofunction(3);
            if ( !strcasecmp(c, "seek-") )     dofunction(7);
            if ( !strcasecmp(c, "seek+") )     dofunction(9);

            if ( atmain ) {
                if ( !strcasecmp(c, "playmode") ) dofunction(4);
                if ( !strcasecmp(c, "playlist") ) {
                    dofunction(5);
                    return;
                }
                if ( !strcasecmp(c, "custom") ) {
                    dofunction(6);
                    clear_lirc();
                    return;
                }
                if ( !strcasecmp(c, "pause") )    dofunction(8);
                if ( !strcasecmp(c, "jump") ) {
                    dofunction(10);
                    return;
                }
                if ( !strcasecmp(c, "fork") )     dofunction(13);
            }

            if ( !strcasecmp(c, "mute") )        dofunction(14);
# ifdef HAVE_SYS_SOUNDCARD_H
            if ( !strcasecmp(c, "vol+") ) set_volume(config.voldev, config.volstep);
            if ( !strcasecmp(c, "vol-") ) set_volume(config.voldev, -config.volstep);
# endif
        }
        free(ir);
        if ( ret == -1 ) break;
    }
}

void clear_lirc(void)
{
    fd_set fds;
    struct timeval currenttime;
    char *ir;

    if ( !use_lircd ) return;

    currenttime.tv_usec = currenttime.tv_sec  = 0;
    FD_ZERO(&fds);
    FD_SET(lirc_lircd, &fds);

    while ( select(lirc_lircd+1, &fds, NULL, NULL, &currenttime) > 0 ) {
        if ( !lirc_nextcode(&ir)  ) free(ir);
    }


}
#endif

void updatedata()
{
//struct timeval currenttime;
//int l, r;

    if ( quiet ) return;

    if ( currentfile.showname[0] != 0 ) {
        if ( config.skin.sampleratey + config.skin.sampleratex > 0 ) printf("\e[%sm\e[%d;%dH%5u", config.skin.sampleratec, config.skin.sampleratey, config.skin.sampleratex, currentfile.samplerate);
        if ( config.skin.bitratey + config.skin.bitratex > 0 ) printf("\e[%sm\e[%d;%dH%3u", config.skin.bitratec, config.skin.bitratey, config.skin.bitratex, currentfile.bitrate);
        if ( config.skin.stereoy + config.skin.stereox > 0 ) printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.stereoc, config.skin.stereoy, config.skin.stereox, xys(config.skin.stereow, ' '), config.skin.stereoy, config.skin.stereox, config.skin.stereotext[currentfile.mode]);
        if ( config.skin.songnumbery + config.skin.songnumberx > 0 ) printf("\e[%sm\e[%d;%dH%4u\e[%d;%dH%-4u", config.skin.songnumberc, config.skin.songnumbery, config.skin.songnumberx, filenumber+1, config.skin.songnumbery, config.skin.songnumberx+5, playlistents);
    }
    updatesongtime('u');
    printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.modetextc, config.skin.modetexty, config.skin.modetextx, xys(config.skin.modetextw, ' '), config.skin.modetexty, config.skin.modetextx, config.skin.modetext[(int)config.playmode]);
#ifdef HAVE_SYS_SOUNDCARD_H
    set_volume(config.voldev, 0);
#else
    printf("\e[%d;%dH\e[%sm--", config.skin.voly, config.skin.volx, config.skin.volc);
#endif
    fflush(stdout);

}

void updatesongtime(char ch)
{
    static struct timeval starttime, pause_start, pause_end, oldcurrenttime;
    struct timeval currenttime;
    int reverse_time;
    FILE *fd;
    char buf[256], scrll = 1;

    if ( quiet && ( ch == 'u' || ch == 'f' ) ) return;

    if ( ch == 'f' ) scrll = 3;

    switch(ch) {

    case 'i':
        gettimeofday(&starttime, NULL);
        break;

    case 'u':
    case 'f':
        if ( config.mpg123 ) { /* dirty fix */
            if ( playsong ) {
                currentfile.length = currentfile.played + currentfile.left;
                currenttime.tv_sec = currentfile.played;
            } else currenttime.tv_sec = currentfile.played;
            starttime.tv_sec   = 0;
        } else
            gettimeofday(&currenttime, NULL);

        if ( ch == 'u' && oldcurrenttime.tv_sec == currenttime.tv_sec ) return;
        oldcurrenttime.tv_sec = currenttime.tv_sec;

        if ( !playsong ) {
            if ( !config.timemode )
                printf("\e[%sm\e[%d;%dH00\e[%d;%d00", config.skin.timec, config.skin.timey, config.skin.timex, config.skin.timey, config.skin.timex+3);
            else
                printf("\e[%sm\e[%d;%dH%02lu\e[%d;%dH%02lu", config.skin.timec, config.skin.timey, config.skin.timex, currentfile.length/60, config.skin.timey, config.skin.timex+3, currentfile.length%60);
            if ( ( config.showtime == 3 || config.showtime == 1 ) && strncasecmp(playlist->showname, "http://", 7) )
                printf("\e[%sm\e[%d;%dH%s\e[%d;%dH[%02lu:%02lu] %s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, currentfile.length / 60, currentfile.length % 60, scrollsongname(scrll));
            else
                printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, scrollsongname(scrll));
            fflush(stdout);
            return;
        }

        if ( ( config.showtime == 3 || config.showtime == 1 ) && strncasecmp(playlist->showname, "http://", 7) ) printf("\e[%sm\e[%d;%dH%s\e[%d;%dH[%02lu:%02lu] %s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, currentfile.length / 60, currentfile.length % 60, scrollsongname(scrll));
        else
            printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s\n", config.skin.songnamec, config.skin.songnamey, config.skin.songnamex, xys(config.skin.songnamew, ' '), config.skin.songnamey, config.skin.songnamex, scrollsongname(scrll));
        /* guess this is the best way of doing this, since we don't want the player to type this,. */
        if ( pausesong ) return;
        if ( config.timemode ) {
            reverse_time = currentfile.length-(currenttime.tv_sec-starttime.tv_sec);
            if ( reverse_time < 0 ) {
                reverse_time = 0;
                printf("\e[%sm\e[%d;%dH--\e[%d;%dH--", config.skin.timec, config.skin.timey, config.skin.timex, config.skin.timey, config.skin.timex+3);
            } else
                printf("\e[%sm\e[%d;%dH%02d\e[%d;%dH%02d", config.skin.timec, config.skin.timey, config.skin.timex, reverse_time/60, config.skin.timey, config.skin.timex+3, reverse_time%60);

        } else
            printf("\e[%sm\e[%d;%dH%02ld\e[%d;%dH%02ld", config.skin.timec, config.skin.timey, config.skin.timex, (currenttime.tv_sec-starttime.tv_sec)/60, config.skin.timey, config.skin.timex+3, (currenttime.tv_sec-starttime.tv_sec)%60);
        updatedata();
        fflush(stdout);
        break;

    case 'e': /* pause end */
        if ( !config.mpg123 ) {
            gettimeofday(&pause_end, NULL);
            starttime.tv_sec += pause_end.tv_sec - pause_start.tv_sec;
        }
        break;

    case 'p': /* pause start */
        if ( !config.mpg123 ) {
            gettimeofday(&pause_start, NULL);
        }
        break;

    case 'w':
        sprintf(buf, "%s/time.camp", TMP_DIR);
        fd = fopen(buf, "w");
        fprintf(fd, "%ld\n%ld\n", starttime.tv_sec, starttime.tv_usec);
        fclose(fd);
        break;

    case 'r':
        sprintf(buf, "%s/time.camp", TMP_DIR);
        fd = fopen(buf, "r");
        if ( !fd ) break;
        fscanf(fd, "%ld\n%ld\n", &starttime.tv_sec, &starttime.tv_usec);
        fclose(fd);
        unlink(buf);
        break;
    }

}


char *scrollsongname(char action)
{
    static char scrolledname[100];
    static int i = 0, wichway = 1;

    if ( action == 0 ) {
        i = -1;
        wichway = 1; /* -> */
        return NULL;
    }

    if ( action == 2 ) { /* scroll back to beginning, then return NULL (unused) */
        if ( i == 0 ) return NULL;
        i--;
        strncpy(scrolledname, currentfile.showname+i, config.skin.songnamew);
        return scrolledname;
    }

    if ( action == 1 ) {
        if ( config.scrollsn && strlen(currentfile.showname) > (config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8) ) {
            if ( i+ (config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8) == strlen(currentfile.showname) && wichway == 1 )
                wichway = -1;
            else /* <- */
                if ( i == 0 && wichway == -1 ) wichway = 1; /* -> */
            i += wichway;
        } else
            i = 0;
    }

    /* action == 3, only print! */

//    if ( action == 3 ) {
        if ( i == -1 )
            strncpy(scrolledname, currentfile.showname, config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8 );
        else
            strncpy(scrolledname, currentfile.showname+i, config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8 );
//    }

//    strncpy(scrolledname, currentfile.showname+i, config.showtime == 2 ? config.skin.songnamew : config.skin.songnamew-8 );

    return scrolledname;
}

int canexit(void)
{
    char checkpasswd[16];

    if ( !alwayslocked )        return TRUE;  /* Yep, not locked at all, go ahead */
    if ( currloc != CAMP_MAIN ) return FALSE; /* Can only exit at main in lockmode */
    quiet = TRUE;
    printf("\e[%sm\e[%d;%dH%s\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '), config.skin.mtexty, config.skin.mtextx, config.skin.mainmsg[2]);
    printf("\e[?25h"); /* cursor on */
    // fflush(stdout);
    memset(checkpasswd, 0, 16);
    readpass(checkpasswd, 15);
    printf("\e[%sm\e[%d;%dH%s", config.skin.mtextc, config.skin.mtexty, config.skin.mtextx, xys(config.skin.mtextw, ' '));
    printf("\e[?25l"); /* cursor off */
    quiet = FALSE;
    if ( !strcmp(passwd, checkpasswd) ) return TRUE;
    else
        return FALSE;

}


int showtip(void)
{
    char buf[501];
    int i=0, j;
    FILE *fd = NULL;

    sprintf(buf, "%s/.camp/tip.txt", getenv("HOME"));
    if ( !exist(buf) ) return FALSE;
    fd = fopen(buf, "rb");

    while ( fgets(buf, 500, fd) != NULL )
        i++; // Count lines ..
    j = myrand(i);
    fseek(fd, 0, SEEK_SET);
    i = 0;
    while ( i != j ) {
        fgets(buf, 500, fd);
        i++;
    }
    fclose(fd);
    printf("\e[1mTip of the day:\n%s\e[0m", buf);
    return TRUE;
}

