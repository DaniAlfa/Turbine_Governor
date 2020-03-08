CFLAGS  = -Wall -g 
SRC = EthercatDrv.cpp EtherDevice.cpp DigitalDev.cpp AnalogDev.cpp  PulseDev.cpp DeviceFactory.cpp main.cpp 
LIBS = -L./libs -lsoem -lxml2 -lz -llzma -lm -lmodbus -lpthread
INCLUDES = -I./includes -I./includes/libsoem
PROG   = drvTest
COMPILER = g++

all :
	$(COMPILER) $(CFLAGS) $(INCLUDES) -o $(PROG) $(SRC) $(LIBS)

clean :
	-rm -f $(PROG) 
