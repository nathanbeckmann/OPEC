SRC = $(wildcard *.cpp)
HDR = $(wildcard *.hpp)

all : opec

.PHONY : opec
opec : $(SRC) $(HDR)
	g++ --std=c++0x -O0 -ggdb3 -Wall -Werror -I ~/tools/eigen-eigen-5097c01bcdc4/ -o$@ $(SRC)
