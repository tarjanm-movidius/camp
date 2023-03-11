#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include "camp.h"

void set_volume(int dev, int change) {
int fd, volume, left, right;
   
   fd = open("/dev/mixer", O_RDWR);
   ioctl(fd, MIXER_READ(dev), &volume);
   left  = volume & 0x7f;
   right = (volume >> 8) & 0x7f;
   if ( left + change >99 )  left = 99; else
     if ( left + change <0 ) left = 0;  else     
     left  += change;
   if ( right + change >99)  right = 99; else
     if ( right + change <0) right = 0;  else
     right += change;
   volume = left + (right << 8);
   ioctl(fd, MIXER_WRITE(dev), &volume);
   close(fd);
}

void get_volume(int dev, int *left, int *right) {
int fd, volume;
   
   fd = open("/dev/mixer", O_RDWR);
   ioctl(fd, MIXER_READ(dev), &volume);
   close(fd);   
   *left  = volume & 0x7f;
   *right = (volume >> 8) & 0x7f;
}

