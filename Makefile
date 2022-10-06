CPP=/usr/bin/cpp
CXX=/usr/bin/g++
AS=/usr/bin/as
LD=/usr/bin/ld

all: main notifier
	$(CXX) -o notifier main.o notifier.o -lpthread

#detailed compilation process
#1 preprocessor generates translation unit using source code
#2 compiler generates assembly code using translation unit 
#3 assembler generates binary file using assembly code 
main: main.cpp
	$(CPP) main.cpp --std=c++17 > main.i 
	$(CXX) -S main.i --std=c++17
	$(AS) -o main.o main.s

notifier: notifier.cpp
	$(CXX) -c notifier.cpp --std=c++17 

clean:
	rm -rf main.o notifier.o main.i main.s
