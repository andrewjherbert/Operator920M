# makefile for Elliott 920M operator console
CC = gcc
NAME = 920M
SRC  = ./src
POPT = `pkg-config popt --cflags --libs`

operator920M: $(SRC)/$(NAME).c
	$(CC) -Wall -Wno-main -o $(NAME) $(SRC)/$(NAME).c  $(POPT)

.PHONY: clean

clean:
	rm -f $(SRC)/*~
	rm -f *~

