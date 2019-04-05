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



SEV = raft_server
CLI = raft_client

all:$(SEV) $(CLI)

$(SEV) : $(OBJS) ./exe/Server.cpp
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)
	-mkdir output
	mv $(SEV) output -f

$(CLI) : $(OBJS) ./exe/Client.cpp
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)
	-mkdir output
	mv $(CLI) output -f

%.o : %.c
	$(CC) -c $< $(CCFLAGS)

.PHONY:clean
clean:
	rm -rf *.o output $(SEV) $(CLI)
