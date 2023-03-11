static inline int inb (short port)
{
        unsigned char val;
        __asm__ volatile ("inb %1,%0"
			  : "=a" (val)
			  : "d" ((unsigned short)port));
        return val;
}

unsigned char rcpressed(int port) {
unsigned char in;

   /* 1 = play
    * 2 = skipR
    * 3 = skipL
    * 4 = stop
    * 5 = vol -
    * 6 = vol +
    */

   return inb(port);

}

int main(int argc, char *argv[]) {

   if ( argc != 2 ) {
      printf("Syntax: %s port\n", argv[0]);
      printf("Where port is the decimal address of the LP-port the remote is connected to.\n");
      printf("The address of lp0 (lpt1) is 888.\n");
      return -1;
   }
   ioperm(atoi(argv[1])+1, 1, 1);
   printf("Remote Control test for CAMP 1.3\n");
   printf("Remember! Remote control needs super-user (root) to work!\n");
   printf("Press the buttons on the rc, to gather values, and the insert them\n");
   printf("into ~/.camp/camprc. Press ^C to interrupt\n");
   printf("Value: ");
   while(1)
     printf("\e[s%d  \e[u", rcpressed(atoi(argv[1])+1));

}

