
CC = i686-pc-mingw32-gcc
AR = i686-pc-mingw32-ar

CFLAGS = -Wall -g -O2  -DORAL_TEST

INCLUDES = -I. -I../include -I../source -I../win32 -I../../emulator

LDFLAGS = -L/usr/local/lib

LIBS = 

TARGET = test.exe

common-obj = oral_model.o  oral_test.o sqlite3.o natv_io.o swapi_sys_cache.o

vpath %.c ../../emulator ../win32


.PHONY : clean
 
$(TARGET) : $(common-obj)
	$(CC) -o $(TARGET) $(common-obj)

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)


clean:
	rm -f $(common-obj) $(swap-obj) $(TARGET)

