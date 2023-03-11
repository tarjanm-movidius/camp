CC=gcc
CFLAGS=-O2
BINDIR=/usr/bin
LIBS=-lz  -lgpm
DEFS= -DLIRCD=\"/dev/lircd\" -DLIRCCFGFILE=\".camp/lircrc\" -DRC_ENABLED -DUSE_GPM_MOUSE -DCUSTOM_RUN=\"/usr/bin/aumix\" -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DHAVE_IOPERM=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_GPM_H=1 -DHAVE_LIBZ=1 -DHAVE_TERMIOS_H=1 -DHAVE_SYS_SOUNDCARD_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_UTMP_H=1
OBJS=misc.o id3.o fork.o player.o cconfig.o filelist.o playlist.o main.o lirc_client.o rc.o mixer.o

all: main

build:
	builder

main:	$(OBJS) 
	$(CC) $(CFLAGS) $(LIBS) -o camp $(OBJS)
	@echo ""
	@echo "If you are planning to use a LPT remote control, run \"make rctools\"."
	@echo ""
	@echo "Type \"make install\" to install the binary and default configfile." 
	@echo "Additionally, type \"make install-skins\" to install the current skin-pack." 
	@echo ""
rctools:
	$(CC) $(CFLAGS) -o testrc testrc.c

install:
	if test -f $(HOME)/.camp; then rm -f $(HOME)/.camp; fi
	if test -f $(HOME)/.camp/camprc; then mv -f $(HOME)/.camp/camprc $(HOME)/.camp/camprc.bak-pre1.5; fi
	./mkinstalldirs $(BINDIR) $(HOME)/.camp
	install -s -m 0755 ./camp     $(BINDIR)
	install    -m 0600 ./camp.ini $(HOME)/.camp/camprc
	install    -m 0600 ./tip.txt  $(HOME)/.camp/tip.txt
	@echo ""
	@echo "Type \"make install-skins\" to install the current skin-pack." 


install-skins:
	./mkinstalldirs $(HOME)/.camp/skins
	cp skins.tar $(HOME)/.camp
	( cd $(HOME)/.camp && tar xvf skins.tar )
	@echo "Done!"
	
.c.o:	
	$(CC) $(DEFS) $(CFLAGS) -c $<

clean: 
	rm -f camp *.o *~

distclean: clean
	rm -f config.* Makefile
