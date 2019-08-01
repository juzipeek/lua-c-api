###########################################################
.SUFFIXES: .so .o .c
.c.o:
	$(CC) -fPIC $(CFLAGS) -g -c $< -Wall

.o.so:
	$(CC) -shared $(CFLAGS) $(LIBDIR) $(LIBS) -o $@ $?
###########################################################
CC       = gcc

LUA_DIR = /root/code/lua-5.1.4
LUA_DIR = /usr/local/lua5.1.5
INCL    = -I$(LUA_DIR)/include
DEFS	=
CFLAGS	= $(INCL) $(DEFS)

LIBS	= -llua
LIBDIR  = -L$(LUA_DIR)/lib

all: counter.so stack.so private_cfg.so call_function.so userdata.so

#counter.so: counter.o
#	$(CC) -shared $(CFLAGS) $(LIBDIR) $(LIBS) -o $@ $?

#stack.so: stack.o
#	$(CC) -shared $(CFLAGS) $(LIBDIR) $(LIBS) -o $@ $?

#private_cfg.so: private_cfg.o
#	$(CC) -shared $(CFLAGS) $(LIBDIR) $(LIBS) -o $@ $?

#call_function.so: call_function.o
#	$(CC) -shared $(CFLAGS) $(LIBDIR) $(LIBS) -o $@ $?

.PHONY : clean
clean:
	-rm *.so *.o
