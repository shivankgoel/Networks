# this target will compile all the files
all: make1 make2

make1: sender.c
	gcc -pthread -o sender sender.c
make2: receiver.c
	gcc -o receiver receiver.c
clean:
	rm -rf *o 
