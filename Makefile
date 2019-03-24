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
		   -I./net/linux		\
		   -I./raft

LIBS = -L./lib

CCFLAGS = -lpthread -fPIC -m64 -std=c++11 -lstdc++ -fpermissive -pthread -lzookeeper_mt


OUTPUT = raft_node

all:$(OUTPUT)

$(OUTPUT) : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)

%.o : %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -rf *.out *.o

.PHONY:clean
