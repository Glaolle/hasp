TARGET = usbhaspinfo
PREFIX = /usr/sbin

.PHONY: all clean install uninstall

all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o

$(TARGET):
	gcc -m32 -c *.c && gcc -m32 *.o libhasplnx.a -o $(TARGET)

install:
	install $(TARGET) $(PREFIX)

uninstall:
	rm -rf $(PREFIX)/$(TARGET)
