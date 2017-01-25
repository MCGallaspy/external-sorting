main.exe: main.cc
	g++-5 -Wall -Werror -std=c++14 $(EXTRA_FLAGS) `pkg-config --cflags --libs $(SEASTAR)/build/release/seastar.pc` main.cc -o main.exe
