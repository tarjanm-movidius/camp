#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>
#include "camp.h"


unsigned int bitrate_table[3][3][15] = {
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
extern char currloc;

char getmp3info(char *filename, unsigned char *mode, unsigned int *sample_rate, unsigned int *bit_rate, char *name, char *artist, char *misc, char *album, char *year, unsigned char *genre)
{
    FILE *fd;
    struct ID3 filetag;
    unsigned char buf[31];
    int lay;
// int error_protection, padding, extension, mode_ext, copyright, original, emphasis;
    unsigned int bitrate=0, sampling_frequency=0;
    int version;
    char stereo=0;

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
        version = 2;   // Uh? Originally -1   -- inm
    }

    if ( buf[0] == 0xFF && (buf[1] & 0xE0) == 0xE0 )  { /* valid mpX */

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
        if ( bit_rate )    *bit_rate    = bitrate;
        if ( sample_rate ) *sample_rate = sampling_frequency;
        if ( mode )        *mode = stereo;
    } else { /* invalid mpX */
        if ( bit_rate )    *bit_rate     = 0;
        if ( sample_rate ) *sample_rate  = 0;
        if ( mode )        *mode         = 0;
    }

    if ( !strncmp(filetag.tag, "TAG", 3) ) {
        if ( name   ) strcpy(name, fuckspaces(filetag.songname, 30));
        if ( artist ) strcpy(artist, fuckspaces(filetag.artist, 30));
        if ( misc   ) strcpy(misc, fuckspaces(filetag.misc, 30));
        if ( album  ) strcpy(album, fuckspaces(filetag.album, 30));
        if ( year   ) strcpy(year, fuckspaces(filetag.year, 4));
        *genre = filetag.genre;
        return TRUE;
    } else
        return FALSE;
}

char writemp3info(char *filename, char *name, char *artist, char *misc, char *album, char *year, unsigned char genre)
{
    FILE *fd;
    struct ID3 filetag;
    char buf[31];
    int  i;

    fd = fopen(filename, "r+");
    if ( fd == NULL ) return FALSE;
    strcpy(filetag.tag, "TAG");
    strcpy(filetag.songname, name);
    for(i=strlen(name); i<30; i++) filetag.songname[i] = 32;
    strcpy(filetag.artist, artist);
    for(i=strlen(artist); i<30; i++) filetag.artist[i] = 32;
    strcpy(filetag.album, album);
    for(i=strlen(album); i<30; i++) filetag.album[i] = 32;
    strcpy(filetag.year, year);
    for(i=strlen(year); i<4; i++) filetag.year[i] = 32;
    strcpy(filetag.misc, misc);
    for(i=strlen(misc); i<30; i++) filetag.misc[i] = 32;
    filetag.genre = genre;
    fseek(fd, -128, SEEK_END);
    fread(buf, 3, 1, fd);
    if ( strncmp(buf, "TAG", 3) )
        fseek(fd, 0, SEEK_END);
    else
        fseek(fd, -128, SEEK_END);
    fwrite((void*)&filetag, 128, 1, fd);
    fclose(fd);
    return TRUE;
}


char *fuckspaces(char *lame, int maxpos)
{
    int i;
    static char buf[256];
    strcpy(buf, lame);
    buf[maxpos] = 0;
    for (i=maxpos-1; i>-1; i--)
        if ( buf[i] == '\r' ) buf[i] = 0;
    for(i=maxpos-1; i>-1; i--)
        if (lame[i] == ' ') buf[i] = 0;
        else return buf;
    return buf;
}

void id3edit(char *filename, struct playlistent *playlist)
{
    char name[31], artist[31], album[31], misc[31], year[5], buf[256], pos=0;
    int i=0, cspos=0;
    unsigned char genre=0;
    int modified=FALSE, exitchar=0;
    fd_set stdinfds;

    if ( filename == NULL ) return;
    currloc = CAMP_DESC;

    for(; i<strlen(filename); i++)
        if (filename[i] == '/' ) cspos = 0;
        else {
            buf[cspos] = filename[i];
            cspos++;
        }

    if ( cspos > config.skin.id3fnw )
        buf[(int)config.skin.id3fnw] = 0;
    else
        buf[cspos] = 0;

    if ( !getmp3info(filename, NULL, NULL, NULL, name, artist, misc, album, year, &genre) ) {
        name[0]   = 0;
        artist[0] = 0;
        album[0]  = 0;
        misc[0]   = 0;
        year[0]   = 0;
        genre     = 0;
    }
    if ( config.skin.iclr ) printf("\e[0m\e[2J");
    printf("\e[?25h\e[1;1H%s\e[%sm", config.skin.id3, config.skin.id3fnc);
    printf("\e[%d;%dH%s\e[%sm", config.skin.iy[5], config.skin.ix[5], buf, config.skin.id3tc);
    printf("\e[%d;%dH%s", config.skin.iy[0], config.skin.ix[0], artist);
    printf("\e[%d;%dH%s", config.skin.iy[1], config.skin.ix[1], name);
    printf("\e[%d;%dH%s", config.skin.iy[2], config.skin.ix[2], misc);
    printf("\e[%d;%dH%s", config.skin.iy[3], config.skin.ix[3], album);
    printf("\e[%d;%dH%s", config.skin.iy[4], config.skin.ix[4], year);
    fflush(stdout);


    do { /* welcome to switcher's paradise */

        switch(pos) {
        case 0:
            strcpy(artist, readyxline(config.skin.iy[0], config.skin.ix[0], artist, 30, &exitchar, &modified));
            break;
        case 1:
            strcpy(name,   readyxline(config.skin.iy[1], config.skin.ix[1], name, 30, &exitchar, &modified));
            break;
        case 2:
            strcpy(misc,   readyxline(config.skin.iy[2], config.skin.ix[2], misc, 30, &exitchar, &modified));
            break;
        case 3:
            strcpy(album,  readyxline(config.skin.iy[3], config.skin.ix[3], album, 30, &exitchar, &modified));
            break;
        case 4:
            strcpy(year,   readyxline(config.skin.iy[4], config.skin.ix[4], year, 4, &exitchar, &modified));
            break;
        }

        if ( pos == 4 && exitchar == 13 ) exitchar = 27;
        else

            switch(exitchar) {
            case 'B':
                ;
            case 13:
                pos++;
                break;
            case 'A':
                pos--;
                break;
            }

    } while ( exitchar != 27 );



    if ( modified ) {
        printf("\e[%d;%dH\e[%sm%s", config.skin.iy[6], config.skin.ix[6], config.skin.id3sc, config.skin.id3st);
        fflush(stdout);
        FD_ZERO(&stdinfds);
        FD_SET(fileno(stdin), &stdinfds);
        select(fileno(stdin)+1, &stdinfds, NULL, NULL, NULL);
        if ( toupper(getchar()) != 'N' ) {
            writemp3info(filename, name, artist, misc, album, year, genre);
            if ( playlist != NULL ) {
                if ( artist[0] != 0 )
                    sprintf(playlist->showname, "%s - %s", artist, name);
                else
                    strcpy(playlist->showname, name);
            }
        }
    } /* if modified */
    printf("\e[?25l");
#ifdef USE_GPM_MOUSE
    my_Gpm_Purge();
#endif
    return;
}
