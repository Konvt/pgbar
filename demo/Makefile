CC:= g++
OFLAG = -Og
STANDARD = c++11
CFLAGS:= -std=$(STANDARD) -Wpedantic -Wall
IFLAG:= -I ../include

%: %.cpp
	$(CC) $(OFLAG) $(CFLAGS) $(IFLAG) $< -o $@

all: demo progressbar blockbar spinbar sweepbar multibar unicode
clean:
	find . -maxdepth 1 -type f -executable ! -name '*.*' ! -name 'Makefile' -exec rm {} +
