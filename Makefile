BASEDIR = $(PWD)
LIB_DIR = $(BASEDIR)/lib
SRC_DIR = $(BASEDIR)/src
TARGET_DIR = $(BASEDIR)/bin
LIBS = -L$(BASEDIR)/lib/x64 -lmsc -ldl -lpthread -lrt -lm -lasound
INCLUDES = -g -w -I$(BASEDIR)/lib/inc
voiceprogram:main.o recordvoice.o
	gcc $^ -o $(TARGET_DIR)/voiceprogram $(LIBS) $(INCLUDES)

main.o:$(SRC_DIR)/main.c
	gcc -c $^ $(INCLUDES) $(LIBS)

recordvoice.o:$(SRC_DIR)/recordvoice.c $(SRC_DIR)/recordvoice.h
	gcc -c $^ $(INCLUDES) $(LIBS)

clean:
	rm -rf *.o 