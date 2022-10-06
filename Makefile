CPP=/usr/bin/cpp
CXX=/usr/bin/g++
AS=/usr/bin/as
LD=/usr/bin/ld

all: create_dirs main notifier
	$(CXX) -o ./bin/notifier build/main.o build/notifier.o -lpthread

create_dirs:
	mkdir -p ./build ./bin

#detailed compilation process
#1 preprocessor generates translation unit using source code
#2 compiler generates assembly code using translation unit 
#3 assembler generates binary file using assembly code 
main: main.cpp
	$(CPP) main.cpp --std=c++17 > ./build/main.i 
	$(CXX) -S ./build/main.i -o ./build/main.s --std=c++17
	$(AS) ./build/main.s -o ./build/main.o

notifier: notifier.cpp
	$(CXX) -c notifier.cpp -o ./build/notifier.o --std=c++17 

clean:
	rm -rf ./build ./bin
