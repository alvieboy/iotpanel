SDL_CFLAGS=$(shell pkg-config --cflags sdl2)
SDL_LIBS=$(shell pkg-config --libs sdl2)

CFLAGS=-I. -ffunction-sections -fdata-sections -Wall -O0 -g -I../esp8266/user -I../esp8266/user/widgets -I./drivers $(SDL_CFLAGS) -DHOST


SOURCES = \
../esp8266/user/font.c                         \
../esp8266/user/font_apple6x10.c               \
../esp8266/user/glcdfont.c                     \
../esp8266/user/server.c                       \
../esp8266/user/schedule.c                     \
../esp8266/user/clock.c                        \
../esp8266/user/font_apple4x6.c                \
../esp8266/user/font_apple5x7.c                \
../esp8266/user/user_main.c                    \
../esp8266/user/gfx.c                          \
../esp8266/user/font_6x10.c                    \
../esp8266/user/font_tom_thumb.c               \
../esp8266/user/font_16x16.c                   \
../esp8266/user/font_12x16.c                   \
../esp8266/user/upgrade.c                      \
../esp8266/user/cdecode.c                      \
../esp8266/user/serdes.c                       \
../esp8266/user/widgets/color.c                \
../esp8266/user/widgets/scrollingtext.c        \
../esp8266/user/widgets/widget.c               \
../esp8266/user/widgets/rectangle.c            \
../esp8266/user/widgets/text.c 	               \
../esp8266/user/widgets/line.c 	               \
../esp8266/user/widgets/clockw.c 	       \
../esp8266/user/widgets/widget_registry.c      \
../esp8266/user/flash_serializer.c     \
../esp8266/user/crc.c     \
iotpanel.c

all: iotpanel iotlogin

iotpanel: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(SDL_LIBS)  -Wl,--gc-sections

clean:
	rm -f iotpanel iotlogin

iotlogin: iotlogin.c
	$(CC) $(CFLAGS) iotlogin.c -o $@

