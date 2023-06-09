#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "camp.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HAVE_TERMIOS_H
# include <fcntl.h>
#endif

#ifdef USE_GPM_MOUSE
#include <gpm.h>
#endif

#ifdef LIRCD
#include "lirc_client.h"
extern char use_lircd;
extern int lirc_lircd;
extern struct lirc_config *lircd_config;
#endif

extern struct configstruct config;

unsigned int ansi_strlen(char *text)
{
    unsigned int i, len=0; /* returns length of a string with ansi codes stripped off */

    for (i = 0; text[i]; i++)
        if ( text[i] == '\e' ) {
            for (i++; text[i] && (text[i] < 'A' || text[i] > 'Z') && (text[i] < 'a' || text[i] > 'z'); i++);
            if( !text[i] ) break;
        }
        else
            len++;
    return len;
}


char *readyxline(char y, char x, char *preval, unsigned char maxlen, int *exitchar, int *modified)
{
    fd_set fds;
    int ch;
    unsigned char maxpos=0, pos=0;
    static char buf[256];
    char buf2[256];
//char *ir, *c;
    struct timeval counter;

    if ( preval != NULL ) {
        strcpy(buf, preval);
        maxpos = pos = strlen(buf);
    } else
        buf[0] = 0;

    printf("\e[%d;%dH%s", y, x, buf);
    fflush(stdout);

    do {

        if ( config.mpg123 )
            mpg123_control(NULL);

#ifdef RC_ENABLED
        if ( config.userc ) checkrc();
#endif
        counter.tv_usec = config.rctime;
        counter.tv_sec  = 0;
        ch = 0;
        FD_ZERO(&fds);
        FD_SET(fileno(stdin), &fds);
#ifdef LIRCD
        if ( use_lircd ) {
            FD_SET(lirc_lircd, &fds);
            if ( select(lirc_lircd+1, &fds, NULL, NULL, &counter) != -1 ) {
                if ( FD_ISSET(lirc_lircd, &fds) ) dolircd(0);

                /*{ // Hmm, this was the old code to wander around in the fields with the remote-control, ripped it out :)
                     ir = lirc_nextir();
                     while ( ir && (c=lirc_ir2char(lircd_config,ir)) != NULL )

                if ( strlen(c) == 1 ) ch = c[0]; else
                if ( !strcasecmp(c, "play") || !strcasecmp(c, "jump") ) ch = 13; else
                if ( !strcasecmp(c, "skip-") ) ch = 127; else
                if ( !strcasecmp(c, "stop") ) ch = 27;
                     if ( ir ) free(ir);
                  } */

                else if (FD_ISSET(fileno(stdin), &fds)) ch = getchar();
            }
        } else if ( select(fileno(stdin)+1, &fds, NULL, NULL, &counter) != -1 )
            if (FD_ISSET(fileno(stdin), &fds)) ch = getchar();
#else
        if ( select(fileno(stdin)+1, &fds, NULL, NULL, &counter) != -1 )
            if (FD_ISSET(fileno(stdin), &fds)) ch = getchar();
#endif
        if ( ch == 3 && canexit() ) exit(0); /* ^C */
        if ( ch == 26 && canexit() ) disappear();
        else /* ^Z */

            if ( ch == 1 ) { /* ^A */
                pos = 0;
                printf("\e[%d;%dH", y, x+pos);
            } else if ( ch == 5 ) { /* ^E */
                pos = maxpos;
                printf("\e[%d;%dH", y, x+pos);
            } else if ( ch == 21 ) { /* ^U */
                printf("\e[%d;%dH%s\e[%d;%dH", y, x, xys(strlen(buf), ' '), y, x);
                maxpos = pos = 0;
                buf[0] = 0;
                if ( modified != NULL ) *modified = TRUE;
            } else if ( ch > 31 && maxpos < maxlen && ch != 127 ) { /* regulars */
                strcpy(buf2, &buf[pos]);
                buf[pos] = ch;
                strcpy(&buf[pos+1], buf2);
                maxpos++;
                pos++;
                printf("\e[%d;%dH%s\e[%d;%dH", y, x, buf, y, x+pos);
                if ( modified != NULL ) *modified = TRUE;
            } else if (ch == 127 && maxpos != 0 && pos != 0 ) { /* backspace */
                strcpy(buf2, &buf[pos]);
                strcpy(&buf[pos-1], buf2);
                maxpos--;
                pos--;
                printf("\e[%d;%dH%s \e[%d;%dH", y, x, buf, y, x+pos);
                if ( modified != NULL ) *modified = TRUE;
            } else if ( ch == 27 ) {
#ifdef HAVE_TERMIOS_H
                fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
#endif
                ch = getchar();
                if ( ch == '[' ) {
                    switch( getchar() ) {
                    case 'A':
                        *exitchar = 'A';
#ifdef HAVE_TERMIOS_H
                        fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#endif
                        return buf;
                    case 'B':
                        *exitchar = 'B';
#ifdef HAVE_TERMIOS_H
                        fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
#endif
                        return buf;
                    case 'C':
                        if ( pos != maxpos ) pos++;
                        printf("\e[%d;%dH", y, x+pos);
                        break;
                    case 'D':
                        if ( pos != 0 ) pos--;
                        printf("\e[%d;%dH", y, x+pos);
                        break;
                    default:
                        ;
                    }
                    fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
                } else {
                    while ( getchar() != -1 ) ;
                    *exitchar = 27;
                    fcntl(fileno(stdin), F_SETFL, !O_NONBLOCK);
                    return buf;
                }
            } else if ( ch == 13 ) {
                *exitchar = 13;
                return buf;
            }

    } while ( TRUE );

    return buf;
}


void readpass(char *text, int len)
{
    char tmp[100];
    int ch=-1;
    struct timeval starttime, currenttime;

    gettimeofday(&starttime, NULL);

    tmp[0] = 0;
    while ( ch != 13 ) {

        if ( config.mpg123 )
            mpg123_control(NULL);

        gettimeofday(&currenttime, NULL);
        if ( currenttime.tv_sec == starttime.tv_sec+10 ) {
            text[0] = 0;
            return;
        }
        if ( mykbhit(0, 900000) ) {
            ch = getchar();
            gettimeofday(&starttime, NULL);

            switch(ch) {

            case 127:
                if ( tmp[0] != 0 ) {
                    tmp[strlen(tmp)-1] = 0;
                    printf("\b \b");
                }
                break;

            case 13:
                break;

            default:
                if ( strlen(tmp) == len ) break;
                tmp[strlen(tmp)+1] = 0;
                tmp[strlen(tmp)] = ch;
                printf("*");
                fflush(stdout);
                break;
            } // switch(ch)

        } // if kbhit ..
    } // while ch ! 13

    strcpy(text, tmp);
} // readpass(char *text)


int exist(char *fname)
{
    FILE *fd;
    fd = fopen(fname, "rb");
    if ( fd != NULL ) {
        fclose(fd);
        return(1); /* file exists! */
    } else return(0); /* file does not exist! */
}

unsigned int myrand(double maxval)
{
    struct timeval tv;
    int i;

    gettimeofday(&tv, NULL);
    srand(tv.tv_sec+tv.tv_usec);
    i=(int) (maxval*rand()/(RAND_MAX+1.0));
    return(i);
}

void lowcases( char *text )
{
    unsigned int i;

    for (i = 0; text[i]; i++)
        text[i] = tolower(text[i]);
}

unsigned char mykbhit(unsigned int sec, unsigned long usec)
{
    struct timeval tv;
    fd_set fds;

    tv.tv_sec = sec;
    tv.tv_usec = usec;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    if ( select(fileno(stdin)+1, &fds, NULL, NULL, &tv) != -1 && FD_ISSET(fileno(stdin), &fds) ) return TRUE;
    else
        return FALSE;
}

int strchrpos(const char *text, int ch, int what)
{
    int i, wht=0;

    for (i = 0; text[i]; i++)
        if ( text[i] == ch ) {
            wht++;
            if ( what == wht ) return i;
        }
    return -1;
}

char *strtrim(char *text, char trimchar)
{

    while (text[0] == trimchar || text[strlen(text)-1] == trimchar) {
        if ( text[0] == trimchar ) strcpy(text, (char*)text+1);
        if ( text[strlen(text)-1] == trimchar ) text[strlen(text)-1] = '\000';
    }
    return text;
}

char *replace(char *text, char oldc, char newc)
{
    unsigned int i;

    for (i = 0; text[i]; i++)
        if ( text[i] == oldc ) text[i] = newc;

    return text;
}

void cpy_replace(char *dst, const char *src, char oldc, char newc)
{
    unsigned int i;

    for (i = 0; src[i]; i++)
        if ( src[i] == oldc )
            dst[i] = newc;
        else
            dst[i] = src[i];

    dst[i] = 0;
}

char *str_strip_end(char *text, unsigned int maxlen)
{
    unsigned int i = 0;

    while (i < maxlen && text[i]) i++;
    if(i) {
        i--;
        while (i && (IS_CRLF(text[i]) || IS_SPACE(text[i]))) i--;
        if (!IS_CRLF(text[i]) && !IS_SPACE(text[i])) i++;
    }

    text[i] = 0;
    return text;
}

void cpy_strip_end(char *dst, const char *src, unsigned int maxlen)
{
    unsigned int i;

    for (i = 0; i < maxlen && src[i]; i++)
        dst[i] = src[i];
    if(i) {
        i--;
        while (i && (IS_CRLF(dst[i]) || IS_SPACE(dst[i]))) i--;
        if (!IS_CRLF(dst[i]) && !IS_SPACE(dst[i])) i++;
    }

    dst[i] = 0;
}

#ifdef HAVE_TERMIOS_H
void termios_raw(struct termios *termios_p)
{
    termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    termios_p->c_oflag &= ~(OPOST|CRTSCTS|PARENB);
    termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    termios_p->c_cflag &= ~(CSIZE|PARENB);
    termios_p->c_cflag |= CS8;
}
#endif

int my_system (char *command)   /* ripped out of system's manpage */
{
    int pid, status;

    if ( !command ) return 1;
    pid = fork();
    if ( pid == -1 ) return -1;
    if ( !pid ) {
        char *argv[4];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = 0;
        execve("/bin/sh", argv, environ);
        exit(127);
    }
    do {
        if (waitpid(pid, &status, 0) == -1) {
            if (errno != EINTR)
                return -1;
        } else
            return status;
    } while(1);
}


#ifdef USE_GPM_MOUSE
void my_Gpm_Init(Gpm_Connect *mouse)
{
    mouse->eventMask = GPM_DOWN|GPM_SINGLE|GPM_DOUBLE|GPM_DRAG;
    mouse->defaultMask = GPM_MOVE; /* Let GPM handle cursor drawing */
    mouse->maxMod = 0;
    mouse->minMod = 0;

    gpm_zerobased      = 0;
    gpm_visiblepointer = 1;
    if (Gpm_Open(mouse, 0)!=-2)  {
        gpm_handler = NULL;
        if ( !gpm_flag ) {
            while ( Gpm_Close() != 0 ) ;
            gpm_fd = -1;
        }
    } else {
        gpm_fd=-1;
        gpm_flag=0;
    }
}

void my_Gpm_Purge(void)
{
    fd_set    gpm_fds;
    Gpm_Event event;
    struct timeval tv;

    if ( gpm_fd == -1 ) return;
    FD_ZERO(&gpm_fds);
    FD_SET(gpm_fd, &gpm_fds);
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    while ( select(gpm_fd+1, &gpm_fds, NULL, NULL, &tv) )
        Gpm_GetEvent(&event);
}
#endif

