TARGET = oss user
all: main user
main: main.c sharedMem.h
	gcc -o oss main.c
user: shmMsg.c sharedMem.h
	gcc -o user shmMsg.c -lpthread -lrt
clean:
	/bin/rm -f *.o $(TARGET)
