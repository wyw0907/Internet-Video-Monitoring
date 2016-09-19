

##makefile for Vedio-pro

CC=gcc
TARGET=video-minitoring
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
LIB+=-libcurl.so -libcurl.a
FLAG=-Wall
INCLUDE=$(PWD)
LIBDIR=$(PWD)/library
all:
	$(CC) $(FLAG) $(SRC) -o $(TARGET) -lm -lxml2 -lpthread -lmysqlclient \
         `pkg-config --cflags --libs opencv` -L$(LIBDIR) -I$(INCLUDE)

clean:
	rm $(OBJ) $(TARGET) -fr
