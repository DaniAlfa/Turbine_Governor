CFLAGS  = -Wall -g 
SRC = EthercatDrv.cpp EtherDevice.cpp DigitalDev.cpp main.cpp 
LIBS = -I/home/ddan/Develop/SOEMTest/includes -L/home/ddan/Develop/SOEMTest/SOEM-master/build -lsoem -lpthread
PROG   = drvTest
COMPILER = g++

all :
	$(COMPILER) $(CFLAGS) -o $(PROG) $(SRC) $(LIBS)

clean :
	-rm -f $(PROG) 
