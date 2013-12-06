
CC = i686-pc-mingw32-gcc
AR = i686-pc-mingw32-ar

CFLAGS = -Wall -g -O2 

INCLUDES = \
		   -I. -I../include -I../source -I../swaps

LDFLAGS = -L/usr/local/lib

LIBS = 

TARGET = libswapi.a

common-obj = swapi_message.o swapi_queue.o swapi_handler.o				\
			 swapi_swap.o swapi_loop.o									\
			 swapi_window.o swapi_view.o swapi_canvas.o					\
			 swapi_image.o												\
			 swapi.o

win32-obj = swapi_sys_cache.o swapi_sys_logger.o						\
			natv_time.o natv_io.o natv_cfgsrv.o							\
			natv_hwdrv.o

swap-obj = swap_clock.o 

#swap_user.o swap_oral.o oral_view.o

vpath %.c ../source ../swaps

.PHONY : clean
 
$(TARGET) : $(win32-obj) $(common-obj) $(swap-obj)
	$(AR) rcu -o $(TARGET) $(win32-obj) $(common-obj) $(swap-obj)
	mv $(TARGET) ../../emulator/libs/
	make -C ../../emulator/win32
	mv ../../emulator/win32/emu.exe .

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)


clean:
	make -C ../../emulator/win32 clean
	rm -f $(common-obj) $(win32-obj) $(swap-obj) $(TARGET) *.exe *.log

