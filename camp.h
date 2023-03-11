#ifndef __camp_h
# define __camp_h

#pragma GCC diagnostic ignored "-Wstringop-truncation"
#pragma GCC diagnostic ignored "-Wformat-truncation"

#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include <stdint.h>
#include "build.h"

#ifdef USE_GPM_MOUSE
# include <gpm.h>
#endif

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

#define CAMP_VERSION "1.7"

#define    MINBUTTON 0
#define    MAXBUTTON 16
#define PL_MINBUTTON 0
#define PL_MAXBUTTON 6
#define FL_MINBUTTON 0
#define FL_MAXBUTTON 7
#define FL_BUTTONQUIT 6

#define TMP_DIR "/tmp"
#define PID_FILE "/tmp/.camp.pid"

#define NICE_NAMES

#ifndef CUSTOM_RUN
# define CUSTOM_RUN    "/usr/local/bin/aumix" /* program to run when CUSTOM is pressed */
#endif

#define KEY_HOME 0
#define KEY_END  1
#define KEY_PGUP 2
#define KEY_PGDN 3

/* permanent defines to know where we are right now */
#define CAMP_MAIN  0
#define CAMP_PL    1
#define CAMP_FL    2
#define CAMP_DESC  3
#define CAMP_RET   4
#define CAMP_UNDEF 5 /* Undefined.. like external programs and crap */

#define CONF_BUF_LEN 256
#define IS_SPACE(chr_) ((chr_) == ' ' || (chr_) == '\t')
#define IS_CRLF(chr_) ((chr_) == '\r' || (chr_) == '\n')

#define ID3_PAD_CH 0   /* was ' ' for some reason */

/* structures */
struct filelistent {
    char name[256];
    struct filelistent *prev;
    struct filelistent *next;
    unsigned int number;
    unsigned char tagged;
    unsigned char type; /* 1=file, 2=dir */
};

struct playlistent {
    char name[256];
    char showname[100];
    unsigned int samplerate;
    unsigned int bitrate;
    unsigned char mode;
    unsigned long length;
    unsigned int number;
    struct playlistent *prev;
    struct playlistent *next;
};

struct oneplaylistent {
    char name[256];
    char showname[100];
    unsigned int samplerate;
    unsigned int bitrate;
    unsigned char mode;
    unsigned long length;
};

struct currentplaylistent {
    char name[256];
    char showname[100];
    unsigned int samplerate;
    unsigned int bitrate;
    unsigned char mode;
    unsigned long length;
    unsigned long played;
    unsigned long left;
    unsigned long frame;
};

struct ID3 {
    char tag[3];
    char songname[30];
    char artist[30];
    char album[30];
    char year[4];
    char misc[30];
    unsigned char genre;
};

struct usableID3 {
    char songname[31];
    char artist[31];
    char album[31];
    char year[5];
    char misc[31];
};

struct skinconfig {
    char mpclr, miclr, piclr, pclr, fclr, iclr;
    char mx[MAXBUTTON+1], my[MAXBUTTON+1], mw[MAXBUTTON+1], *ma[MAXBUTTON+1], *mi[MAXBUTTON+1], mh[MAXBUTTON+1];
    signed int mju[MAXBUTTON+1], mjd[MAXBUTTON+1], mjl[MAXBUTTON+1], mjr[MAXBUTTON+1];
    char px[PL_MAXBUTTON+1], py[PL_MAXBUTTON+1], pw[PL_MAXBUTTON+1], *pa[PL_MAXBUTTON+1], *pi[PL_MAXBUTTON+1];
    char fx[FL_MAXBUTTON+1], fy[FL_MAXBUTTON+1], fw[FL_MAXBUTTON+1], *fa[FL_MAXBUTTON+1], *fi[FL_MAXBUTTON+1];
    char ix[7], iy[7], *modetext[3], *stereotext[2];
    char flistbo[FL_MAXBUTTON+1], plistbo[PL_MAXBUTTON+1], msb, psb, fsb;
    char findbarw, findbarx, findbary, *findbarc;
    char mtextx, mtexty, mtextw, *mtextc, textx, texty, textw, *textc;
    char *flistmsg[6], *mainmsg[4];
    char id3fnw, *id3fnc, *id3tc, *id3sc, *id3st;
    char plistx, plisty, plistw, plistlines;
    char *plistci, *plistca, *plistcc;
    char flistx, flisty, flistw, flistlines;
    char *flistcdi, *flistcda, *flistcfi, *flistcfa, *flistcta, *flistcti;
    char mouseexpand, pmouseexpand, fmouseexpand;
    char standardrows;
    char songnamey, songnamex, modetexty, modetextx, modetextw, bitratex, bitratey;
    char timey, timex, stereox, stereoy, stereow, songnamew, sampleratex, sampleratey;
    char songnumberx, songnumbery, volx, voly;
    char platmain, id3atmain;
    char mainatpl, id3atpl;
    char *songnamec, *sampleratec, *bitratec, *timec, *stereoc, *volc;
    char *modetextc, *songnumberc;
    char *main;
    char *playlist;
    char *filelist;
    char *id3;
};

struct rcconfig {
    unsigned char play, pause, stop, skipb, skipf, seekb, seekf, volinc, voldec;
    unsigned int port;
};

struct configstruct {
    char playerpath[100];
    char playername[100];
    char *playerargv[15];
    char quiet[100];
    char rate[100];
    char device[100];
    char downmix[100];
    char showtime; /* 0 = none, 1 = main, 2 = playlist, 3 = both */
    char timemode; /* 0 = Show elapsed time, 1 = Show remaining time */
    char useid3;
    char rescanid3;
    char hidedot;
    char playmode; /* 0 = normal, 1 = loop, 2 = randOm */
    char dontreopen;
    char startincwd;
    char ttymode;
    char volstep;
    char voldev;
    char mutevol;
    char userc;
    char compresspl;
    char scrollsn;
    char kill2pids;
    char nicekill;
    char lockvt;
    char showtip;
    char forkseg;
    char mpg123;
    char mpg123fastjump;
    char defpl;
    char dropinfo;
    char dropfile[100];
    int playerprio;
    unsigned int rctime;
    unsigned int bufferdelay;
    struct skinconfig skin;
    struct rcconfig rc;
};

#ifdef USE_GPM_MOUSE /* if we're using the mouse, declare additional proc/func's */
void main_domouse(Gpm_Event *revent);
void pl_domouse(struct playlistent **playlist, unsigned int *filenumber, Gpm_Event *revent);
void my_Gpm_Init(struct Gpm_Connect *mouse);
void my_Gpm_Purge(void);
int fl_domouse(struct filelistent **filelist, struct playlistent **playlist);
void close_gpm(void);
#endif

#ifdef LIRCD
void dolircd(char atmain);
void clear_lirc(void);
unsigned char pl_dolircd(struct playlistent **playlist, unsigned int *filenumber);
#endif

/* main.o */
char *scrollsongname(char action);
void myexit(void);
void myinit(void);
void escfix(void);
void updatebuttons(char motion);
void updatedata(void);
void updatesongtime(char ch);
void unloadskin(struct skinconfig *skin);
void sighandler(int sig);
uintptr_t dofunction(int forcedbutton);
int  showtip(void);
int canexit(void);

/* playlist.o */
unsigned int pl_count( struct playlistent *playlist );
struct playlistent *pl_seek( unsigned int pos, struct playlistent **playlist );
void pl_search(char ch, struct playlistent *playlist, unsigned int *);
void pl_showents( int startpos, struct playlistent *playlist, const unsigned int *);
void pl_updatebuttons(int add);
void l_status(char *text);
void clearplaylist(struct playlistent **playlist);
void pl_dofunction(struct playlistent **playlist, unsigned int *filenumber, int forcedbutton);
void addfiletolist(struct playlistent **playlist, char *filename, const char *showname, unsigned int bitrate, unsigned int samplerate, unsigned char mode, char scanid3 );
void rplaylist(struct playlistent **playlist, unsigned int *filenumber);
void sortplaylist(struct playlistent **playlist);


/* filelist.o */
void recurseplaylist (struct filelistent *filelist, struct playlistent **playlist, char *dir);
void fl_search(char ch, struct filelistent *filelist);
void fl_showents( int startpos, struct filelistent *filelist );
void fl_updatebuttons( int add );
void releasedir( struct filelistent *filelist );
void togglemark( struct filelistent *filelist, char movedown );
void saveplaylist(struct playlistent *playlist, char *filename, char directsave);
void loadplaylist(struct playlistent **playlist, char *filename, char filemanager );
void fl_dofunction( struct filelistent *filelist, struct playlistent **playlist );
void getfiles( struct playlistent **playlist );
struct filelistent *camp_chdir( struct filelistent *filelist );
struct filelistent *loaddir( );
struct filelistent *file_seek( int pos, struct filelistent *filelist );
struct filelistent *sortfilelist( struct filelistent *filelist );

/* id3.o */
struct ID3 usable2id3(struct usableID3 *usabletag);
struct usableID3 id32usable(struct ID3 *tag);
void id3edit(char *filename, struct playlistent *playlist);
char getmp3info(char *filename, unsigned char *mode, unsigned int *sample_rate, unsigned int *bitrate, char *name, char *artist, char *misc, char *album, char *year, unsigned char *genre);
char writemp3info(char *filename, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre);

/* fork.o */
void disappear(void);
void sigusr1(int signr);
void stealback(void);
void killcamp(void);

/* player.o */
void call_player(struct playlistent *pl);
void slave(char *filename);
void killslave(void);
void playnext(int);
char *mpg123_control(char *command); /* 1t'z the n3w sh1t */


/* cconfig.o */
struct configstruct getconfig(char *configfile);
void loadskin(char *name, struct configstruct *config);
int parseconfig(const char *str, char **arg, char **value);
char is_val_true(char *value);
char *randomskin(char *rbuf);
int probe_dir(char *dir);

/* misc.o */
int exist(char *fname);
int ansi_strlen(char *string);
char *readyxline(char y, char x, char *preval, unsigned char maxlen, int *exitchar, int *modified);
char *strtrim(char *text, char trimchar);
char *replace(char *text, char oldc, char newc);
void cpy_replace(char *dst, const char *src, char oldc, char newc);
void cpy_strip_end(char *dst, const char *src, unsigned int maxpos);
char *xys(unsigned char number, char ch);
char *lowercases(char *str);
void lowcases( char *strng );
void readpass(char *text, int len);
unsigned int  myrand(double maxval);
unsigned char mykbhit(unsigned int sec, unsigned long usec);
int strchrpos(const char *text, int ch, int what);
int my_system (char *command);
#ifdef HAVE_TERMIOS_H
void termios_raw(struct termios *termios_p);
#endif

/* rc.o */
unsigned char rcpressed(void);
void checkrc(void);
//int inb (short port);

/* mixer.o */
void set_volume(int dev, int change);
void get_volume(int dev, int *left, int *right);
void mute(unsigned char mute);

#endif
