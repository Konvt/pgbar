CC:= g++
OFLAG = -Og
STANDARD = c++11
CFLAGS:= -std=$(STANDARD) -Wpedantic -Wall
IFLAG:= -I ../include

%: %.cpp
	$(CC) $(CFLAGS) $(IFLAG) $< -o $@

all: demo progressbar blockprogressbar spinnerbar scannerbar unicode
clean:
	find . -maxdepth 1 -type f -executable ! -name '*.*' ! -name 'Makefile' -exec rm {} +
