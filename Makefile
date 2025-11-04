CXX = g++-13
CXXFLAGS = -std=c++14 -O2 -Wall

code: main.cpp bpt.hpp
	$(CXX) $(CXXFLAGS) main.cpp -o code

clean:
	rm -f code data.db

.PHONY: clean
