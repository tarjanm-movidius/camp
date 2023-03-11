#ifndef __camp_h
# define __camp_h

#include <termios.h>
#include "config.h"
#include "machtype.h"
#include "build.h"

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

#define CAMP_VERSION "1.1" 

#define    MINBUTTON 1
#define    MAXBUTTON 14
#define PL_MINBUTTON 1
#define PL_MAXBUTTON 7
#define FL_MINBUTTON 1
#define FL_MAXBUTTON 7

#ifndef CUSTOM_BUTTON                    /* if NOT present in config.h, define here */
# define CUSTOM_BUTTON "mixr"            /* must be 4 chars exactly */
# define CUSTOM_RUN    "/usr/bin/aumix"  /* program to run when CUSTOM is pressed */
#endif                        


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
   int bitrate;
   unsigned char mode;
   unsigned int number;
   struct playlistent *prev;
   struct playlistent *next;
};

struct oneplaylistent {
   char name[256];
   char showname[100];
   unsigned int samplerate;
   int bitrate;
   unsigned char mode;
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
   char useid3;
   char hidedot;
   char playmode; /* 0 = normal, 1 = loop, 2 = randOm */
   char dontreopen;
};



/* main.o */
void myexit(void);
void myinit(void);
void escfix(void);
void updatebuttons(int add);
void updatedata(void);
void updatesongtime(char ch);
void termios_raw(struct termios *termios_p);
int  dofunction(void);

/* playlist.o */
void fl_showents( int startpos, struct filelistent *filelist );
void pl_showents( int startpos, struct playlistent *playlist );
void togglemark( struct filelistent *filelist );
void fl_updatebuttons(int add);
void pl_updatebuttons(int add);
void l_status(char *text);
void saveplaylist(struct playlistent *playlist, char *cdir, char *filename);
void releasedir(struct filelistent *filelist);
void clearplaylist(struct playlistent *playlist);
char *xys(unsigned char number, char ch);
char *lowercases(char *str);
int  pl_count( struct playlistent *playlist );
struct filelistent *camp_chdir(struct filelistent *filelist, char *cdir);
struct filelistent *loaddir( char *dir );
struct filelistent *file_seek( int pos, struct filelistent *filelist );
struct filelistent *sortfilelist(struct filelistent *filelist);
struct playlistent *pl_seek( unsigned int pos, struct playlistent *playlist );
struct playlistent *fl_dofunction( struct filelistent *filelist, struct playlistent *playlist, char *cdir );
struct playlistent *pl_dofunction(struct playlistent *playlist, unsigned int *filenumber);
struct playlistent *rplaylist(struct playlistent *playlist, unsigned int *filenumber);
struct playlistent *getfiles(struct playlistent *playlist);
struct playlistent *loadplaylist(struct playlistent *playlist, char *filename, char filemanager );
struct playlistent *addfiletolist(struct playlistent *playlist, char *filename, char *showname, int bitrate, unsigned int samplerate, unsigned char mode, char scanid3 );
struct playlistent *sortplaylist(struct playlistent *playlist);

/* id3.o */
struct ID3 usable2id3(struct usableID3 *usabletag);
struct usableID3 id32usable(struct ID3 *tag);
void id3edit(char *filename, struct playlistent *playlist);
char *fuckspaces(char *lame, int maxpos);
char getmp3info(char *filename, unsigned char *mode, unsigned int *sample_rate, int *bitrate, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre);
char writemp3info(char *filename, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre);

/* player.o */
void call_player(struct playlistent *pl);
void slave(char *filename);
void killslave(void);
void playnext(int);

/* cconfig.o */
struct configstruct getconfig(char *configfile);

/* misc.o */
int  readln(FILE *fd, char *text);
char *readyxline(char y, char x, char *preval, unsigned char maxlen, int *exitchar, int *modified);
void readpass(char *text, int len);
int  exist(char *fname);
unsigned int myrand(double maxval);
void lowcases( char *strng );
unsigned char mykbhit(unsigned int);
char *strtrim(char *text);

#endif
