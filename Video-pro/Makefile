

##makefile for Vedio-pro

CC=gcc
TARGET=video-minitoring
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
LIB+=-libcurl.so -libcurl.a
FLAG=-Wall
INCLUDE=$(PWD)/include
LIBDIR=$(PWD)/library
all:
	$(CC) $(FLAG) $(SRC) -o $(TARGET) -lm -L$(LIBDIR)  -I$(INCLUDE)

clean:
	rm $(OBJ) $(TARGET) -fr
