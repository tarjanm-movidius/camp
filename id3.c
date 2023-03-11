#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "camp.h"
#include "id3gui.c"
#include "id3gui7bit.c"

unsigned int bitrate_table[3][3][15] =
{
     {
	  {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
	  {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
	  {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}
     },
     {
	  {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
	  {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
	  {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
     },
     {
	  {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
	  {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
	  {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
     }
};


unsigned int samplerate_table[3][4] =  {
     {44100, 48000, 32000, 0},
     {22050, 24000, 16000, 0},
     {11025, 8000,  8000,  0}
};

extern struct configstruct config;
	     
char getmp3info(char *filename, unsigned char *mode, unsigned int *sample_rate, unsigned int *bit_rate, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre) {
FILE *fd;
struct ID3 filetag;
unsigned char buf[31];
int lay, error_protection, padding, extension, mode_ext, copyright, original, emphasis;
unsigned int bitrate=0, sampling_frequency=0;
unsigned char version, stereo=0;
   
   fd = fopen(filename, "r");
   if ( fd == NULL ) return FALSE;
   
   fread(&buf, 4, 1, fd);
   if ( ( buf[0] != 0xFF ) || (( buf[1] & 0xE0) != 0xE0 ) )
      do { 
	 memmove(&buf[0], &buf[1], 3);
	 buf[3] = getc(fd);
      } while ( ( buf[0] != 0xFF ) || (( buf[1] & 0xE0) != 0xE0 ) );
   
   fseek(fd, -128, SEEK_END);
   fread((void*)&filetag, 128, 1, fd);
   fclose(fd);
   
   switch ((buf[1] >> 2) & 0x3) {
      case 2:
	version = 0;
	break;
      case 3:
	version = 1;
	break;
      case 0:
	version = 2;
	break;
      default:
	version = -1;
     }
   lay = 4 - ((buf[1] >> 1) & 0x03);
   bitrate = bitrate_table[version][lay - 1][(buf[2] >> 4) & 0x0F];
   sampling_frequency = samplerate_table[version][(buf[2] >> 2) & 0x3];
   /* padding = (buf[2] >> 1) & 0x01; 
    extension = buf[2] & 0x01;
    error_protection = !(buf[1] & 0x1); 
    mode_ext = (buf[3] >> 4) & 0x03;
    copyright = (buf[3] >> 3) & 0x01;
    original = (buf[3] >> 2) & 0x01;
    emphasis = (buf[3]) & 0x3; */
   stereo = (((buf[3] >> 6) & 0x03) == 3) ? 0 : 1; 
   if ( bit_rate != NULL ) {
      *bit_rate    = bitrate;
      *sample_rate = sampling_frequency;
      *mode = stereo;
   }
   if ( !strncmp(filetag.tag, "TAG", 3) ) {
      if ( name   != NULL ) strcpy(name, fuckspaces(filetag.songname, 30));
      if ( artist != NULL ) strcpy(artist, fuckspaces(filetag.artist, 30));
      if ( misc   != NULL ) strcpy(misc, fuckspaces(filetag.misc, 30));
      if ( album  != NULL ) strcpy(album, fuckspaces(filetag.album, 30));
      if ( year   != NULL ) strcpy(year, fuckspaces(filetag.year, 4));
      genre = filetag.genre;
      return TRUE;
   } else
   return FALSE;
}

char writemp3info(char *filename, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre) {
FILE *fd;
struct ID3 filetag;
char buf[31];
int  i;
   
   fd = fopen(filename, "r+");
   if ( fd == NULL ) return FALSE;
   strcpy(filetag.tag, "TAG"); /* the NULL will get at songname[0] but who cares? :) */
   strcpy(filetag.songname, name);
   for(i=strlen(name);i<30;i++) filetag.songname[i] = 32;
   strcpy(filetag.artist, artist);
   for(i=strlen(artist);i<30;i++) filetag.artist[i] = 32;
   strcpy(filetag.album, album);
   for(i=strlen(album);i<30;i++) filetag.album[i] = 32;
   strcpy(filetag.year, year);
   for(i=strlen(year);i<4;i++) filetag.year[i] = 32;
   strcpy(filetag.misc, misc);
   for(i=strlen(misc);i<30;i++) filetag.misc[i] = 32;
   filetag.genre = genre; 
   fseek(fd, -128, SEEK_END);
   fread(buf, 3, 1, fd);
   if ( strncmp(buf, "TAG", 3) )
     fseek(fd, 0, SEEK_END); else
     fseek(fd, -128, SEEK_END);        
   fwrite((void*)&filetag, 128, 1, fd); 
   fclose(fd);
   return TRUE;
}


char *fuckspaces(char *lame, int maxpos) {
int i;
static char buf[256];
   strcpy(buf, lame);
   buf[maxpos] = 0;
   for(i=maxpos-1;i>-1;i--)
     if (lame[i] == ' ') buf[i] = 0; else return buf;
   return buf;
}

void id3edit(char *filename, struct playlistent *playlist) {
char name[31], artist[31], album[31], misc[31], year[5], i=0, cspos=0, buf[256], pos=0;
unsigned char genre;
int modified=FALSE, exitchar=0;
fd_set stdinfds;

   if ( filename == NULL ) return;
   
   for(;i<strlen(filename);i++)
     if (filename[i] == '/' ) cspos = 0; else {
	buf[cspos] = filename[i];
	cspos++;
     }
   
   if ( cspos > 39 ) buf[39] = 0; else
     buf[cspos] = 0;
   
   if ( !getmp3info(filename, NULL, NULL, NULL, name, artist, misc, album, year, genre) ) {
      name[0]   = 0;
      artist[0] = 0;
      album[0]  = 0;
      misc[0]   = 0;
      year[0]   = 0;
      genre     = 0;
   }
   if ( config.ttymode == 8 )
     printf("%s\e[?25h\e[0;34;46m", id3screen); else
     printf("%s\e[?25h\e[0;34;46m", id3screen7bit);
   printf("\e[9;27H%s", buf);
   printf("\e[10;27H%s", artist);
   printf("\e[11;27H%s", name);
   printf("\e[12;27H%s", misc);
   printf("\e[13;27H%s", album);
   printf("\e[14;28H%s", year);
   fflush(stdout);

   do { /* welcome to switcher's paradise */

      switch(pos) {
       case 0: strcpy(artist, readyxline(10, 27, artist, 30, &exitchar, &modified)); break;
       case 1: strcpy(name,   readyxline(11, 27, name, 30, &exitchar, &modified)); break;
       case 2: strcpy(misc,   readyxline(12, 27, misc, 30, &exitchar, &modified)); break;
       case 3: strcpy(album,  readyxline(13, 27, album, 30, &exitchar, &modified)); break;
       case 4: strcpy(year,   readyxline(14, 28, year, 4, &exitchar, &modified)); break;
      }
      
      if ( pos == 4 && exitchar == 13 ) exitchar = 27; else
      
      switch(exitchar) {
       case 'B': ;
       case 13:  pos++; break;
       case 'A': pos--; break;
      }
      
   } while ( exitchar != 27 );
   
   if ( modified ) {
      printf("\e[14;34H\e[0;30;46mSave (Y/n)"); fflush(stdout);
      FD_ZERO(&stdinfds);
      FD_SET(0, &stdinfds);
      select(1, &stdinfds, NULL, NULL, NULL);
      if ( toupper(getchar()) != 'N' ) {
	 writemp3info(filename, name, artist, misc, album, year, genre);
	 if ( playlist != NULL ) 
	   if ( artist[0] != 0 )
	     sprintf(playlist->showname, "%s - %s", artist, name); else
	   strcpy(playlist->showname, name);
      }
   } /* if modified */
   printf("\e[?25l");
#ifdef USE_GPM_MOUSE
   my_Gpm_Purge();
#endif
}

