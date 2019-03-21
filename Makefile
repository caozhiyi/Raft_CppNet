SRCS = $(wildcard ./base/*.cpp			\
				  ./zkgenerated/*.c		\
				  ./zk/*.cpp			\
				  ./raft/*.cpp			\
				  ./net/*.cpp			\
				  ./net/linux/*.cpp)

OBJS = $(SRCS:.cpp = .o)

CC = g++

INCLUDES = -I.					\
           -I./base				\
		   -I./zk				\
           -I./zk/include		\
           -I./zk/generated		\
		   -I./net				\
		   -I./raft

LIBS = -L./lib

CCFLAGS = -lpthread -fPIC -m64 -std=c++11 -lstdc++ -fpermissive -lzookeeper_mt -pthread

OUTPUT = test.out

all:$(OUTPUT)

$(OUTPUT) : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)

%.o : %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -rf *.out *.o

.PHONY:clean
