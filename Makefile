BASEDIR = $(PWD)
LIB_DIR = $(BASEDIR)/lib
SRC_DIR = $(BASEDIR)/src
TARGET_DIR = $(BASEDIR)/bin
LIBS = -L$(BASEDIR)/lib/x64 -lmsc -ldl -lpthread -lrt -lm -lasound
INCLUDES = -g -w -I$(BASEDIR)/lib/inc
voiceprogram:main.o recordvoice.o cJSON.o VAD.o
	gcc $^ -o $(TARGET_DIR)/voiceprogram $(LIBS) $(INCLUDES)

cJSON.o:$(SRC_DIR)/cJSON.h $(SRC_DIR)/cJSON.c
	gcc -c $^

main.o:$(SRC_DIR)/main.c $(SRC_DIR)/VAD.h
	gcc -c $^ $(INCLUDES) $(LIBS)

recordvoice.o:$(SRC_DIR)/recordvoice.c $(SRC_DIR)/recordvoice.h $(SRC_DIR)/cJSON.h
	gcc -c $^ $(INCLUDES) $(LIBS)

vad.o:$(SRC_DIR)/VAD.c $(SRC_DIR)/VAD.h $(SRC_DIR)/params.h
	gcc -c $^ $(INCLUDES)

clean:
	rm -rf *.o 