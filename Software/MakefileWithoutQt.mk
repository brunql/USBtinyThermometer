#
# make -f MakefileWithoutQt.mk
#
all:
	g++ -c -pipe -g -Wall -W -D_REENTRANT -I/usr/share/qt4/mkspecs/linux-g++ -I. -o main.o main.cpp
	g++ -c -pipe -g -Wall -W -D_REENTRANT -I/usr/share/qt4/mkspecs/linux-g++ -I. -o opendevice.o opendevice.cpp
	g++ -o USBtinyThermometer main.o opendevice.o -L/usr/lib -lusb

clean:
	rm -f main.o opendevice.o
