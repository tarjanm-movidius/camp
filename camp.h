#ifndef __camp_h
# define __camp_h

#ifdef USE_TERMIOS
# include <termios.h>
#endif
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

#define CAMP_VERSION "1.2"

#define    MINBUTTON 1
#define    MAXBUTTON 14
#define PL_MINBUTTON 1
#define PL_MAXBUTTON 7
#define FL_MINBUTTON 1
#define FL_MAXBUTTON 7

#define PID_FILE "/tmp/.camp.pid"
#define TMP_DIR "/tmp"

#define NICE_NAMES

#ifndef CUSTOM_BUTTON
# define CUSTOM_BUTTON "mixr"                 /* must be 4 chars exactly */
# define CUSTOM_RUN    "/usr/local/bin/aumix" /* program to run when CUSTOM is pressed */
#endif                       

#define KEY_HOME 0
#define KEY_END  1
#define KEY_PGUP 2
#define KEY_PGDN 3

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
   char hidedot;
   char playmode; /* 0 = normal, 1 = loop, 2 = randOm */
   char dontreopen;
   char startincwd;
   char ttymode;
};

#ifdef USE_GPM_MOUSE /* if we're using the mouse, declare additional proc/func's */
void main_domouse(void);
struct playlistent *pl_domouse(struct playlistent *playlist, unsigned int *filenumber);
struct filelistent *fl_domouse(struct filelistent *filelist, struct playlistent *playlist);
void my_Gpm_Init(struct Gpm_Connect *mouse);
void my_Gpm_Purge(void);
#endif

/* main.o */
void myexit(void);
void myinit(void);
void escfix(void);
void updatebuttons(int add);
void updatedata(void);
void updatesongtime(char ch);
int  dofunction(unsigned char forcedbutton);

/* playlist.o */
void pl_showents( int startpos, struct playlistent *playlist );
void pl_updatebuttons(int add);
void l_status(char *text);
unsigned int pl_count( struct playlistent *playlist );
struct playlistent *clearplaylist(struct playlistent *playlist);
struct playlistent *pl_seek( unsigned int pos, struct playlistent *playlist );
struct playlistent *pl_dofunction(struct playlistent *playlist, unsigned int *filenumber);
struct playlistent *rplaylist(struct playlistent *playlist, unsigned int *filenumber);
struct playlistent *addfiletolist(struct playlistent *playlist, char *filename, char *showname, unsigned int bitrate, unsigned int samplerate, unsigned char mode, char scanid3 );
struct playlistent *sortplaylist(struct playlistent *playlist);

/* filelist.o */
void fl_showents( int startpos, struct filelistent *filelist );
void fl_updatebuttons( int add );
void releasedir( struct filelistent *filelist );
void togglemark( struct filelistent *filelist, char movedown );
void saveplaylist(struct playlistent *playlist, char *filename);
struct playlistent *loadplaylist(struct playlistent *playlist, char *filename, char filemanager );
struct playlistent *fl_dofunction( struct filelistent *filelist, struct playlistent *playlist );
struct playlistent *getfiles( struct playlistent *playlist );
struct filelistent *camp_chdir( struct filelistent *filelist );
struct filelistent *loaddir( );
struct filelistent *file_seek( int pos, struct filelistent *filelist );
struct filelistent *sortfilelist( struct filelistent *filelist );

/* id3.o */
struct ID3 usable2id3(struct usableID3 *usabletag);
struct usableID3 id32usable(struct ID3 *tag);
void id3edit(char *filename, struct playlistent *playlist);
char *fuckspaces(char *lame, int maxpos);
char getmp3info(char *filename, unsigned char *mode, unsigned int *sample_rate, unsigned int *bitrate, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre);
char writemp3info(char *filename, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre);

/* fork.o */
void disappear(void);
void sigusr1(int signr);
void stealback(void);

/* player.o */
void call_player(struct playlistent *pl);
void slave(char *filename);
void killslave(void);
void playnext(int);

/* cconfig.o */
struct configstruct getconfig(char *configfile);

/* misc.o */
int  readln(FILE *fd, char *text);
int  exist(char *fname);
char *readyxline(char y, char x, char *preval, unsigned char maxlen, int *exitchar, int *modified);
char *strtrim(char *text);
char *xys(unsigned char number, char ch);

/* oops =) */
char *lowercases(char *str);
void lowcases( char *strng );

void readpass(char *text, int len);
void termios_raw(struct termios *termios_p);
unsigned int  myrand(double maxval);
unsigned char mykbhit(unsigned int sec, unsigned long usec);

#endif
