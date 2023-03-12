#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "camp.h"
#include <sys/types.h>

#ifdef DEBUG
# define realloc(ptr_, sz_) ({void *retval_; if(ptr_) printf("WW: duplicate definition of '%s'\n", arg); fflush(stdout); retval_ = realloc(ptr_, sz_); retval_;})
#endif

struct configstruct getconfig(char *configfile)
{
    struct configstruct cconfig;
    char *buf, *arg, *value;
    int  i, j, k;
    FILE *fd;

    buf = (char*)malloc(CONF_BUF_LEN);
    memset((void*)&cconfig, 0, sizeof(cconfig));

    if ( !exist(configfile) ) {
        printf("The configfile %s does not exist! Copy camp.ini from the distribution package, and modify it to fit your needs!\n", configfile);
        exit(-1);
    }

    /* defaults */
    cconfig.volstep     = 3;
    cconfig.playmode    = 2;
    cconfig.useid3      = TRUE;
    cconfig.rescanid3   = FALSE;
    cconfig.startincwd  = TRUE;

#ifdef HAVE_LIBZ
    cconfig.compresspl  = TRUE;
#else
    cconfig.compresspl  = FALSE;
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
    cconfig.voldev      = SOUND_MIXER_PCM;
#endif
#ifdef ENABLE_RC
    cconfig.rctime      = 25000;
    cconfig.userc       = TRUE;
#else
    cconfig.rctime      = 900000;
    cconfig.userc       = FALSE;
#endif

    fd = fopen(configfile, "r");

    while ( fgets((char*)buf, CONF_BUF_LEN, fd) != NULL ) {
        if ( !parseconfig(buf, &arg, &value) ) continue;

        if ( !strcasecmp(arg, "player") ) {
            if ( !strchr(value, '/') ) {
                printf("You need to type the path to your mp3 player (%s)!", value);
                exit(-1);
            }
            strcpy ( cconfig.playername, (char*)rindex(value, '/')+1 );
            strncpy( cconfig.playerpath, value, (uintptr_t)rindex(value, '/')-(uintptr_t)value );

            if ( !exist(value) ) {
                printf("Player %s does not exist, edit %s!\n", value, configfile);
                exit(-1);
            }

            printf("Using %s (%s) as player\n", cconfig.playername, value);
            cconfig.playerargv[0] = (char*)realloc(cconfig.playerargv[0], strlen(cconfig.playername)+1);
            strcpy(cconfig.playerargv[0], cconfig.playername);
        } else if ( !strcasecmp(arg, "playmode") )
            if ( !strcasecmp(value, "random") )
                cconfig.playmode = 2;
            else if ( !strcasecmp(value, "loop") || !strcasecmp(value, "repeat") )
                cconfig.playmode = 1;
            else
                cconfig.playmode = 0;
        else
#ifdef HAVE_SYS_SOUNDCARD_H
            if ( !strcasecmp(arg, "volumedev") ) {
                if ( !strcasecmp(value, "pcm") ) cconfig.voldev = SOUND_MIXER_PCM;
                else if ( !strcasecmp(value, "synth") ) cconfig.voldev = SOUND_MIXER_SYNTH;
                else if ( !strcasecmp(value, "main") ) cconfig.voldev = SOUND_MIXER_VOLUME;
                else
                    cconfig.voldev = atoi(value);
            } else
#endif
                if ( !strcasecmp(arg, "dropfile") )
                    strcpy(cconfig.dropfile, value);
                else if ( !strcasecmp(arg, "dropinfo") )
                    cconfig.dropinfo = is_val_true(value);
                else if ( !strcasecmp(arg, "mpg123mode") )
                    cconfig.mpg123 = is_val_true(value);
                else if ( !strcasecmp(arg, "defplaylist") )
                    cconfig.defpl = is_val_true(value);
                else if ( !strcasecmp(arg, "readid3") )
                    cconfig.useid3 = is_val_true(value);
                else if ( !strcasecmp(arg, "rescaninfo") )
                    cconfig.rescanid3 = is_val_true(value);
                else if ( !strcasecmp(arg, "compresspl") )
                    cconfig.compresspl = is_val_true(value);
                else if ( !strcasecmp(arg, "scrollname") )
                    cconfig.scrollsn = is_val_true(value);
                else if ( !strcasecmp(arg, "userc") )
                    cconfig.userc = is_val_true(value);
                else if ( !strcasecmp(arg, "nicekill") )
                    cconfig.nicekill = is_val_true(value);
                else if ( !strcasecmp(arg, "skin") )
                    if ( !strcasecmp(value, "random") )
                        loadskin(randomskin(buf), &cconfig);
                    else
                        loadskin(value, &cconfig);
                else if ( !strcasecmp(arg, "rctime") )
                    cconfig.rctime = atoi(value);
                else if ( !strcasecmp(arg, "delay") )
                    cconfig.bufferdelay = atoi(value);
                else if ( !strcasecmp(arg, "volumestep") )
                    cconfig.volstep = atoi(value);
                else if ( !strcasecmp(arg, "mutevol") )
                    cconfig.mutevol = atoi(value);
                else if ( !strcasecmp(arg, "playerprio") )
                    cconfig.playerprio = atoi(value);
                else if ( !strcasecmp(arg, "kill2pids") )
                    cconfig.kill2pids = is_val_true(value);
                else if ( !strcasecmp(arg, "hidedot") )
                    cconfig.hidedot = is_val_true(value);
                else if ( !strcasecmp(arg, "dontreopen") )
                    cconfig.dontreopen = is_val_true(value);
                else if ( !strcasecmp(arg, "lockvt") )
                    cconfig.lockvt = is_val_true(value);
                else if ( !strcasecmp(arg, "forkonerror") )
                    cconfig.forkseg = is_val_true(value);
                else if ( !strcasecmp(arg, "tip") )
                    cconfig.showtip = is_val_true(value);
                else if ( !strcasecmp(arg, "startincwd") )
                    cconfig.startincwd = is_val_true(value);
                else if ( !strcasecmp(arg, "showlength" ) )
                    if      ( !strcasecmp(value, "both") )     cconfig.showtime = 3;
                    else if ( !strcasecmp(value, "playlist") ) cconfig.showtime = 2;
                    else if ( !strcasecmp(value, "main") )     cconfig.showtime = 1;
                    else                                       cconfig.showtime = 0;
                else if ( !strcasecmp(arg, "timemode" ) )
                    if ( !strcasecmp(value, "reverse") || !strcasecmp(value, "remaining" ) )
                        cconfig.timemode = 1;
                    else
                        cconfig.timemode = 0;
                else if ( !strcasecmp(arg, "play") )
                    cconfig.rc.play = atoi(value);
                else if ( !strcasecmp(arg, "pause") )
                    cconfig.rc.pause = atoi(value);
                else if ( !strcasecmp(arg, "stop") )
                    cconfig.rc.stop = atoi(value);
                else if ( !strcasecmp(arg, "skipb") )
                    cconfig.rc.skipb = atoi(value);
                else if ( !strcasecmp(arg, "skipf") )
                    cconfig.rc.skipf = atoi(value);
                else if ( !strcasecmp(arg, "seekb") )
                    cconfig.rc.seekb = atoi(value);
                else if ( !strcasecmp(arg, "seekf") )
                    cconfig.rc.seekf = atoi(value);
                else if ( !strcasecmp(arg, "vol-") )
                    cconfig.rc.voldec = atoi(value);
                else if ( !strcasecmp(arg, "vol+") )
                    cconfig.rc.volinc = atoi(value);
                else if ( !strcasecmp(arg, "port") )
                    cconfig.rc.port = atoi(value);
                else if ( !strcasecmp(arg, "switches") || !strcasecmp(arg, "params") ) {
                    for(k = 1; k < 15; k++)
                        if ( cconfig.playerargv[k] == NULL ) break;
                    for (i = 0; k < 15 && value[i]; ) {
                        while (IS_SPACE(value[i])) i++;
                        j = i;
                        while (value[i] && !IS_SPACE(value[i])) i++;
                        if (i != j) {
                            cconfig.playerargv[k] = (char*)malloc(i-j+1);
                            memcpy(cconfig.playerargv[k], &value[j], i-j);
                            cconfig.playerargv[k][i-j] = 0;
                            k++;
                        }
                    }
                }
    }

    fclose(fd);

    if ( cconfig.dontreopen ) printf("NOT Reopening stdin, stdout!\n");

//  printf("%s\n", cconfig.playername);
    if ( cconfig.playername[0] == 0 ) {
        printf("You need to set a default player in ~/.camp/camprc!\n");
        exit(-1);
    } else if ( !strcmp(cconfig.playername, "mpg123") ) {
        printf("Using defaults for mpg123\n");
        strcpy(cconfig.downmix, "-m");
        strcpy(cconfig.rate, "-r");
        strcpy(cconfig.quiet, "-q");
        strcpy(cconfig.device, "-a");
/*      printf(" downmix = -m\n");
        printf(" rate    = -r\n");
        printf(" quiet   = -q\n");
        printf(" device  = -a\n");
        printf("Using nice kill\n");
        cconfig.nicekill = TRUE; */
    } else if ( !strcmp(cconfig.playername, "xaudio") ) {
        printf("Using defaults for xaudio\n");
        strcpy(cconfig.downmix, "-mono=mix");
        cconfig.rate[0] = 0;
        cconfig.quiet[0] = 0;
        strcpy(cconfig.device, "-output=");
/*      printf(" downmix = -mono=mix\n");
        printf(" rate    = n/a\n");
        printf(" quiet   = n/a\n");
        printf(" device  = -output=\n"); */
    } else if ( !strcmp(cconfig.playername, "amp") ) {
        printf("Using defaults for amp\n");
        strcpy(cconfig.downmix, "-downmix");
        cconfig.rate[0] = 0;
        strcpy(cconfig.quiet, "-q");
        cconfig.device[0] = 0;
/*      printf(" downmix = -downmix\n");
        printf(" rate    = n/a\n");
        printf(" quiet   = -q\n");
        printf(" device  = n/a\n"); */
    } else
        printf("The player \"%s\" is unknown to me, no arguments added.\n", cconfig.playername);

    if ( strcmp(cconfig.playername, "mpg123") && cconfig.mpg123 )
        printf("\e[1mWARNING! Setting mpg123mode = true in ~/.camp/camprc without having mpg123 as the player will result in failure trying to play music!!\e[0m\n");

    if ( cconfig.quiet[0] != 0 ) {
        for(i=0; i<14; i++)
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
    case 0:
        printf("Playing files in straight order.\n");
        break;
    case 1:
        printf("Playing files in straight order (repeat).\n");
        break;
    case 2:
        printf("Playing files in random order.\n");
        break;
    default:
        printf("Invalid mode! Setting to normal.\b");
        cconfig.playmode = 0;
        break;
    }

    switch(cconfig.showtime) {
    case 0:
        printf("Won't show songtime\n");
        break;
    case 1:
        printf("Will show songtime in player window\n");
        break;
    case 2:
        printf("Will show songtime in playlist editor\n");
        break;
    case 3:
        printf("Will show songtime in main/playlist window\n");
        break;
    default:
        printf("Invalid mode! Setting to normal.\b");
        cconfig.showtime = 0;
        break;
    }

    if (cconfig.timemode == 1) printf ("Will show time as time remaining\n");
    else
        printf("Will show time as time elapsed\n");

#ifdef HAVE_LIBZ
    if ( cconfig.compresspl ) printf("Saved playlists (with id3 tags) will be compressed.\n");
    else
        printf("Saved playlists will be uncompressed.");
#else
    if ( cconfig.compresspl ) printf("Playlists are set to be compressed, but at compile-time, the 'zlib' could not be found, disabling compression.\n");
    cconfig.compresspl = FALSE;
#endif

    free(buf);
    return cconfig;
}

void loadskin(char *name, struct configstruct *config)
{
    int    fd,i,version=0;
    FILE  *filefd;
    struct stat statbuf;
    char *buf, *arg, *value, skin_home[CONF_BUF_LEN-16];

    buf = (char*)malloc(CONF_BUF_LEN);
    memset((void*)&config->skin, 0, sizeof(struct skinconfig));

    for(fd=MINBUTTON; fd<(MAXBUTTON+1); fd++) {
        config->skin.mju[fd] = -1;
        config->skin.mjd[fd] = -1;
        config->skin.mjl[fd] = -1;
        config->skin.mjr[fd] = -1;
        config->skin.mh[fd]  = -1;
    }

    memset(config->skin.plistbo, -1, sizeof(config->skin.plistbo));
    memset(config->skin.flistbo, -1, sizeof(config->skin.flistbo));

    /*  Erp, this is default hotkeys for volume +/- ..
     *  i felt it had to be defaults here..
     */

    config->skin.mh[15] = '-';
    config->skin.mh[16] = '+';
    config->skin.mpclr = TRUE;
    config->skin.miclr = TRUE;
    config->skin.piclr = TRUE;

    config->skin.id3fnw = 39;
    /* I fucked up in v1.00, included this in the structure, but NEVER used it, so set default for "old" skins */

    printf("Loading skin \"%s\"... ", name);
    fflush(stdout);
    sprintf(skin_home,"%s/.camp/skins/%s",getenv("HOME"), name);
    if (!probe_dir(skin_home)) {
        sprintf(skin_home,"/usr/share/camp/skins/%s",name);
        if (!probe_dir(skin_home)) {
            printf("Unable to find skin %s. \n",name);
            exit(-1);
        }
    }

    sprintf(buf, "%s/main.ans", skin_home);
    fd = open(buf, O_RDONLY);
    if ( fd == -1 ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }
    fstat(fd, &statbuf);
    config->skin.main = (char*)malloc(statbuf.st_size+1);
    read(fd, config->skin.main, statbuf.st_size);
    close(fd);
    config->skin.main[statbuf.st_size] = 0;

    sprintf(buf, "%s/playlist.ans", skin_home);
    fd = open(buf, O_RDONLY);
    if ( fd == -1 ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }
    fstat(fd, &statbuf);
    config->skin.playlist = (char*)malloc(statbuf.st_size+1);
    read(fd, config->skin.playlist, statbuf.st_size);
    close(fd);
    config->skin.playlist[statbuf.st_size] = 0;

    sprintf(buf, "%s/filelist.ans", skin_home);
    fd = open(buf, O_RDONLY);
    if ( fd == -1 ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }
    fstat(fd, &statbuf);
    config->skin.filelist = (char*)malloc(statbuf.st_size+1);
    read(fd, config->skin.filelist, statbuf.st_size);
    close(fd);
    config->skin.filelist[statbuf.st_size] = 0;

    sprintf(buf, "%s/id3.ans", skin_home);
    fd = open(buf, O_RDONLY);
    if ( fd == -1 ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }
    fstat(fd, &statbuf);
    config->skin.id3 = (char*)malloc(statbuf.st_size+1);
    read(fd, config->skin.id3, statbuf.st_size);
    close(fd);
    config->skin.id3[statbuf.st_size] = 0;

    sprintf(buf, "%s/main.conf", skin_home);
    filefd = fopen(buf, "r");
    if ( !filefd ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }

    while ( fgets((char*)buf, CONF_BUF_LEN, filefd) != NULL ) { /* parse skin configuration */

        if ( !parseconfig(buf, &arg, &value) ) continue;

        if ( !strcasecmp(arg, "version") ) {
            version = atoi(value);
            if ( version < 100 || version >= 110 ) {
                printf("a) skin version is %d, must be between 100-109 to work with this camp version!\n", version);
                exit(-1);
            }
        } else if ( !strcasecmp(arg, "textcolor") ) {
            config->skin.mtextc = (char*)realloc(config->skin.mtextc, strlen(value)+1);
            cpy_replace(config->skin.mtextc, value, '&', 27);
        } else if ( !strcasecmp(arg, "textx") )
            config->skin.mtextx = atoi(value);
        else if ( !strcasecmp(arg, "texty") )
            config->skin.mtexty = atoi(value);
        else if ( !strcasecmp(arg, "textw") )
            config->skin.mtextw = atoi(value);
        else if ( !strcasecmp(arg, "startbutton") )
            config->skin.msb = atoi(value);
        else if ( !strcasecmp(arg, "lockpass") ) {
            config->skin.mainmsg[0] = (char*)realloc(config->skin.mainmsg[0], strlen(value)+1);
            cpy_replace(config->skin.mainmsg[0], value, '&', 27);
        } else if ( !strcasecmp(arg, "lockpassagain") ) {
            config->skin.mainmsg[1] = (char*)realloc(config->skin.mainmsg[1], strlen(value)+1);
            cpy_replace(config->skin.mainmsg[1], value, '&', 27);
        } else if ( !strcasecmp(arg, "unlock") ) {
            config->skin.mainmsg[2] = (char*)realloc(config->skin.mainmsg[2], strlen(value)+1);
            cpy_replace(config->skin.mainmsg[2], value, '&', 27);
        } else if ( !strcasecmp(arg, "jumpsong") ) {
            config->skin.mainmsg[3] = (char*)realloc(config->skin.mainmsg[3], strlen(value)+1);
            cpy_replace(config->skin.mainmsg[3], value, '&', 27);
        } else


            if ( !strcasecmp(arg, "playlistvisible") )
                config->skin.platmain = atoi(value);
            else if ( !strcasecmp(arg, "id3editvisible") )
                config->skin.id3atmain = atoi(value);
            else if ( !strcasecmp(arg, "mouseexpand") )
                config->skin.mouseexpand = atoi(value);
            else if ( !strcasecmp(arg, "clearscreenafterpl" ) )
                if ( !strcasecmp(value, "true") ) config->skin.mpclr = TRUE;
                else
                    config->skin.mpclr = FALSE;
            else if ( !strcasecmp(arg, "clearscreenafterid3" ) )
                if ( !strcasecmp(value, "true") ) config->skin.miclr = TRUE;
                else
                    config->skin.miclr = FALSE;
            else if ( !strcasecmp(arg, "timex") )
                config->skin.timex = atoi(value);
            else if ( !strcasecmp(arg, "timey") )
                config->skin.timey = atoi(value);
            else if ( !strcasecmp(arg, "volumex") )
                config->skin.volx = atoi(value);
            else if ( !strcasecmp(arg, "volumey") )
                config->skin.voly = atoi(value);
            else if ( !strcasecmp(arg, "songnamex") )
                config->skin.songnamex = atoi(value);
            else if ( !strcasecmp(arg, "songnamey") )
                config->skin.songnamey = atoi(value);
            else if ( !strcasecmp(arg, "songnamew") )
                config->skin.songnamew = atoi(value);
            else if ( !strcasecmp(arg, "sampleratex") )
                config->skin.sampleratex = atoi(value);
            else if ( !strcasecmp(arg, "sampleratey") )
                config->skin.sampleratey = atoi(value);
            else if ( !strcasecmp(arg, "bitratex") )
                config->skin.bitratex = atoi(value);
            else if ( !strcasecmp(arg, "bitratey") )
                config->skin.bitratey = atoi(value);
            else if ( !strcasecmp(arg, "modetexty") )
                config->skin.modetexty = atoi(value);
            else if ( !strcasecmp(arg, "modetextx") )
                config->skin.modetextx = atoi(value);
            else if ( !strcasecmp(arg, "modetextw") )
                config->skin.modetextw = atoi(value);
            else if ( !strcasecmp(arg, "modenormal") ) {
                config->skin.modetext[0] = (char*)realloc(config->skin.modetext[0], strlen(value)+1);
                cpy_replace(config->skin.modetext[0], value, '&', 27);
            } else if ( !strcasecmp(arg, "modeloop") ) {
                config->skin.modetext[1] = (char*)realloc(config->skin.modetext[1], strlen(value)+1);
                cpy_replace(config->skin.modetext[1], value, '&', 27);
            } else if ( !strcasecmp(arg, "moderandom") ) {
                config->skin.modetext[2] = (char*)realloc(config->skin.modetext[2], strlen(value)+1);
                cpy_replace(config->skin.modetext[2], value, '&', 27);
            } else if ( !strcasecmp(arg, "stereox") )
                config->skin.stereox = atoi(value);
            else if ( !strcasecmp(arg, "stereoy") )
                config->skin.stereoy = atoi(value);
            else if ( !strcasecmp(arg, "stereow") )
                config->skin.stereow = atoi(value);
            else if ( !strcasecmp(arg, "monotext") ) {
                config->skin.stereotext[0] = (char*)realloc(config->skin.stereotext[0], strlen(value)+1);
                cpy_replace(config->skin.stereotext[0], value, '&', 27);
            } else if ( !strcasecmp(arg, "stereotext") ) {
                config->skin.stereotext[1] = (char*)realloc(config->skin.stereotext[1], strlen(value)+1);
                cpy_replace(config->skin.stereotext[1], value, '&', 27);
            } else if ( !strcasecmp(arg, "songnumberx") )
                config->skin.songnumberx = atoi(value);
            else if ( !strcasecmp(arg, "songnumbery") )
                config->skin.songnumbery = atoi(value);
            else if ( !strcasecmp(arg, "songnamecolor") ) {
                config->skin.songnamec = (char*)realloc(config->skin.songnamec, strlen(value)+1);
                cpy_replace(config->skin.songnamec, value, '&', 27);
            } else if ( !strcasecmp(arg, "songnumbercolor") ) {
                config->skin.songnumberc = (char*)realloc(config->skin.songnumberc, strlen(value)+1);
                cpy_replace(config->skin.songnumberc, value, '&', 27);
            } else if ( !strcasecmp(arg, "bitratecolor") ) {
                config->skin.bitratec = (char*)realloc(config->skin.bitratec, strlen(value)+1);
                cpy_replace(config->skin.bitratec, value, '&', 27);
            } else if ( !strcasecmp(arg, "sampleratecolor") ) {
                config->skin.sampleratec = (char*)realloc(config->skin.sampleratec, strlen(value)+1);
                cpy_replace(config->skin.sampleratec, value, '&', 27);
            } else if ( !strcasecmp(arg, "timecolor") ) {
                config->skin.timec = (char*)realloc(config->skin.timec, strlen(value)+1);
                cpy_replace(config->skin.timec, value, '&', 27);
            } else if ( !strcasecmp(arg, "stereocolor") ) {
                config->skin.stereoc = (char*)realloc(config->skin.stereoc, strlen(value)+1);
                cpy_replace(config->skin.stereoc, value, '&', 27);
            } else if ( !strcasecmp(arg, "modetextcolor") ) {
                config->skin.modetextc = (char*)realloc(config->skin.modetextc, strlen(value)+1);
                cpy_replace(config->skin.modetextc, value, '&', 27);
            } else if ( !strcasecmp(arg, "volumecolor") ) {
                config->skin.volc = (char*)realloc(config->skin.volc, strlen(value)+1);
                cpy_replace(config->skin.volc, value, '&', 27);
            } else

                /* warder, here you got your h0wt-keyz :> */

                if ( !strcasecmp(arg, "skipbh") )
                    config->skin.mh[0] = value[0];
                else if ( !strcasecmp(arg, "playh") )
                    config->skin.mh[1] = value[0];
                else if ( !strcasecmp(arg, "skipfh") )
                    config->skin.mh[2] = value[0];
                else if ( !strcasecmp(arg, "stoph") )
                    config->skin.mh[3] = value[0];
                else if ( !strcasecmp(arg, "modeh") )
                    config->skin.mh[4] = value[0];
                else if ( !strcasecmp(arg, "plh") )
                    config->skin.mh[5] = value[0];
                else if ( !strcasecmp(arg, "custh") )
                    config->skin.mh[6] = value[0];
                else if ( !strcasecmp(arg, "seekbh") )
                    config->skin.mh[7] = value[0];
                else if ( !strcasecmp(arg, "pauseh") )
                    config->skin.mh[8] = value[0];
                else if ( !strcasecmp(arg, "seekfh") )
                    config->skin.mh[9] = value[0];
                else if ( !strcasecmp(arg, "jumph") )
                    config->skin.mh[10] = value[0];
                else if ( !strcasecmp(arg, "lockh") )
                    config->skin.mh[11] = value[0];
                else if ( !strcasecmp(arg, "desch") )
                    config->skin.mh[12] = value[0];
                else if ( !strcasecmp(arg, "forkh") )
                    config->skin.mh[13] = value[0];
                else if ( !strcasecmp(arg, "muteh") )
                    config->skin.mh[14] = value[0];
                else if ( !strcasecmp(arg, "vol-h") )
                    config->skin.mh[15] = value[0];
                else if ( !strcasecmp(arg, "vol+h") )
                    config->skin.mh[16] = value[0];
                else

                    /* button moving */

                    if ( !strcasecmp(arg, "skipbu") )
                        config->skin.mju[0] = atoi(value);
                    else if ( !strcasecmp(arg, "playu") )
                        config->skin.mju[1] = atoi(value);
                    else if ( !strcasecmp(arg, "skipfu") )
                        config->skin.mju[2] = atoi(value);
                    else if ( !strcasecmp(arg, "stopu") )
                        config->skin.mju[3] = atoi(value);
                    else if ( !strcasecmp(arg, "modeu") )
                        config->skin.mju[4] = atoi(value);
                    else if ( !strcasecmp(arg, "plu") )
                        config->skin.mju[5] = atoi(value);
                    else if ( !strcasecmp(arg, "custu") )
                        config->skin.mju[6] = atoi(value);
                    else if ( !strcasecmp(arg, "seekbu") )
                        config->skin.mju[7] = atoi(value);
                    else if ( !strcasecmp(arg, "pauseu") )
                        config->skin.mju[8] = atoi(value);
                    else if ( !strcasecmp(arg, "seekfu") )
                        config->skin.mju[9] = atoi(value);
                    else if ( !strcasecmp(arg, "jumpu") )
                        config->skin.mju[10] = atoi(value);
                    else if ( !strcasecmp(arg, "locku") )
                        config->skin.mju[11] = atoi(value);
                    else if ( !strcasecmp(arg, "descu") )
                        config->skin.mju[12] = atoi(value);
                    else if ( !strcasecmp(arg, "forku") )
                        config->skin.mju[13] = atoi(value);
                    else if ( !strcasecmp(arg, "muteu") )
                        config->skin.mju[14] = atoi(value);
                    else if ( !strcasecmp(arg, "vol-u") )
                        config->skin.mju[15] = atoi(value);
                    else if ( !strcasecmp(arg, "vol+u") )
                        config->skin.mju[16] = atoi(value);
                    else

                        if ( !strcasecmp(arg, "skipbd") )
                            config->skin.mjd[0] = atoi(value);
                        else if ( !strcasecmp(arg, "playd") )
                            config->skin.mjd[1] = atoi(value);
                        else if ( !strcasecmp(arg, "skipfd") )
                            config->skin.mjd[2] = atoi(value);
                        else if ( !strcasecmp(arg, "stopd") )
                            config->skin.mjd[3] = atoi(value);
                        else if ( !strcasecmp(arg, "moded") )
                            config->skin.mjd[4] = atoi(value);
                        else if ( !strcasecmp(arg, "pld") )
                            config->skin.mjd[5] = atoi(value);
                        else if ( !strcasecmp(arg, "custd") )
                            config->skin.mjd[6] = atoi(value);
                        else if ( !strcasecmp(arg, "seekbd") )
                            config->skin.mjd[7] = atoi(value);
                        else if ( !strcasecmp(arg, "paused") )
                            config->skin.mjd[8] = atoi(value);
                        else if ( !strcasecmp(arg, "seekfd") )
                            config->skin.mjd[9] = atoi(value);
                        else if ( !strcasecmp(arg, "jumpd") )
                            config->skin.mjd[10] = atoi(value);
                        else if ( !strcasecmp(arg, "lockd") )
                            config->skin.mjd[11] = atoi(value);
                        else if ( !strcasecmp(arg, "descd") )
                            config->skin.mjd[12] = atoi(value);
                        else if ( !strcasecmp(arg, "forkd") )
                            config->skin.mjd[13] = atoi(value);
                        else if ( !strcasecmp(arg, "muted") )
                            config->skin.mjd[14] = atoi(value);
                        else if ( !strcasecmp(arg, "vol-d") )
                            config->skin.mjd[15] = atoi(value);
                        else if ( !strcasecmp(arg, "vol+d") )
                            config->skin.mjd[16] = atoi(value);
                        else

                            if ( !strcasecmp(arg, "skipbl") )
                                config->skin.mjl[0] = atoi(value);
                            else if ( !strcasecmp(arg, "playl") )
                                config->skin.mjl[1] = atoi(value);
                            else if ( !strcasecmp(arg, "skipfl") )
                                config->skin.mjl[2] = atoi(value);
                            else if ( !strcasecmp(arg, "stopl") )
                                config->skin.mjl[3] = atoi(value);
                            else if ( !strcasecmp(arg, "model") )
                                config->skin.mjl[4] = atoi(value);
                            else if ( !strcasecmp(arg, "pll") )
                                config->skin.mjl[5] = atoi(value);
                            else if ( !strcasecmp(arg, "custl") )
                                config->skin.mjl[6] = atoi(value);
                            else if ( !strcasecmp(arg, "seekbl") )
                                config->skin.mjl[7] = atoi(value);
                            else if ( !strcasecmp(arg, "pausel") )
                                config->skin.mjl[8] = atoi(value);
                            else if ( !strcasecmp(arg, "seekfl") )
                                config->skin.mjl[9] = atoi(value);
                            else if ( !strcasecmp(arg, "jumpl") )
                                config->skin.mjl[10] = atoi(value);
                            else if ( !strcasecmp(arg, "lockl") )
                                config->skin.mjl[11] = atoi(value);
                            else if ( !strcasecmp(arg, "descl") )
                                config->skin.mjl[12] = atoi(value);
                            else if ( !strcasecmp(arg, "forkl") )
                                config->skin.mjl[13] = atoi(value);
                            else if ( !strcasecmp(arg, "mutel") )
                                config->skin.mjl[14] = atoi(value);
                            else if ( !strcasecmp(arg, "vol-l") )
                                config->skin.mjl[15] = atoi(value);
                            else if ( !strcasecmp(arg, "vol+l") )
                                config->skin.mjl[16] = atoi(value);
                            else

                                if ( !strcasecmp(arg, "skipbr") )
                                    config->skin.mjr[0] = atoi(value);
                                else if ( !strcasecmp(arg, "playr") )
                                    config->skin.mjr[1] = atoi(value);
                                else if ( !strcasecmp(arg, "skipfr") )
                                    config->skin.mjr[2] = atoi(value);
                                else if ( !strcasecmp(arg, "stopr") )
                                    config->skin.mjr[3] = atoi(value);
                                else if ( !strcasecmp(arg, "moder") )
                                    config->skin.mjr[4] = atoi(value);
                                else if ( !strcasecmp(arg, "plr") )
                                    config->skin.mjr[5] = atoi(value);
                                else if ( !strcasecmp(arg, "custr") )
                                    config->skin.mjr[6] = atoi(value);
                                else if ( !strcasecmp(arg, "seekbr") )
                                    config->skin.mjr[7] = atoi(value);
                                else if ( !strcasecmp(arg, "pauser") )
                                    config->skin.mjr[8] = atoi(value);
                                else if ( !strcasecmp(arg, "seekfr") )
                                    config->skin.mjr[9] = atoi(value);
                                else if ( !strcasecmp(arg, "jumpr") )
                                    config->skin.mjr[10] = atoi(value);
                                else if ( !strcasecmp(arg, "lockr") )
                                    config->skin.mjr[11] = atoi(value);
                                else if ( !strcasecmp(arg, "descr") )
                                    config->skin.mjr[12] = atoi(value);
                                else if ( !strcasecmp(arg, "forkr") )
                                    config->skin.mjr[13] = atoi(value);
                                else if ( !strcasecmp(arg, "muter") )
                                    config->skin.mjr[14] = atoi(value);
                                else if ( !strcasecmp(arg, "vol-r") )
                                    config->skin.mjr[15] = atoi(value);
                                else if ( !strcasecmp(arg, "vol+r") )
                                    config->skin.mjr[16] = atoi(value);
                                else

                                    /* .. button layout .. */

                                    if ( !strcasecmp(arg, "skipbi") ) {
                                        config->skin.mi[0] = (char*)realloc(config->skin.mi[0], strlen(value)+1);
                                        cpy_replace(config->skin.mi[0], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "skipba") ) {
                                        config->skin.ma[0] = (char*)realloc(config->skin.ma[0], strlen(value)+1);
                                        cpy_replace(config->skin.ma[0], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "playi") ) {
                                        config->skin.mi[1] = (char*)realloc(config->skin.mi[1], strlen(value)+1);
                                        cpy_replace(config->skin.mi[1], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "playa") ) {
                                        config->skin.ma[1] = (char*)realloc(config->skin.ma[1], strlen(value)+1);
                                        cpy_replace(config->skin.ma[1], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "skipfi") ) {
                                        config->skin.mi[2] = (char*)realloc(config->skin.mi[2], strlen(value)+1);
                                        cpy_replace(config->skin.mi[2], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "skipfa") ) {
                                        config->skin.ma[2] = (char*)realloc(config->skin.ma[2], strlen(value)+1);
                                        cpy_replace(config->skin.ma[2], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "stopi") ) {
                                        config->skin.mi[3] =(char*) malloc(strlen(value)+1);
                                        cpy_replace(config->skin.mi[3], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "stopa") ) {
                                        config->skin.ma[3] = (char*)realloc(config->skin.ma[3], strlen(value)+1);
                                        cpy_replace(config->skin.ma[3], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "modei") ) {
                                        config->skin.mi[4] = (char*)realloc(config->skin.mi[4], strlen(value)+1);
                                        cpy_replace(config->skin.mi[4], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "modea") ) {
                                        config->skin.ma[4] = (char*)realloc(config->skin.ma[4], strlen(value)+1);
                                        cpy_replace(config->skin.ma[4], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "pli") ) {
                                        config->skin.mi[5] = (char*)realloc(config->skin.mi[5], strlen(value)+1);
                                        cpy_replace(config->skin.mi[5], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "pla") ) {
                                        config->skin.ma[5] = (char*)realloc(config->skin.ma[5], strlen(value)+1);
                                        cpy_replace(config->skin.ma[5], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "custi") ) {
                                        config->skin.mi[6] = (char*)realloc(config->skin.mi[6], strlen(value)+1);
                                        cpy_replace(config->skin.mi[6], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "custa") ) {
                                        config->skin.ma[6] = (char*)realloc(config->skin.ma[6], strlen(value)+1);
                                        cpy_replace(config->skin.ma[6], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "seekbi") ) {
                                        config->skin.mi[7] = (char*)realloc(config->skin.mi[7], strlen(value)+1);
                                        cpy_replace(config->skin.mi[7], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "seekba") ) {
                                        config->skin.ma[7] = (char*)realloc(config->skin.ma[7], strlen(value)+1);
                                        cpy_replace(config->skin.ma[7], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "pausei") ) {
                                        config->skin.mi[8] = (char*)realloc(config->skin.mi[8], strlen(value)+1);
                                        cpy_replace(config->skin.mi[8], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "pausea") ) {
                                        config->skin.ma[8] = (char*)realloc(config->skin.ma[8], strlen(value)+1);
                                        cpy_replace(config->skin.ma[8], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "seekfi") ) {
                                        config->skin.mi[9] = (char*)realloc(config->skin.mi[9], strlen(value)+1);
                                        cpy_replace(config->skin.mi[9], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "seekfa") ) {
                                        config->skin.ma[9] = (char*)realloc(config->skin.ma[9], strlen(value)+1);
                                        cpy_replace(config->skin.ma[9], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "jumpi") ) {
                                        config->skin.mi[10] = (char*)realloc(config->skin.mi[10], strlen(value)+1);
                                        cpy_replace(config->skin.mi[10], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "jumpa") ) {
                                        config->skin.ma[10] = (char*)realloc(config->skin.ma[10], strlen(value)+1);
                                        cpy_replace(config->skin.ma[10], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "locki") ) {
                                        config->skin.mi[11] = (char*)realloc(config->skin.mi[11], strlen(value)+1);
                                        cpy_replace(config->skin.mi[11], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "locka") ) {
                                        config->skin.ma[11] = (char*)realloc(config->skin.ma[11], strlen(value)+1);
                                        cpy_replace(config->skin.ma[11], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "desci") ) {
                                        config->skin.mi[12] = (char*)realloc(config->skin.mi[12], strlen(value)+1);
                                        cpy_replace(config->skin.mi[12], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "desca") ) {
                                        config->skin.ma[12] = (char*)realloc(config->skin.ma[12], strlen(value)+1);
                                        cpy_replace(config->skin.ma[12], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "forki") ) {
                                        config->skin.mi[13] = (char*)realloc(config->skin.mi[13], strlen(value)+1);
                                        cpy_replace(config->skin.mi[13], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "forka") ) {
                                        config->skin.ma[13] = (char*)realloc(config->skin.ma[13], strlen(value)+1);
                                        cpy_replace(config->skin.ma[13], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "mutei") ) {
                                        config->skin.mi[14] = (char*)realloc(config->skin.mi[14], strlen(value)+1);
                                        cpy_replace(config->skin.mi[14], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "mutea") ) {
                                        config->skin.ma[14] = (char*)realloc(config->skin.ma[14], strlen(value)+1);
                                        cpy_replace(config->skin.ma[14], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "vol-i") ) {
                                        config->skin.mi[15] = (char*)realloc(config->skin.mi[15], strlen(value)+1);
                                        cpy_replace(config->skin.mi[15], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "vol-a") ) {
                                        config->skin.ma[15] = (char*)realloc(config->skin.ma[15], strlen(value)+1);
                                        cpy_replace(config->skin.ma[15], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "vol+i") ) {
                                        config->skin.mi[16] = (char*)realloc(config->skin.mi[16], strlen(value)+1);
                                        cpy_replace(config->skin.mi[16], value, '&', 27);
                                    } else if ( !strcasecmp(arg, "vol+a") ) {
                                        config->skin.ma[16] = (char*)realloc(config->skin.ma[16], strlen(value)+1);
                                        cpy_replace(config->skin.ma[16], value, '&', 27);
                                    } else

                                        /* .. and locations .. */

                                        if ( !strcasecmp(arg, "skipbx") )
                                            config->skin.mx[0] = atoi(value);
                                        else if ( !strcasecmp(arg, "skipby") )
                                            config->skin.my[0] = atoi(value);
                                        else if ( !strcasecmp(arg, "skipbw") )
                                            config->skin.mw[0] = atoi(value);
                                        else if ( !strcasecmp(arg, "playx") )
                                            config->skin.mx[1] = atoi(value);
                                        else if ( !strcasecmp(arg, "playy") )
                                            config->skin.my[1] = atoi(value);
                                        else if ( !strcasecmp(arg, "playw") )
                                            config->skin.mw[1] = atoi(value);
                                        else if ( !strcasecmp(arg, "skipfx") )
                                            config->skin.mx[2] = atoi(value);
                                        else if ( !strcasecmp(arg, "skipfy") )
                                            config->skin.my[2] = atoi(value);
                                        else if ( !strcasecmp(arg, "skipfw") )
                                            config->skin.mw[2] = atoi(value);
                                        else if ( !strcasecmp(arg, "stopx") )
                                            config->skin.mx[3] = atoi(value);
                                        else if ( !strcasecmp(arg, "stopy") )
                                            config->skin.my[3] = atoi(value);
                                        else if ( !strcasecmp(arg, "stopw") )
                                            config->skin.mw[3] = atoi(value);
                                        else if ( !strcasecmp(arg, "modex") )
                                            config->skin.mx[4] = atoi(value);
                                        else if ( !strcasecmp(arg, "modey") )
                                            config->skin.my[4] = atoi(value);
                                        else if ( !strcasecmp(arg, "modew") )
                                            config->skin.mw[4] = atoi(value);
                                        else if ( !strcasecmp(arg, "plx") )
                                            config->skin.mx[5] = atoi(value);
                                        else if ( !strcasecmp(arg, "ply") )
                                            config->skin.my[5] = atoi(value);
                                        else if ( !strcasecmp(arg, "plw") )
                                            config->skin.mw[5] = atoi(value);
                                        else if ( !strcasecmp(arg, "custx") )
                                            config->skin.mx[6] = atoi(value);
                                        else if ( !strcasecmp(arg, "custy") )
                                            config->skin.my[6] = atoi(value);
                                        else if ( !strcasecmp(arg, "custw") )
                                            config->skin.mw[6] = atoi(value);
                                        else if ( !strcasecmp(arg, "seekbx") )
                                            config->skin.mx[7] = atoi(value);
                                        else if ( !strcasecmp(arg, "seekby") )
                                            config->skin.my[7] = atoi(value);
                                        else if ( !strcasecmp(arg, "seekbw") )
                                            config->skin.mw[7] = atoi(value);
                                        else if ( !strcasecmp(arg, "pausex") )
                                            config->skin.mx[8] = atoi(value);
                                        else if ( !strcasecmp(arg, "pausey") )
                                            config->skin.my[8] = atoi(value);
                                        else if ( !strcasecmp(arg, "pausew") )
                                            config->skin.mw[8] = atoi(value);
                                        else if ( !strcasecmp(arg, "seekfx") )
                                            config->skin.mx[9] = atoi(value);
                                        else if ( !strcasecmp(arg, "seekfy") )
                                            config->skin.my[9] = atoi(value);
                                        else if ( !strcasecmp(arg, "seekfw") )
                                            config->skin.mw[9] = atoi(value);
                                        else if ( !strcasecmp(arg, "jumpx") )
                                            config->skin.mx[10] = atoi(value);
                                        else if ( !strcasecmp(arg, "jumpy") )
                                            config->skin.my[10] = atoi(value);
                                        else if ( !strcasecmp(arg, "jumpw") )
                                            config->skin.mw[10] = atoi(value);
                                        else if ( !strcasecmp(arg, "lockx") )
                                            config->skin.mx[11] = atoi(value);
                                        else if ( !strcasecmp(arg, "locky") )
                                            config->skin.my[11] = atoi(value);
                                        else if ( !strcasecmp(arg, "lockw") )
                                            config->skin.mw[11] = atoi(value);
                                        else if ( !strcasecmp(arg, "descx") )
                                            config->skin.mx[12] = atoi(value);
                                        else if ( !strcasecmp(arg, "descy") )
                                            config->skin.my[12] = atoi(value);
                                        else if ( !strcasecmp(arg, "descw") )
                                            config->skin.mw[12] = atoi(value);
                                        else if ( !strcasecmp(arg, "forkx") )
                                            config->skin.mx[13] = atoi(value);
                                        else if ( !strcasecmp(arg, "forky") )
                                            config->skin.my[13] = atoi(value);
                                        else if ( !strcasecmp(arg, "forkw") )
                                            config->skin.mw[13] = atoi(value);
                                        else if ( !strcasecmp(arg, "mutex") )
                                            config->skin.mx[14] = atoi(value);
                                        else if ( !strcasecmp(arg, "mutey") )
                                            config->skin.my[14] = atoi(value);
                                        else if ( !strcasecmp(arg, "mutew") )
                                            config->skin.mw[14] = atoi(value);
                                        else if ( !strcasecmp(arg, "vol-x") )
                                            config->skin.mx[15] = atoi(value);
                                        else if ( !strcasecmp(arg, "vol-y") )
                                            config->skin.my[15] = atoi(value);
                                        else if ( !strcasecmp(arg, "vol-w") )
                                            config->skin.mw[15] = atoi(value);
                                        else if ( !strcasecmp(arg, "vol+x") )
                                            config->skin.mx[16] = atoi(value);
                                        else if ( !strcasecmp(arg, "vol+y") )
                                            config->skin.my[16] = atoi(value);
                                        else if ( !strcasecmp(arg, "vol+w") )
                                            config->skin.mw[16] = atoi(value);

//      for(i=MINBUTTON;i<(MAXBUTTON+1);i++)
//	printf("%d: %c\n", i, config->skin.mh[i]);


    }

    if ( version == 0 ) {
        printf("Skin version not found, possible error while parsing skin configuration file?\n");
        exit(-1);
    }

    fclose(filefd);

    /* playlist skin configuration */

    sprintf(buf, "%s/playlist.conf", skin_home);
    filefd = fopen(buf, "r");
    if ( !filefd ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }

    while ( fgets((char*)buf, CONF_BUF_LEN, filefd) != NULL ) { /* parse skin configuration */
        if ( !parseconfig(buf, &arg, &value) ) continue;

        if ( !strcasecmp(arg, "clearscreen" ) )
            if ( !strcasecmp(value, "true") ) config->skin.pclr = TRUE;
            else
                config->skin.pclr = FALSE;
        else if ( !strcasecmp(arg, "startbutton") )
            config->skin.psb = atoi(value);
        else if ( !strcasecmp(arg, "buttonorder") ) {
            strcpy(config->skin.plistbo, value);
            for(i=PL_MINBUTTON; i<(PL_MAXBUTTON+1); i++) {
                config->skin.plistbo[i] -= 48;
                if ( config->skin.plistbo[i] < 0 || config->skin.plistbo[i] > 9 ) config->skin.plistbo[i] = -1;
            }
        } else if ( !strcasecmp(arg, "clearscreenafterid3" ) )
            if ( !strcasecmp(value, "true") ) config->skin.piclr = TRUE;
            else
                config->skin.piclr = FALSE;
        else if ( !strcasecmp(arg, "mainvisible") )
            config->skin.mainatpl = atoi(value);
        else if ( !strcasecmp(arg, "id3editvisible") )
            config->skin.id3atpl = atoi(value);
        else if ( !strcasecmp(arg, "mouseexpand") )
            config->skin.pmouseexpand = atoi(value);
        else if ( !strcasecmp(arg, "listlines") )
            config->skin.plistlines = atoi(value);
        else if ( !strcasecmp(arg, "listx") )
            config->skin.plistx = atoi(value);
        else if ( !strcasecmp(arg, "listy") )
            config->skin.plisty = atoi(value);
        else if ( !strcasecmp(arg, "listw") )
            config->skin.plistw = atoi(value);
        else

            if ( !strcasecmp(arg, "listcolori") ) {
                config->skin.plistci = (char*)realloc(config->skin.plistci, strlen(value)+1);
                cpy_replace(config->skin.plistci, value, '&', 27);
            } else if ( !strcasecmp(arg, "listcolora") ) {
                config->skin.plistca = (char*)realloc(config->skin.plistca, strlen(value)+1);
                cpy_replace(config->skin.plistca, value, '&', 27);
            } else if ( !strcasecmp(arg, "listcolorc") ) {
                config->skin.plistcc = (char*)realloc(config->skin.plistcc, strlen(value)+1);
                cpy_replace(config->skin.plistcc, value, '&', 27);
            } else

                if ( !strcasecmp(arg, "findbarx") )
                    config->skin.findbarx = atoi(value);
                else if ( !strcasecmp(arg, "findbary") )
                    config->skin.findbary = atoi(value);
                else if ( !strcasecmp(arg, "findbarw") )
                    config->skin.findbarw = atoi(value);
                else if ( !strcasecmp(arg, "findbarc") ) {
                    config->skin.findbarc = (char*)realloc(config->skin.findbarc, strlen(value)+1);
                    cpy_replace(config->skin.findbarc, value, '&', 27);
                } else

                    if ( !strcasecmp(arg, "browsex") )
                        config->skin.px[0] = atoi(value);
                    else if ( !strcasecmp(arg, "browsey") )
                        config->skin.py[0] = atoi(value);
                    else if ( !strcasecmp(arg, "browsew") )
                        config->skin.pw[0] = atoi(value);
                    else if ( !strcasecmp(arg, "playx") )
                        config->skin.px[1] = atoi(value);
                    else if ( !strcasecmp(arg, "playy") )
                        config->skin.py[1] = atoi(value);
                    else if ( !strcasecmp(arg, "playw") )
                        config->skin.pw[1] = atoi(value);
                    else if ( !strcasecmp(arg, "removex") )
                        config->skin.px[2] = atoi(value);
                    else if ( !strcasecmp(arg, "removey") )
                        config->skin.py[2] = atoi(value);
                    else if ( !strcasecmp(arg, "removew") )
                        config->skin.pw[2] = atoi(value);
                    else if ( !strcasecmp(arg, "newlstx") )
                        config->skin.px[3] = atoi(value);
                    else if ( !strcasecmp(arg, "newlsty") )
                        config->skin.py[3] = atoi(value);
                    else if ( !strcasecmp(arg, "newlstw") )
                        config->skin.pw[3] = atoi(value);
                    else if ( !strcasecmp(arg, "descx") )
                        config->skin.px[4] = atoi(value);
                    else if ( !strcasecmp(arg, "descy") )
                        config->skin.py[4] = atoi(value);
                    else if ( !strcasecmp(arg, "descw") )
                        config->skin.pw[4] = atoi(value);
                    else if ( !strcasecmp(arg, "sortx") )
                        config->skin.px[5] = atoi(value);
                    else if ( !strcasecmp(arg, "sorty") )
                        config->skin.py[5] = atoi(value);
                    else if ( !strcasecmp(arg, "sortw") )
                        config->skin.pw[5] = atoi(value);
                    else if ( !strcasecmp(arg, "backx") )
                        config->skin.px[6] = atoi(value);
                    else if ( !strcasecmp(arg, "backy") )
                        config->skin.py[6] = atoi(value);
                    else if ( !strcasecmp(arg, "backw") )
                        config->skin.pw[6] = atoi(value);
                    else

                        if ( !strcasecmp(arg, "browsei") ) {
                            config->skin.pi[0] = (char*)realloc(config->skin.pi[0], strlen(value)+1);
                            cpy_replace(config->skin.pi[0], value, '&', 27);
                        } else if ( !strcasecmp(arg, "browsea") ) {
                            config->skin.pa[0] = (char*)realloc(config->skin.pa[0], strlen(value)+1);
                            cpy_replace(config->skin.pa[0], value, '&', 27);
                        } else if ( !strcasecmp(arg, "playi") ) {
                            config->skin.pi[1] = (char*)realloc(config->skin.pi[1], strlen(value)+1);
                            cpy_replace(config->skin.pi[1], value, '&', 27);
                        } else if ( !strcasecmp(arg, "playa") ) {
                            config->skin.pa[1] = (char*)realloc(config->skin.pa[1], strlen(value)+1);
                            cpy_replace(config->skin.pa[1], value, '&', 27);
                        } else if ( !strcasecmp(arg, "removei") ) {
                            config->skin.pi[2] = (char*)realloc(config->skin.pi[2], strlen(value)+1);
                            cpy_replace(config->skin.pi[2], value, '&', 27);
                        } else if ( !strcasecmp(arg, "removea") ) {
                            config->skin.pa[2] = (char*)realloc(config->skin.pa[2], strlen(value)+1);
                            cpy_replace(config->skin.pa[2], value, '&', 27);
                        } else if ( !strcasecmp(arg, "newlsti") ) {
                            config->skin.pi[3] = (char*)realloc(config->skin.pi[3], strlen(value)+1);
                            cpy_replace(config->skin.pi[3], value, '&', 27);
                        } else if ( !strcasecmp(arg, "newlsta") ) {
                            config->skin.pa[3] = (char*)realloc(config->skin.pa[3], strlen(value)+1);
                            cpy_replace(config->skin.pa[3], value, '&', 27);
                        } else if ( !strcasecmp(arg, "desci") ) {
                            config->skin.pi[4] = (char*)realloc(config->skin.pi[4], strlen(value)+1);
                            cpy_replace(config->skin.pi[4], value, '&', 27);
                        } else if ( !strcasecmp(arg, "desca") ) {
                            config->skin.pa[4] = (char*)realloc(config->skin.pa[4], strlen(value)+1);
                            cpy_replace(config->skin.pa[4], value, '&', 27);
                        } else if ( !strcasecmp(arg, "sorti") ) {
                            config->skin.pi[5] = (char*)realloc(config->skin.pi[5], strlen(value)+1);
                            cpy_replace(config->skin.pi[5], value, '&', 27);
                        } else if ( !strcasecmp(arg, "sorta") ) {
                            config->skin.pa[5] = (char*)realloc(config->skin.pa[5], strlen(value)+1);
                            cpy_replace(config->skin.pa[5], value, '&', 27);
                        } else if ( !strcasecmp(arg, "backi") ) {
                            config->skin.pi[6] = (char*)realloc(config->skin.pi[6], strlen(value)+1);
                            cpy_replace(config->skin.pi[6], value, '&', 27);
                        } else if ( !strcasecmp(arg, "backa") ) {
                            config->skin.pa[6] = (char*)realloc(config->skin.pa[6], strlen(value)+1);
                            cpy_replace(config->skin.pa[6], value, '&', 27);
                        }

    }
    fclose(filefd);

    /* filebrowser skin configuration */

    sprintf(buf, "%s/filelist.conf", skin_home);
    filefd = fopen(buf, "r");
    if ( !filefd ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }

    while ( fgets((char*)buf, CONF_BUF_LEN, filefd) != NULL ) { /* parse skin configuration */
        if ( !parseconfig(buf, &arg, &value) ) continue;

        if ( !strcasecmp(arg, "clearscreen" ) )
            if ( !strcasecmp(value, "true") ) config->skin.fclr = TRUE;
            else
                config->skin.fclr = FALSE;
        else if ( !strcasecmp(arg, "startbutton") )
            config->skin.fsb = atoi(value);
        else if ( !strcasecmp(arg, "buttonorder") ) {
            strcpy(config->skin.flistbo, value);
            for(i=FL_MINBUTTON; i<(FL_MAXBUTTON+1); i++) {
                config->skin.flistbo[i] -= 48;
                if ( config->skin.flistbo[i] < 0 || config->skin.flistbo[i] > 9 ) config->skin.flistbo[i] = -1;
            }
        } else if ( !strcasecmp(arg, "textcolor") ) {
            config->skin.textc = (char*)realloc(config->skin.textc, strlen(value)+1);
            cpy_replace(config->skin.textc, value, '&', 27);
        } else if ( !strcasecmp(arg, "textx") )
            config->skin.textx = atoi(value);
        else if ( !strcasecmp(arg, "texty") )
            config->skin.texty = atoi(value);
        else if ( !strcasecmp(arg, "textw") )
            config->skin.textw = atoi(value);
        else if ( !strcasecmp(arg, "loading") ) {
            config->skin.flistmsg[0] = (char*)realloc(config->skin.flistmsg[0], strlen(value)+1);
            cpy_replace(config->skin.flistmsg[0], value, '&', 27);
        } else if ( !strcasecmp(arg, "direrror") ) {
            config->skin.flistmsg[1] = (char*)realloc(config->skin.flistmsg[1], strlen(value)+1);
            cpy_replace(config->skin.flistmsg[1], value, '&', 27);
        } else if ( !strcasecmp(arg, "wrongplaylist") ) {
            config->skin.flistmsg[2] = (char*)realloc(config->skin.flistmsg[2], strlen(value)+1);
            cpy_replace(config->skin.flistmsg[2], value, '&', 27);
        } else if ( !strcasecmp(arg, "saveas") ) {
            config->skin.flistmsg[3] = (char*)realloc(config->skin.flistmsg[3], strlen(value)+1);
            cpy_replace(config->skin.flistmsg[3], value, '&', 27);
        } else if ( !strcasecmp(arg, "saveid3tags") ) {
            config->skin.flistmsg[4] = (char*)realloc(config->skin.flistmsg[4], strlen(value)+1);
            cpy_replace(config->skin.flistmsg[4], value, '&', 27);
        } else if ( !strcasecmp(arg, "saving") ) {
            config->skin.flistmsg[5] = (char*)realloc(config->skin.flistmsg[5], strlen(value)+1);
            cpy_replace(config->skin.flistmsg[5], value, '&', 27);
        } else if ( !strcasecmp(arg, "mouseexpand") )
            config->skin.fmouseexpand = atoi(value);
        else if ( !strcasecmp(arg, "listlines") )
            config->skin.flistlines = atoi(value);
        else if ( !strcasecmp(arg, "listx") )
            config->skin.flistx = atoi(value);
        else if ( !strcasecmp(arg, "listy") )
            config->skin.flisty = atoi(value);
        else if ( !strcasecmp(arg, "listw") )
            config->skin.flistw = atoi(value);
        else

            if ( !strcasecmp(arg, "listcolorfilei") ) {
                config->skin.flistcfi = (char*)realloc(config->skin.flistcfi, strlen(value)+1);
                cpy_replace(config->skin.flistcfi, value, '&', 27);
            } else if ( !strcasecmp(arg, "listcolorfilea") ) {
                config->skin.flistcfa = (char*)realloc(config->skin.flistcfa, strlen(value)+1);
                cpy_replace(config->skin.flistcfa, value, '&', 27);
            } else if ( !strcasecmp(arg, "listcolordiri") ) {
                config->skin.flistcdi = (char*)realloc(config->skin.flistcdi, strlen(value)+1);
                cpy_replace(config->skin.flistcdi, value, '&', 27);
            } else if ( !strcasecmp(arg, "listcolordira") ) {
                config->skin.flistcda = (char*)realloc(config->skin.flistcda, strlen(value)+1);
                cpy_replace(config->skin.flistcda, value, '&', 27);
            } else if ( !strcasecmp(arg, "listtagi") ) {
                config->skin.flistcti = (char*)realloc(config->skin.flistcti, strlen(value)+1);
                cpy_replace(config->skin.flistcti, value, '&', 27);
            } else if ( !strcasecmp(arg, "listtaga") ) {
                config->skin.flistcta = (char*)realloc(config->skin.flistcta, strlen(value)+1);
                cpy_replace(config->skin.flistcta, value, '&', 27);
            } else

                if ( !strcasecmp(arg, "addx") )
                    config->skin.fx[0] = atoi(value);
                else if ( !strcasecmp(arg, "addy") )
                    config->skin.fy[0] = atoi(value);
                else if ( !strcasecmp(arg, "addw") )
                    config->skin.fw[0] = atoi(value);
                else if ( !strcasecmp(arg, "tagx") )
                    config->skin.fx[1] = atoi(value);
                else if ( !strcasecmp(arg, "tagy") )
                    config->skin.fy[1] = atoi(value);
                else if ( !strcasecmp(arg, "tagw") )
                    config->skin.fw[1] = atoi(value);
                else if ( !strcasecmp(arg, "untagx") )
                    config->skin.fx[2] = atoi(value);
                else if ( !strcasecmp(arg, "untagy") )
                    config->skin.fy[2] = atoi(value);
                else if ( !strcasecmp(arg, "untagw") )
                    config->skin.fw[2] = atoi(value);
                else if ( !strcasecmp(arg, "loadpx") )
                    config->skin.fx[3] = atoi(value);
                else if ( !strcasecmp(arg, "loadpy") )
                    config->skin.fy[3] = atoi(value);
                else if ( !strcasecmp(arg, "loadpw") )
                    config->skin.fw[3] = atoi(value);
                else if ( !strcasecmp(arg, "savepx") )
                    config->skin.fx[4] = atoi(value);
                else if ( !strcasecmp(arg, "savepy") )
                    config->skin.fy[4] = atoi(value);
                else if ( !strcasecmp(arg, "savepw") )
                    config->skin.fw[4] = atoi(value);
                else if ( !strcasecmp(arg, "clearx") )
                    config->skin.fx[5] = atoi(value);
                else if ( !strcasecmp(arg, "cleary") )
                    config->skin.fy[5] = atoi(value);
                else if ( !strcasecmp(arg, "clearw") )
                    config->skin.fw[5] = atoi(value);
                else if ( !strcasecmp(arg, "backx") )
                    config->skin.fx[6] = atoi(value);
                else if ( !strcasecmp(arg, "backy") )
                    config->skin.fy[6] = atoi(value);
                else if ( !strcasecmp(arg, "backw") )
                    config->skin.fw[6] = atoi(value);
                else if ( !strcasecmp(arg, "direx") )
                    config->skin.fx[7] = atoi(value);
                else if ( !strcasecmp(arg, "direy") )
                    config->skin.fy[7] = atoi(value);
                else if ( !strcasecmp(arg, "direw") )
                    config->skin.fw[7] = atoi(value);
                else

                    if ( !strcasecmp(arg, "addi") ) {
                        config->skin.fi[0] = (char*)realloc(config->skin.fi[0], strlen(value)+1);
                        cpy_replace(config->skin.fi[0], value, '&', 27);
                    } else if ( !strcasecmp(arg, "adda") ) {
                        config->skin.fa[0] = (char*)realloc(config->skin.fa[0], strlen(value)+1);
                        cpy_replace(config->skin.fa[0], value, '&', 27);
                    } else if ( !strcasecmp(arg, "tagi") ) {
                        config->skin.fi[1] = (char*)realloc(config->skin.fi[1], strlen(value)+1);
                        cpy_replace(config->skin.fi[1], value, '&', 27);
                    } else if ( !strcasecmp(arg, "taga") ) {
                        config->skin.fa[1] = (char*)realloc(config->skin.fa[1], strlen(value)+1);
                        cpy_replace(config->skin.fa[1], value, '&', 27);
                    } else if ( !strcasecmp(arg, "untagi") ) {
                        config->skin.fi[2] = (char*)realloc(config->skin.fi[2], strlen(value)+1);
                        cpy_replace(config->skin.fi[2], value, '&', 27);
                    } else if ( !strcasecmp(arg, "untaga") ) {
                        config->skin.fa[2] = (char*)realloc(config->skin.fa[2], strlen(value)+1);
                        cpy_replace(config->skin.fa[2], value, '&', 27);
                    } else if ( !strcasecmp(arg, "loadpi") ) {
                        config->skin.fi[3] = (char*)realloc(config->skin.fi[3], strlen(value)+1);
                        cpy_replace(config->skin.fi[3], value, '&', 27);
                    } else if ( !strcasecmp(arg, "loadpa") ) {
                        config->skin.fa[3] = (char*)realloc(config->skin.fa[3], strlen(value)+1);
                        cpy_replace(config->skin.fa[3], value, '&', 27);
                    } else if ( !strcasecmp(arg, "savepi") ) {
                        config->skin.fi[4] = (char*)realloc(config->skin.fi[4], strlen(value)+1);
                        cpy_replace(config->skin.fi[4], value, '&', 27);
                    } else if ( !strcasecmp(arg, "savepa") ) {
                        config->skin.fa[4] = (char*)realloc(config->skin.fa[4], strlen(value)+1);
                        cpy_replace(config->skin.fa[4], value, '&', 27);
                    } else if ( !strcasecmp(arg, "cleari") ) {
                        config->skin.fi[5] = (char*)realloc(config->skin.fi[5], strlen(value)+1);
                        cpy_replace(config->skin.fi[5], value, '&', 27);
                    } else if ( !strcasecmp(arg, "cleara") ) {
                        config->skin.fa[5] = (char*)realloc(config->skin.fa[5], strlen(value)+1);
                        cpy_replace(config->skin.fa[5], value, '&', 27);
                    } else if ( !strcasecmp(arg, "backi") ) {
                        config->skin.fi[6] = (char*)realloc(config->skin.fi[6], strlen(value)+1);
                        cpy_replace(config->skin.fi[6], value, '&', 27);
                    } else if ( !strcasecmp(arg, "backa") ) {
                        config->skin.fa[6] = (char*)realloc(config->skin.fa[6], strlen(value)+1);
                        cpy_replace(config->skin.fa[6], value, '&', 27);
                    } else if ( !strcasecmp(arg, "direi") ) {
                        config->skin.fi[7] = (char*)realloc(config->skin.fi[7], strlen(value)+1);
                        cpy_replace(config->skin.fi[7], value, '&', 27);
                    } else if ( !strcasecmp(arg, "direa") ) {
                        config->skin.fa[7] = (char*)realloc(config->skin.fa[7], strlen(value)+1);
                        cpy_replace(config->skin.fa[7], value, '&', 27);
                    }
    }
    fclose(filefd);

    /* id3-tag editor skin configuration */

    sprintf(buf, "%s/id3.conf", skin_home);
    filefd = fopen(buf, "r");
    if ( !filefd ) {
        printf("error! %s not found!\n", buf);
        exit(-1);
    }

    while ( fgets((char*)buf, CONF_BUF_LEN, filefd) != NULL ) { /* parse skin configuration */
        if ( !parseconfig(buf, &arg, &value) ) continue;

        if ( !strcasecmp(arg, "clearscreen" ) )
            if ( !strcasecmp(value, "true") ) config->skin.iclr = TRUE;
            else
                config->skin.iclr = FALSE;
        else if ( !strcasecmp(arg, "filenamec") ) {
            config->skin.id3fnc = (char*)realloc(config->skin.id3fnc, strlen(value)+1);
            cpy_replace(config->skin.id3fnc, value, '&', 27);
        } else if ( !strcasecmp(arg, "filenamew") )
            config->skin.id3fnw = atoi(value);
        else if ( !strcasecmp(arg, "textcolor") ) {
            config->skin.id3tc = (char*)realloc(config->skin.id3tc, strlen(value)+1);
            cpy_replace(config->skin.id3tc, value, '&', 27);
        } else if ( !strcasecmp(arg, "savec") ) {
            config->skin.id3sc = (char*)realloc(config->skin.id3sc, strlen(value)+1);
            cpy_replace(config->skin.id3sc, value, '&', 27);
        } else if ( !strcasecmp(arg, "savet") ) {
            config->skin.id3st = (char*)realloc(config->skin.id3st, strlen(value)+1);
            cpy_replace(config->skin.id3st, value, '&', 27);
        } else if ( !strcasecmp(arg, "artistx") )
            config->skin.ix[0] = atoi(value);
        else if ( !strcasecmp(arg, "artisty") )
            config->skin.iy[0] = atoi(value);
        else if ( !strcasecmp(arg, "songnamex") )
            config->skin.ix[1] = atoi(value);
        else if ( !strcasecmp(arg, "songnamey") )
            config->skin.iy[1] = atoi(value);
        else if ( !strcasecmp(arg, "commentx") )
            config->skin.ix[2] = atoi(value);
        else if ( !strcasecmp(arg, "commenty") )
            config->skin.iy[2] = atoi(value);
        else if ( !strcasecmp(arg, "albumx") )
            config->skin.ix[3] = atoi(value);
        else if ( !strcasecmp(arg, "albumy") )
            config->skin.iy[3] = atoi(value);
        else if ( !strcasecmp(arg, "yearx") )
            config->skin.ix[4] = atoi(value);
        else if ( !strcasecmp(arg, "yeary") )
            config->skin.iy[4] = atoi(value);
        else if ( !strcasecmp(arg, "filenamex") )
            config->skin.ix[5] = atoi(value);
        else if ( !strcasecmp(arg, "filenamey") )
            config->skin.iy[5] = atoi(value);
        if ( !strcasecmp(arg, "savex") )
            config->skin.ix[6] = atoi(value);
        else if ( !strcasecmp(arg, "savey") )
            config->skin.iy[6] = atoi(value);
    }
    fclose(filefd);

    for(i=0; i<14; i++) { /* check that everything is ok (kinda bad checks..) */
        if ( i == 7 || i == 9 ) continue;
        if ( !config->skin.ma[i] ) {
            printf("error, active button layout missing in main.conf!\n");
            exit(-1);
        }
        if ( !config->skin.mi[i] ) {
            printf("error, inactive button layout missing in main.conf!\n");
            exit(-1);
        }
        if ( !(config->skin.mju[i]+config->skin.mjd[i]+config->skin.mjl[i]+config->skin.mjr[i]) ) {
            printf("error, button movement missing in main.conf!\n");
            exit(-1);
        }
    }

    fd = 0;
    for(i=0; i<14; i++) fd += config->skin.mx[i];
    if ( !fd ) {
        printf("error, button X location(s) missing in main.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<14; i++) fd += config->skin.my[i];
    if ( !fd ) {
        printf("error, button Y location(s) missing in main.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<14; i++) fd += config->skin.mw[i];
    if ( !fd ) {
        printf("error, button width(s) missing in main.conf!\n");
        exit(-1);
    }
    fd = 0;

    for(i=0; i<7; i++) fd += config->skin.px[i];
    if ( !fd ) {
        printf("error, button X location(s) missing in playlist.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.py[i];
    if ( !fd ) {
        printf("error, button Y location(s) missing in playlist.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.pw[i];
    if ( !fd ) {
        printf("error, button width(s) missing in playlist.conf!\n");
        exit(-1);
    }

    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.fx[i];
    if ( !fd ) {
        printf("error, button X location(s) missing in filelist.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.fy[i];
    if ( !fd ) {
        printf("error, button Y location(s) missing in filelist.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.fw[i];
    if ( !fd ) {
        printf("error, button width(s) missing in filelist.conf!\n");
        exit(-1);
    }

    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.ix[i];
    if ( !fd ) {
        printf("error, text X location(s) missing in id3.conf!\n");
        exit(-1);
    }
    fd = 0;
    for(i=0; i<7; i++) fd += config->skin.iy[i];
    if ( !fd ) {
        printf("error, text Y location(s) missing in id3.conf!\n");
        exit(-1);
    }

    for(i=0; i<7; i++) {
        if ( !config->skin.pa[i] ) {
            printf("error, active button layout(s) missing in playlist.conf!\n");
            exit(-1);
        }
        if ( !config->skin.pi[i] ) {
            printf("error, inactive button layout(s) missing in playlist.conf!\n");
            exit(-1);
        }

        if ( !config->skin.fa[i] ) {
            printf("error, active button layout(s) missing in filelist.conf!\n");
            exit(-1);
        }
        if ( !config->skin.fi[i] ) {
            printf("error, inactive button layout(s) missing in filelist.conf!\n");
            exit(-1);
        }
    }

    for(i=0; i<4; i++)
        if ( !config->skin.mainmsg[i] ) {
            printf("error, text message missing in main.conf!\n");
            exit(-1);
        }

    for(i=0; i<6; i++)
        if ( !config->skin.flistmsg[i] ) {
            printf("error, text message missing in filelist.conf!\n");
            exit(-1);
        }

    if ( !(config->skin.plistbo[0] + config->skin.plistbo[1]) ) {
        printf("error, button order missing (or illegal) in playlist.conf!\n");
        exit(-1);
    }
    if ( !(config->skin.flistbo[0] + config->skin.flistbo[1]) ) {
        printf("error, button order missing (or illegal) in filelist.conf!\n");
        exit(-1);
    }

    /* we do not check everything... (yet)  */

    free(buf);
    printf("done\n");
}

int parseconfig(const char *str, char **arg, char **value)
{
    int i = 0, j;

    while (IS_SPACE(str[i])) i++;
    if (!str[i] || str[i] == '#' || IS_CRLF(str[i])) return FALSE;
    *arg = (char*)&str[i]; j = i;
    while (str[i] && str[i] != '=' && !IS_CRLF(str[i])) i++;
    if((j = i - j)) {
        j--;
        while (j && IS_SPACE((*arg)[j])) j--;
        if (!IS_SPACE((*arg)[j])) j++;
    }
    (*arg)[j] = 0;

    if(str[i]) {
        i++;
        while (IS_SPACE(str[i])) i++;
        if(str[i] == '\"') {
            i++;
            *value = (char*)&str[i]; j = i;
            while (str[i] && str[i] != '\"' && !IS_CRLF(str[i])) i++;
            j = i - j;
        } else {
            *value = (char*)&str[i]; j = i;
            while (str[i] && !IS_CRLF(str[i])) i++;
            if((j = i - j)) {
                j--;
                while (j && IS_SPACE((*value)[j])) j--;
                if (!IS_SPACE((*value)[j])) j++;
            }
        }
        (*value)[j] = 0;
    } else
        *value = (char*)&str[i];

    if (i >= CONF_BUF_LEN - 2)
        printf("Warning: CONF_BUF_LEN is too small");
    return TRUE;
}

char is_val_true(char *value)
{
    switch(value[0]) {

    case '1':
        if ( value[1] == 0 )
            return TRUE;
        break;

    case 'T':
    case 't':
        if ( (value[1] == 'R' || value[1] == 'r' ) \
          && (value[2] == 'U' || value[2] == 'u' ) \
          && (value[3] == 'E' || value[3] == 'e' ) \
          && value[4] == 0 )
            return TRUE;
        break;

    case 'Y':
    case 'y':
        if ( (value[1] == 'E' || value[1] == 'e' ) \
          && (value[2] == 'S' || value[2] == 's' ) \
          && value[3] == 0 )
            return TRUE;
        break;
    }

    return FALSE;
}

char *randomskin(char *buf)
{
    int i;
    FILE *filefd;

    sprintf(buf, "%s/.camp/skins/random", getenv("HOME"));
    filefd = fopen(buf, "r");
    if ( !filefd ) {
        printf("error! %s not found (create it, or choose a skin instead of random)!\n", buf);
        exit(-1);
    }

    fgets((char*)buf, CONF_BUF_LEN, filefd);
    str_strip_end(buf, CONF_BUF_LEN);
    i = myrand(atoi(buf))+1;

    while(i--)
        fgets((char*)buf, CONF_BUF_LEN, filefd);
    str_strip_end(buf, CONF_BUF_LEN);

    fclose(filefd);

    return buf;
}

int probe_dir(char *dir)
{
    struct stat statbuf;
    if (stat(dir, &statbuf) == -1)
        return 0;
    return((statbuf.st_mode & S_IFDIR) == S_IFDIR );
}
