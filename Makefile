CC=g++

CFLAGS=-c -Wall

all: main
main: main.o
	  $(CC) main.o -o main

# or just:
# all: g++ main.cpp -o main
