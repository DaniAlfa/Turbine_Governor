MAIN_SRC_DIR = ../../src
MAIN_OBJ_DIR = ../../obj
SRC_DIRS = $(MAIN_SRC_DIR) $(MAIN_SRC_DIR)/SlaveDriver $(MAIN_SRC_DIR)/FieldDriver
OBJ_DIRS = $(MAIN_OBJ_DIR) $(MAIN_OBJ_DIR)/SlaveDriver $(MAIN_OBJ_DIR)/FieldDriver

SRC_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
SRC_FILES := $(filter-out $(MAIN_SRC_DIR)/main.cpp,$(SRC_FILES))
OBJ_FILES := $(patsubst $(MAIN_SRC_DIR)/%.cpp,$(MAIN_OBJ_DIR)/%.o,$(SRC_FILES))
OBJ_FILES += ./obj/main.o 
SRC_FILES += main.cpp

CFLAGS  = -Wall -g 
LDFLAGS = 
RUNTIME_PATH = -Wl,-rpath,../../../libs
LIB_PATH = -L../../../libs
LIBS = -lsoem -lmodbus -lxml2 -lz -llzma -lm -lpthread
INCLUDES = -I../../../includes -I../../src/SlaveDriver -I../../src/FieldDriver
PROG   = drvTest
CC = g++
LD = g++

.PHONY : all dirs clean

all : dirs $(PROG)

define PROBJS
$(patsubst $(MAIN_SRC_DIR)%, $(MAIN_OBJ_DIR)%, $(1))/%.o : $(1)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $$@ $$<
endef
$(foreach dir,$(SRC_DIRS),$(eval $(call PROBJS,$(dir))))
	

$(PROG) : $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIB_PATH) $(RUNTIME_PATH) $(LIBS)

./obj/main.o : main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

dirs : $(OBJ_DIRS) ./obj

$(OBJ_DIRS) :
	mkdir -p $@

./obj : 
	mkdir -p $@

clean :
	-rm -f $(PROG) 
	-rm -f -r $(MAIN_OBJ_DIR)
	-rm -f -r ./obj
