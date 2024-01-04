all:
	g++ -Og -std=c++11 -I ./include sample.cpp -o sample
clean:
	rm -f sample
