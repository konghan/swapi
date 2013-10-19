
CC = i686-pc-mingw32-gcc
AR = i686-pc-mingw32-ar

CFLAGS = -Wall -g -O2 -I. -I../include -I../source

LDFLAGS = -L/usr/local/lib

LIBS = 

TARGET = libswapi.a

common-obj = swapi_message.o swapi_queue.o swapi_handler.o \
			 swapi_swap.o swapi_loop.o swapi.o

win32-obj = swapi_sys_cache.o swapi_sys_logger.o

swap-obj = swap_clock.o

vpath %.c ../source

.PHONY : clean
 
$(TARGET) : $(win32-obj) $(common-obj) $(swap-obj)
	$(AR) rcu -o $(TARGET) $(win32-obj) $(common-obj) $(swap-obj)

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)


clean:
	rm -f $(common-obj) $(win32-obj) $(swap-obj) $(TARGET)
