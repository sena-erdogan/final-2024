CC = gcc
CFLAGS = -Wall -pthread

all: PideShop HungryVeryMuch

PideShop: PideShop.c
	$(CC) $(CFLAGS) PideShop.c -o PideShop -lpthread

HungryVeryMuch: HungryVeryMuch.c
	$(CC) $(CFLAGS) HungryVeryMuch.c -o HungryVeryMuch -lpthread

clean:
	rm -f PideShop HungryVeryMuch
