dnl Process this file with autoconf to produce a configure script. 
AC_INIT(camp.h)
#AC_CONFIG_HEADER(config.h)

LIBS=""
CFLAGS="-O2 -Wall"
DEFINES=""
bindir=""
OBJS="misc.o id3.o fork.o player.o cconfig.o filelist.o playlist.o main.o"

dnl Checks for programs.
AC_PROG_CC
#AC_PROG_INSTALL

AC_MSG_CHECKING(whether to compile LIRCD remote control code)
AC_ARG_WITH(lircd,[  --without-lircd         Don't compile code for using LIRC remote controls])

if test "$with_lircd" != no; then
 AC_MSG_RESULT(yes)
 DEFINES="$DEFINES -DLIRCD=\\\"/dev/lircd\\\" -DLIRCCFGFILE=\\\".camp/lircrc\\\""
 OBJS="$OBJS lirc_client.o"
else
 AC_MSG_RESULT(no)
fi




AC_MSG_CHECKING([for x86 assembler instructions])
AC_TRY_COMPILE(,[
        unsigned char val,port;
        __asm__ volatile ("inb %1,%0"
                          : "=a" (val)
                          : "d" ((unsigned short)port));
  ],[
   x86asm="yes"
   AC_MSG_RESULT(yes)
  ],[
   x86asm="no"
   AC_MSG_RESULT(no)
  ])



AC_CHECK_FUNCS(ioperm,[
AC_MSG_CHECKING(whether to compile LPT remote control code)
AC_ARG_WITH(rc,[  --without-rc            Don't compile code for using LPT remote control])

if test "$with_rc" != no; then
 if test "$x86asm" != no; then
    DEFINES="$DEFINES -DRC_ENABLED"
    OBJS="$OBJS rc.o"
    AC_MSG_RESULT(yes)
 else
  AC_MSG_RESULT([no, x86asm missing])
 fi
 else
 AC_MSG_RESULT(no)
fi

],)



dnl Checks for libraries.


AC_MSG_CHECKING(whether to compile with gpm if available)
AC_ARG_WITH(gpm,[  --without-gpm           Don't compile with gpm even if found])


AC_CHECK_HEADERS(gpm.h,[

if test "$with_gpm" != no; then

  AC_CHECK_LIB(gpm,main,[
   LIBS="$LIBS -lgpm"
   DEFINES="$DEFINES -DUSE_GPM_MOUSE" 
  ],[
  oldLIBS=$LIBS
  LIBS="$LIBS -lncurses" 
  AC_CHECK_LIB(gpm,Gpm_Open,[
   LIBS="$LIBS -lgpm"   
   DEFINES="$DEFINES -DUSE_GPM_MOUSE" 
  ],[
   LIBS=$oldLIBS
  ])
  ])
else
 AC_MSG_RESULT(no)
fi

])

AC_CHECK_LIB(z, gzopen)

AC_CHECK_HEADERS(termios.h,,[
 AC_CHECK_LIB(ncurses,main,[
  LIBS="$LIBS -lncurses"
]) ])

AC_CHECK_HEADERS(sys/soundcard.h,[
 OBJS="$OBJS mixer.o"
])

AC_CHECK_HEADERS(sys/ioctl.h,)

AC_CHECK_HEADERS(utmp.h,)

dnl Checks for header files.
#AC_HEADER_STDC

#dnl Checks for typedefs, structures, and compiler characteristics.
#AC_C_CONST
#AC_TYPE_PID_T
#AC_TYPE_SIZE_T

dnl Checks for library functions.
#AC_TYPE_SIGNAL
#AC_CHECK_FUNCS(gethostname)

dnl Checks for mixer..
AC_MSG_CHECKING(for external mixer)

for dir in /usr/bin /usr/local/bin /bin /sbin \
           /usr/sbin /usr/local/sbin; do
if test -x $dir/alsamixer
then mixer="$dir/alsamixer"; break;
fi
done

if test x$mixer != x
 then 
  AC_MSG_RESULT($mixer)
  DEFINES="$DEFINES -DCUSTOM_RUN=\\\"$mixer\\\"" 
 else AC_MSG_RESULT(not found)
fi


dnl Check for bindir

AC_MSG_CHECKING(for old camp binary)

for dir in /usr/bin /usr/local/bin /bin /sbin \
           /usr/sbin /usr/local/sbin; do
if test -x $dir/camp
then bindir="$dir"; break;
fi
done

if test x$bindir != x 
 then AC_MSG_RESULT($bindir) 
 else AC_MSG_RESULT(not found)
 bindir=/usr/local/bin
fi

AC_SUBST(OBJS)
AC_SUBST(DEFINES)
AC_OUTPUT(Makefile)
