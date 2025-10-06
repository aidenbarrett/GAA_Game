CC      = g++
CFLAGS  = -std=c++20 -Wall -Wextra -pedantic -O2
RM      = rm -f

default: all

all: GAA

GAA: cli_main.cpp gaa/match.cpp
	$(CC) $(CFLAGS) -I. -o GAA cli_main.cpp gaa/match.cpp

clean veryclean:
	$(RM) GAA
