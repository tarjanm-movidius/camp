#include <string.h>
#include <stdio.h>
#include "camp.h"

struct configstruct getconfig(char *configfile) {
struct configstruct cconfig;
char buf[128];
char arg[100];
char value[100];
int  i,j ;
FILE *fd;
   
   memset((void*)&cconfig, 0, sizeof(cconfig));
   
   if ( !exist(configfile) ) {
      fd = fopen(configfile, "w");
      fprintf(fd, "# Configfile created by CAMP\nplayer=/usr/local/bin/mpg123\nplaymode=random\nhidedot=true\ndontreopen=false;");
      fclose(fd);
   }
   fd = fopen(configfile, "r");

   while ( fgets((char*)buf, 125, fd) != NULL ) {
      if ( buf[strlen(buf)-1] == '\n' ) buf[strlen(buf)-1] = 0;
      if ( buf[0] == '#' || buf[0] == 0 ) continue;
      strncpy(arg, buf, strchrpos(buf, '=', 1) );
      strcpy(value, buf+strchrpos(buf, '=', 1)+1 );
      strcpy(arg, strtrim(arg));
      strcpy(value, strtrim(value));
      
      if ( !strcasecmp(arg, "player") ) {
	 strcpy ( cconfig.playername, rindex(value, '/')+1 );
	 strncpy( cconfig.playerpath, value, rindex(value, '/')-value );
	 if ( !exist(value) ) {
	    printf("Player %s does not exist, edit %s!\n", value, configfile);
	    exit(-1);
	 }
	 printf("Using %s (%s) as player\n", cconfig.playername, value);
	 cconfig.playerargv[0] = (char*)malloc(strlen(cconfig.playername)+1);
	 strcpy(cconfig.playerargv[0], cconfig.playername);
      } else
	if ( !strcasecmp(arg, "playmode") ) 
	  if ( !strcasecmp(value, "random") )
	    cconfig.playmode = 2; else
	if ( !strcasecmp(value, "loop") || !strcasecmp(value, "repeat") ) 
	  cconfig.playmode = 1; else 
	cconfig.playmode = 0; else
	if ( !strcasecmp(arg, "id3") ) 
	  if ( !strcasecmp(value, "yes") || !strcasecmp(value, "true") || !strcasecmp(value, "1") )
	    cconfig.useid3 = TRUE; else
	cconfig.useid3 = FALSE; else      
	if ( !strcasecmp(arg, "hidedot") )
	  if ( !strcasecmp(value, "yes") || !strcasecmp(value, "true") || !strcasecmp(value, "1") )
	    cconfig.hidedot = TRUE; else
	cconfig.hidedot = FALSE; else     
	if ( !strcasecmp(arg, "dontreopen") ) 
	  if ( !strcasecmp(value, "yes") || !strcasecmp(value, "true") || !strcasecmp(value, "1") )
	    cconfig.dontreopen = TRUE; else
	cconfig.dontreopen = FALSE; else
	if ( !strcasecmp(arg, "startincwd") )
	  if ( !strcasecmp(value, "yes") || !strcasecmp(value, "true") || !strcasecmp(value, "1") )
	    cconfig.startincwd = TRUE; else
	cconfig.startincwd = FALSE; else
	if ( !strcasecmp(arg, "showlength" ) )
	  if ( !strcasecmp(value, "both") )   cconfig.showtime = 3; else
	if ( !strcasecmp(value, "playlist") ) cconfig.showtime = 2; else
	if ( !strcasecmp(value, "main") )     cconfig.showtime = 1; else
	cconfig.showtime = 0; else
	if ( !strcasecmp(arg, "timemode" ) )
	  if ( !strcasecmp(value, "reverse") || !strcasecmp(value, "remaining" ) )
	    cconfig.timemode = 1; else
	cconfig.timemode = 0; else
	if ( !strcasecmp(arg, "ttymode" ) )
	  if ( value[0] == '7')
	    cconfig.ttymode = 7; else
	cconfig.ttymode = 8; else	
	if ( !strcasecmp(arg, "switches") || !strcasecmp(arg, "params") ) {
	   memset(buf, 0, 127);
	   i = 0;
	   value[strlen(value)+1] = 0; 
	   value[strlen(value)] = 32; 
	   for(j=1;j<15;j++)
	     if ( cconfig.playerargv[j] == NULL ) break;
	   while ( value[i] != 0 ) {
	      if ( value[i] == 32 && strlen(buf) != 0 ) {
		 cconfig.playerargv[j] = (char*)malloc(strlen(buf)+1);
		 strcpy(cconfig.playerargv[j], buf);
		 j++;
		 memset(buf, 0, 127);
	      } else 
		buf[strlen(buf)] = value[i];
	      i++;
	   }
	}
   }
   
   
   fclose(fd);
   
   if ( cconfig.dontreopen ) printf("NOT Reopening stdin, stdout!\n");
   
   if ( !strcmp(cconfig.playername, "mpg123") ) {
      printf("Using defaults for mpg123:\n");
      strcpy(cconfig.downmix, "-m");
      strcpy(cconfig.rate, "-r");
      strcpy(cconfig.quiet, "-q");
      strcpy(cconfig.device, "-a");      
      printf(" downmix = -m\n");
      printf(" rate    = -r\n");
      printf(" quiet   = -q\n");
      printf(" device  = -a\n");
   } else
     if ( !strcmp(cconfig.playername, "xaudio") ) {
	printf("Using defaults for xaudio:\n");
	strcpy(cconfig.downmix, "-mono=mix");
	cconfig.rate[0] = 0;
	cconfig.quiet[0] = 0;
	strcpy(cconfig.device, "-output=");      
	printf(" downmix = -mono=mix\n");
	printf(" rate    = n/a\n");
	printf(" quiet   = n/a\n");
	printf(" device  = -output=\n");
     } else
     if ( !strcmp(cconfig.playername, "amp") ) {
	printf("Using defaults for amp:\n");
	strcpy(cconfig.downmix, "-downmix");
	cconfig.rate[0] = 0;
	strcpy(cconfig.quiet, "-q");
	cconfig.device[0] = 0;
	printf(" downmix = -downmix\n");
	printf(" rate    = n/a\n");
	printf(" quiet   = -q\n");
	printf(" device  = n/a\n");
     } else
     printf("The player \"%s\" is unknown to me, no arguments added.\n");
   
   if ( cconfig.quiet[0] != 0 ) {
      for(i=0;i<15;i++)
	if ( cconfig.playerargv[i] == NULL ) break;
      cconfig.playerargv[i] = (char*)malloc(strlen(cconfig.quiet)+1);
      strcpy(cconfig.playerargv[i], cconfig.quiet);
   }
   
   i = 0;
   printf("Using params: ");
   while ( cconfig.playerargv[i] != NULL ) {
      printf("%s ", cconfig.playerargv[i]);
      i++ ;
   }
   printf("\n");
   
   
   
   switch(cconfig.playmode) {
    case 0: printf("Playing files in straight order.\n"); break;
    case 1: printf("Playing files in straight order (repeat).\n"); break;
    case 2: printf("Playing files in random order.\n"); break;
    default: 
      printf("Invalid mode! Setting to normal.\b"); 
      cconfig.playmode = 0;
      break;
   }

   switch(cconfig.showtime) {
    case 0: printf("Won't show songtime\n"); break;
    case 1: printf("Will show songtime in player window\n"); break;
    case 2: printf("Will show songtime in playlist editor\n"); break;
    case 3: printf("Will show songtime in main/playlist window\n"); break;
    default: 
      printf("Invalid mode! Setting to normal.\b"); 
      cconfig.showtime = 0;
      break;
   }
   
   if (cconfig.timemode == 1) printf ("Will show time as time remaining\n"); else
     printf("Will show time as time elapsed\n");
   
   if (cconfig.ttymode == 7) printf("Using 7bits interface\n"); else
     printf("Using full 8bits interface\n");
   
   return cconfig;
   
}
