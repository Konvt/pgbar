CC:= g++
OFLAG = -Og
STANDARD = c++23
CFLAGS:= -std=$(STANDARD) -Wpedantic -Wall -Wextra -Wno-implicit-fallthrough
IFLAG:= -I../include

%: %.cpp
	$(CC) $(OFLAG) $(CFLAGS) $(IFLAG) $< -o $@

all: demo progressbar blockbar spinbar sweepbar flowbar multibar dynamicbar unicode
clean:
	find . -maxdepth 1 -type f -executable ! -name '*.*' ! -name 'Makefile' -exec rm {} +
