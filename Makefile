main.exe: main.cc
	c++ `pkg-config --cflags --libs $(SEASTAR)/build/release/seastar.pc` main.cc -o main.exe
