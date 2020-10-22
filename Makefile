GCCFLAGS=-g -Os -Wall -mmcu=atmega168 
LINKFLAGS=-Wl,-u,vfprintf -lprintf_flt -Wl,-u,vfscanf -lscanf_flt -lm
AVRDUDEFLAGS=-c avr109 -p m168 -b 115200 -P COM8

LINKOBJECTS=libnerdkits/delay.o libnerdkits/lcd.o libnerdkits/uart.o common/commonkeypad.o common/sevensegment.o common/timeddelay.o


all:	main-upload

main.hex:	main.c
	make -C libnerdkits 
	make -C common
	avr-gcc ${GCCFLAGS} ${LINKFLAGS} -o main.o main.c ${LINKOBJECTS} 
	avr-objcopy -j .text -O ihex main.o main.hex
		
main-upload:	main.hex
	avrdude ${AVRDUDEFLAGS} -U flash:w:main.hex:a

	
	
.PHONY : clean	
clean:
	rm -f main.o  main.hex 
