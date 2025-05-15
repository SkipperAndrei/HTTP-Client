CXX=g++
CXXFLAGS= -g -Wall -Werror -Wno-error=unused-variable -std=c++11
CXXSOURCES= client.cpp helpers.cpp requests.cpp buffer.cpp utils.cpp

build:
	$(CXX) $(CXXFLAGS) $(CXXSOURCES) -o client

clean:
	rm -rf client