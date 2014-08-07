# $Id: Makefile,v 1.31.2.1 2012/01/17 22:49:01 jmaggard Exp $
# MiniDLNA project
# http://sourceforge.net/projects/minidlna/
# (c) 2008-2009 Justin Maggard
# for use with GNU Make
# To install use :
# $ DESTDIR=/dummyinstalldir make install
# or :
# $ INSTALLPREFIX=/usr/local make install
# or :
# $ make install
#
CROSS_COMPILE_PREFIX=sh4-linux-
CROSS_SYSROOT_PREFIX=/opt/STM/STLinux-2.4/devkit/sh4/target
# CROSS_BASE_OBJECTS = minidlna.o upnphttp.o upnpdescgen.o upnpsoap.o \
#                      upnpreplyparse.o minixml.o \
#                      getifaddr.o daemonize.o upnpglobalvars.o \
#                      options.o minissdp.o uuid.o upnpevents.o \
#                      utils.o sql.o image_utils.o scanner.o inotify.o\
#                      tivo_utils.o tivo_beacon.o tivo_commands.o \
#                      log.o
# CROSS_LIBS = -lpthread -lsqlite3 -lexif -ljpeg -lavformat

#CFLAGS = -Wall -O -D_GNU_SOURCE -g -DDEBUG
#CFLAGS = -Wall -g -Os -D_GNU_SOURCE

CFLAGS = -Wall -g -O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64	\
	-I.														\
	-I$(CROSS_SYSROOT_PREFIX)/include						\
	-I$(CROSS_SYSROOT_PREFIX)/usr/include					\
	-I$(CROSS_SYSROOT_PREFIX)/usr/local/include

#STATIC_LINKING: CFLAGS += -DSTATIC
#STATIC_LINKING: LDFLAGS = -static
CC = $(CROSS_COMPILE_PREFIX)gcc
RM = rm -f
INSTALL = install
LDFLAGS = -Wl,-rpath /usr/local/lib				\
	-L$(CROSS_SYSROOT_PREFIX)/lib				\
	-L$(CROSS_SYSROOT_PREFIX)/usr/lib			\
	-L$(CROSS_SYSROOT_PREFIX)/usr/local/lib

INSTALLPREFIX ?= $(CROSS_SYSROOT_PREFIX)$(DESTDIR)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
ETCINSTALLDIR = $(CROSS_SYSROOT_PREFIX)$(DESTDIR)/etc

ifeq "$(CROSS_BASE_OBJECTS)" ""
BASEOBJS = minidlna.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o \
           getifaddr.o daemonize.o upnpglobalvars.o \
           options.o minissdp.o uuid.o upnpevents.o \
           sql.o utils.o metadata.o scanner.o inotify.o \
           tivo_utils.o tivo_beacon.o tivo_commands.o \
           tagutils/textutils.o tagutils/misc.o tagutils/tagutils.o \
           playlist.o image_utils.o albumart.o log.o
else
BASEOBJS = $(CROSS_BASE_OBJECTS)
endif

ALLOBJS = $(BASEOBJS) $(LNXOBJS)

ifeq "$(CROSS_LIBS)" ""
LIBS = -lpthread -lexif -ljpeg -lsqlite3 -lavformat -lavutil -lavcodec -lid3tag -lFLAC -logg -lvorbis
#STATIC_LINKING: LIBS = -lvorbis -logg -lm -lsqlite3 -lpthread -lexif -ljpeg -lFLAC -lm -lid3tag -lz -lavformat -lavutil -lavcodec -lm
else
LIBS = $(CROSS_LIBS)
endif

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = minidlna testupnpdescgen

.PHONY:	all clean distclean install depend

all:	$(EXECUTABLES)

clean:
	$(RM) $(ALLOBJS)
	$(RM) $(EXECUTABLES)
	$(RM) testupnpdescgen.o

distclean: clean
	$(RM) config.h

install:	minidlna
	-$(INSTALL) -d $(SBININSTALLDIR)
	$(INSTALL) minidlna $(SBININSTALLDIR)

install-conf:
	-$(INSTALL) -d $(ETCINSTALLDIR)
	$(INSTALL) --mode=0644 minidlna.conf $(ETCINSTALLDIR)

minidlna:	$(BASEOBJS) $(LNXOBJS)
	@echo Linking $@
	$(CC) -Wl,--start-group $(CFLAGS) $(LDFLAGS) -o $@ $(BASEOBJS) $(LNXOBJS) $(LIBS) -Wl,--end-group


testupnpdescgen:	$(TESTUPNPDESCGENOBJS)
	@echo Linking $@
	@$(CC) $(CFLAGS) -o $@ $(TESTUPNPDESCGENOBJS)

config.h:	genconfig.sh
	./genconfig.sh

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) # 2>/dev/null

# DO NOT DELETE

minidlna.o: config.h upnpglobalvars.h minidlnatypes.h sql.h upnphttp.h
minidlna.o: upnpdescgen.h minidlnapath.h getifaddr.h upnpsoap.h options.h
minidlna.o: utils.h minissdp.h daemonize.h upnpevents.h scanner.h inotify.h
minidlna.o: log.h
upnphttp.o: config.h upnphttp.h minidlnatypes.h upnpdescgen.h minidlnapath.h
upnphttp.o: upnpsoap.h upnpevents.h upnpglobalvars.h utils.h getifaddr.h
upnphttp.o: image_utils.h log.h sql.h icons.c
upnpdescgen.o: config.h getifaddr.h upnpdescgen.h minidlnapath.h
upnpdescgen.o: upnpglobalvars.h minidlnatypes.h upnpdescstrings.h
upnpsoap.o: config.h upnpglobalvars.h minidlnatypes.h utils.h upnphttp.h
upnpsoap.o: upnpsoap.h upnpreplyparse.h getifaddr.h scanner.h sql.h log.h
upnpreplyparse.o: upnpreplyparse.h minixml.h
minixml.o: minixml.h
getifaddr.o: upnpglobalvars.h minidlnatypes.h config.h getifaddr.h log.h
daemonize.o: daemonize.h config.h log.h
upnpglobalvars.o: config.h upnpglobalvars.h minidlnatypes.h upnpdescstrings.h
options.o: options.h config.h upnpglobalvars.h minidlnatypes.h
minissdp.o: config.h upnpdescstrings.h minidlnapath.h upnphttp.h
minissdp.o: minidlnatypes.h upnpglobalvars.h upnpreplyparse.h getifaddr.h
minissdp.o: minissdp.h codelength.h utils.h log.h
uuid.o: getifaddr.h log.h
upnpevents.o: config.h upnpevents.h minidlnapath.h upnpglobalvars.h
upnpevents.o: minidlnatypes.h upnpdescgen.h uuid.h log.h
sql.o: sql.h upnpglobalvars.h minidlnatypes.h config.h log.h
utils.o: minidlnatypes.h config.h upnpglobalvars.h log.h
metadata.o: image_utils.h tagutils/tagutils.h upnpglobalvars.h
metadata.o: minidlnatypes.h config.h upnpreplyparse.h metadata.h albumart.h
metadata.o: utils.h sql.h log.h
scanner.o: config.h upnpglobalvars.h minidlnatypes.h metadata.h playlist.h
scanner.o: utils.h sql.h scanner.h albumart.h log.h
inotify.o: config.h upnpglobalvars.h minidlnatypes.h inotify.h utils.h sql.h
inotify.o: scanner.h metadata.h albumart.h playlist.h log.h
tivo_utils.o: config.h
tivo_beacon.o: config.h
tivo_commands.o: config.h
tagutils/textutils.o: tagutils/misc.h tagutils/textutils.h log.h
tagutils/misc.o: tagutils/misc.h
tagutils/tagutils.o: config.h tagutils/tagutils.h tagutils/misc.h
tagutils/tagutils.o: tagutils/textutils.h metadata.h log.h
tagutils/tagutils.o: tagutils/tagutils-mp3.h tagutils/tagutils-aac.h
tagutils/tagutils.o: tagutils/tagutils-ogg.h tagutils/tagutils-flc.h
tagutils/tagutils.o: tagutils/tagutils-asf.h tagutils/tagutils-wav.h
tagutils/tagutils.o: tagutils/tagutils-pcm.h tagutils/tagutils-misc.c
tagutils/tagutils.o: tagutils/tagutils-mp3.c tagutils/tagutils-aac.c
tagutils/tagutils.o: tagutils/tagutils-ogg.c tagutils/tagutils-flc.c
tagutils/tagutils.o: tagutils/tagutils-asf.c tagutils/tagutils-wav.c
tagutils/tagutils.o: tagutils/tagutils-pcm.c tagutils/tagutils-plist.c log.h
playlist.o: tagutils/tagutils.h upnpglobalvars.h minidlnatypes.h config.h
playlist.o: scanner.h metadata.h utils.h sql.h log.h
image_utils.o: upnpreplyparse.h image_utils.h log.h
albumart.o: upnpglobalvars.h minidlnatypes.h config.h albumart.h sql.h
albumart.o: utils.h image_utils.h log.h
log.o: log.h
testupnpdescgen.o: config.h upnpdescgen.h
upnpdescgen.o: config.h getifaddr.h upnpdescgen.h minidlnapath.h
upnpdescgen.o: upnpglobalvars.h minidlnatypes.h upnpdescstrings.h
