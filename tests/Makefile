CC := g++
FLAGS := -std=c++11 -Wall -Og -Wpedantic -Wall 
HEADER := -I ../include
EXTRA_FLAGS := -fsanitize=address -fsanitize=undefined

TARGET := test
# use `make file=example.cpp && ./test` to run a unit test
SRC_FILE = $(if $(filter %.cpp,$(file)),$(file),$(file).cpp)

$(TARGET): $(SRC_FILE)
	$(CC) $(FLAGS) $(EXTRA_FLAGS) $(HEADER) $(SRC_FILE) -o $(TARGET)

clean:
	rm -f $(TARGET)
