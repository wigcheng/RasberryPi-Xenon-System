CC=gcc
CFLAGS=-g -O3 -Wall -Werror
PHONY := clean

xenon-app: main.o serial.o
	${CC} main.o serial.o -o $@

main.o: main.c
	${CC} $< ${FLAGS} -c

serial.o: serial.c
	${CC} $< ${FLAGS} -c

clean:
	@rm xenon-app
	@rm -rf *.o

PHONY: $(PHONY)
