CC ?= gcc

FFMPEG_LIBS = libavdevice \
              libavfilter \
              libavformat \
              libavcodec \
              libswresample \
              libswscale \
              libavutil \

PKGCONFIG = $(shell which pkg-config)
CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
CFLAGS += $(shell $(PKGCONFIG) --cflags gtk4)
CFLAGS += $(shell $(PKGCONFIG) --cflags sdl2)
LIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LIBS)
LIBS += $(shell $(PKGCONFIG) --libs gtk4)
LIBS += $(shell $(PKGCONFIG) --libs sdl2)

GLIB_COMPILE_RESOURCES = glib-compile-resources

SRC = main.c playback.c playlistitem.c utils.c
GEN_SRC = resources.c

OBJDIR := objdir
OBJS := $(addprefix $(OBJDIR)/,$(SRC:.c=.o) $(GEN_SRC:.c=.o))

.PHONY=all clean

all: lamp

resources.c: resources.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies resources.xml)
	$(GLIB_COMPILE_RESOURCES) resources.xml --target=$@ --generate-source

$(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

lamp: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(GEN_SRC)
	rm -f $(OBJS)
	rm -rf $(OBJDIR)
	rm -f lamp

install:
	cp res/img/lamp.png /usr/share/icons/hicolor/512x512/apps/lamp.png
	cp res/img/lamp.png /usr/share/icons/hicolor/48x48/apps/lamp.png
	cp res/lamp.desktop /usr/share/applications/lamp.desktop
	cp lamp /usr/bin/lamp

uninstall:
	rm /usr/share/icons/hicolor/512x512/apps/lamp.png
	rm /usr/share/icons/hicolor/48x48/apps/lamp.png
	rm /usr/share/applications/lamp.desktop
	rm /usr/bin/lamp
