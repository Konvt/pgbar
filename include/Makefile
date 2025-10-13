CC:= g++ # requires gcc 15.1
OFLAG = O2
STANDARD = c++20
CFLAGS := -fmodules -c -fmodule-only -fsearch-include-path

MODULES = pgbar.Core.ixx pgbar.BlockBar.ixx pgbar.ProgressBar.ixx pgbar.SweepBar.ixx pgbar.SpinBar.ixx pgbar.FlowBar.ixx pgbar.DynamicBar.ixx pgbar.MultiBar.ixx pgbar.ixx

all:
	$(CC) -std=$(STANDARD) -$(OFLAG) $(CFLAGS) $(MODULES)
std:
	$(CC) -std=$(STANDARD) -$(OFLAG) $(CFLAGS) bits/std.cc
