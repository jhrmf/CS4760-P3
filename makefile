TARGET = oss shmMsg
all: main shmMsg
main: main.c sharedMem.h
	gcc -o oss main.c
shmMsg: shmMsg.c sharedMem.h
	gcc -o shmMsg shmMsg.c -lpthread -lrt
clean:
	/bin/rm -f *.o $(TARGET)


