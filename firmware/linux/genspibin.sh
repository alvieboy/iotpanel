#!/bin/sh


dd if=/dev/zero of=spi.bin bs=512K count=1
dd bs=1 if=../esp8266/firmware/0x00000.bin of=spi.bin seek=0x00000 conv=notrunc
dd bs=1 if=../esp8266/firmware/0x40000.bin of=spi.bin seek=262144 conv=notrunc
dd bs=1 if=../esp8266/firmware/0x10000.bin of=spi.bin seek=65536 conv=notrunc
