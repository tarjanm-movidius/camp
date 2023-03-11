#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "camp.h"

int readln(FILE *fd, char *text) {
   char buf[256];
   int ch, i=0;
   do {
      ch = getc(fd);
      if ( ch != EOF && ch != '\n' ) {
	 buf[i] = ch;
	 i++;
      }
   } while( ch != EOF && ch != '\n' );
   buf[i] = 0;
   strcpy(text, buf);
   
   if ( ch == EOF && i == 0) return EOF; else
     return i;
}


char *readyxline(char y, char x, char *preval, unsigned char maxlen, int *exitchar, int *modified) {
fd_set stdinfds;
int ch;
unsigned char pos=0;
static char buf[256];
   
   if ( preval != NULL ) {
      strcpy(buf, preval);
      pos = strlen(buf);
   } else
     buf[0] = 0;
   
   printf("\e[%d;%dH%s", y, x, buf); fflush(stdout);
     
   do {
      FD_ZERO(&stdinfds);
      FD_SET(0, &stdinfds);
      select(1, &stdinfds, NULL, NULL, NULL);
      ch = getchar();
      
      do {
	 if ( ch > 31 && pos < maxlen && ch != 127 ) {
	    buf[strlen(buf)+1] = 0;
	    buf[strlen(buf)] = ch;
	    printf("%c", ch);
	    pos++;
	    *modified = TRUE;
	 } else
	   if (ch == 127 && pos != 0) {
	      buf[strlen(buf)-1] = 0;
	      pos--;
	      printf("\e[%d;%dH%s \e[%d;%dH", y, x, buf, y, x+pos);
	      *modified = TRUE;
	   } else
	   if ( ch == 27 ) {
	      ch = getchar();
	      if ( ch == '[' ) {
		 switch( getchar() ) {
		  case 'A': 
		    *exitchar = 'A';
		    return buf;
		  case 'B':
		    *exitchar = 'B';
		    return buf;
		  default: ;
		 }
	      } else {
		 while ( getchar() != -1 ) ;
		 *exitchar = 27;
		 return buf;
	      }
	   } else 
	   if ( ch == 13 ) {
	      *exitchar = 13;
	      return buf;
	   }
	 
	 fflush(stdout);
	 ch = getchar();
      } while ( ch != -1 );
   } while ( TRUE );
   
return buf;
}


void readpass(char *text, int len) {
char tmp[100];
int ch=-1;
   
   tmp[0] = 0;
   while ( ch != 13 ) {
      ch = getchar();
      if ( ch != -1 ) {
	 switch(ch) { 	    
	  case 127: if ( tmp[0] != 0 ) {
	     tmp[strlen(tmp)-1] = 0;
	     printf("\b \b"); 
	  } break;
	    
	  case 13: break;
	    
	  default: if ( strlen(tmp) == len ) break; tmp[strlen(tmp)+1] = 0; tmp[strlen(tmp)] = ch; printf("*"); break;
	 } // switch(ch)
	 
      } // if ch != -1
   } // while ch ! 13
   
   strcpy(text, tmp);
} // readpass(char *text)


int exist(char *fname) {
FILE *fd;
   fd = fopen(fname, "rb");
   if ( fd != NULL ) {   
      fclose(fd);
      return(1); /* file exists! */
   } else return(0); /* file does not exist! */
}

unsigned int myrand(double maxval) {
struct timeval tv;
int i;
   
   gettimeofday(&tv, NULL);
   srand(tv.tv_sec+tv.tv_usec);
   i=(int) (maxval*rand()/(RAND_MAX+1.0));
   return(i);
}

void lowcases( char *strng ) {
int i=0;
   
   for(;i<strlen(strng);i++)
     strng[i] = tolower(strng[i]);
}

unsigned char mykbhit(unsigned int sec) {
struct timeval tv;
fd_set fds;
   
   tv.tv_sec = sec;
   tv.tv_usec = 0;
   FD_ZERO(&fds);
   FD_SET(0, &fds);
   if ( select(1, &fds, NULL, NULL, &tv) ) return TRUE; else
     return FALSE;
}

int strchrpos(const char *text, int ch, int what) {
int i=0, wht=0;
   for(;i<strlen(text);i++)
     if ( text[i] == ch ) {
	wht++;
	if ( what == wht ) return i;
     }
   return -1;
}

char *strtrim(char *text) {
   
   while (text[0] == 32 || text[strlen(text)-1] == 32) {
      if ( text[0] == 32 ) strcpy(text, (char*)text+1);
      if ( text[strlen(text)-1] == 32 ) text[strlen(text)-1] = '\000';
   }
   return text;
}

