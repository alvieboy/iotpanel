QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iotpanel
TEMPLATE = app


SOURCES += main.cpp \
mainwindow.cpp \
../../paneleditor/PanelFramebuffer.cpp \
../../paneleditor/Panel.cpp \
        iotpanel.c \
../esp8266/user/font.c                         \
../esp8266/user/server.c                       \
../esp8266/user/schedule.c                     \
../esp8266/user/clock.c                        \
../esp8266/user/user_main.c                    \
../esp8266/user/gfx.c                          \
../esp8266/user/font_tom_thumb.c               \
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
../esp8266/user/widgets/romimage.c 	       \
../esp8266/user/widgets/widget_registry.c      \
../esp8266/user/flash_serializer.c     \
../esp8266/user/crc.c     \
../esp8266/user/framebuffer.c     \
../esp8266/user/flash.c     \
../esp8266/user/smallfs.c


HEADERS  += ../../paneleditor/Panel.h \
            ../../paneleditor/PanelFramebuffer.h \
            mainwindow.h

QMAKE_CFLAGS=-Wall -Werror -Wno-unused -O2 -g -I../esp8266/user -I../esp8266/user/widgets -I../../paneleditor/ -I./drivers $(SDL_CFLAGS) -DHOST
QMAKE_CXXFLAGS+=-DPANELDISPLAYNOEDIT  -I. -I../../paneleditor/ -I../esp8266/user -I../esp8266/user/widgets -DHOST

#FORMS    += mainwindow.ui
