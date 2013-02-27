SRC = $(wildcard *.cpp)
HDR = $(wildcard *.hpp)

all : opec

opec : $(SRC) $(HDR)
	g++ --std=c++0x -O2 -ggdb3 -Wall -Werror -I ~/tools/eigen-eigen-5097c01bcdc4/ -o$@ $(SRC)
