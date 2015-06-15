BASEDIR = $(PWD)
LIB_DIR = $(BASEDIR)/lib
SRC_DIR = $(BASEDIR)/src
TARGET_DIR = $(BASEDIR)/bin
LIBS = -L$(BASEDIR)/lib/x64 -lmsc -ldl -lpthread -lrt -lm -lasound
INCLUDES = -g -w -I$(BASEDIR)/lib/inc


voiceprogram:main.o recordvoice.o VAD.o cJSON.o
	gcc $^ -o $(TARGET_DIR)/$@ $(LIBS) $(INCLUDES)

main.o:$(SRC_DIR)/main.c $(SRC_DIR)/VAD.h
	gcc -c $^ $(INCLUDES) $(LIBS)

recordvoice.o:$(SRC_DIR)/recordvoice.c $(SRC_DIR)/recordvoice.h $(SRC_DIR)/cJSON.c
	gcc -c $^ $(INCLUDES) $(LIBS)

cJSON.o:$(SRC_DIR)/cJSON.h $(SRC_DIR)/cJSON.c
	gcc -o $@ -c $^

VAD.o:$(SRC_DIR)/VAD.c $(SRC_DIR)/VAD.h $(SRC_DIR)/params.h
	gcc -c $^ $(INCLUDES)

clean:
	rm -rf *.o 