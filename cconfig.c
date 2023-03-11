#include <string.h>
#include <stdio.h>
#include "camp.h"

extern char useid3;

struct configstruct getconfig(char *configfile) {
struct configstruct cconfig;
char buf[128];
char arg[100];
char value[100];
FILE *fd;
   
   memset((void*)&cconfig, 0, sizeof(cconfig));
   
   if ( !exist(configfile) ) {
      fd = fopen(configfile, "w");
      fprintf(fd, "# Configfile created by CAMP\nplayer=/usr/local/bin/mpg123\nplaymode=random\n");
      fclose(fd);
   }
   fd = fopen(configfile, "r");
   while ( readln(fd, buf) != EOF ) {
      
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
	cconfig.playmode = 0;
      else
	if ( !strcasecmp(arg, "id3") ) 
	  if ( !strcasecmp(value, "yes") || !strcasecmp(value, "true") || !strcasecmp(value, "1") )
	    useid3 = TRUE; else
	useid3 = FALSE;
   }
   
   fclose(fd);
   
   switch(cconfig.playmode) {
    case 0: printf("Playing files in straight order.\n"); break;
    case 1: printf("Playing files in straight order (repeat).\n"); break;
    case 2: printf("Playing files in random order.\n"); break;
    default: 
      printf("Invalid mode! Setting to normal.\b"); 
      cconfig.playmode = 0;
      break;
   }
   
   return cconfig;

}
