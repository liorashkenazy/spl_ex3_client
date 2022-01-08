CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -pthread

all: bgs_client
	g++ -o bin/bgs_client bin/connectionHandler.o bin/messageEncoderDecoder.o bin/bgsClient.o $(LDFLAGS)

bgs_client: bin/messageEncoderDecoder.o bin/connectionHandler.o bin/bgsClient.o

bin/messageEncoderDecoder.o: src/messageEncoderDecoder.cpp
	g++ $(CFLAGS) -o bin/messageEncoderDecoder.o src/messageEncoderDecoder.cpp

bin/connectionHandler.o: src/connectionHandler.cpp
	g++ $(CFLAGS) -o bin/connectionHandler.o src/connectionHandler.cpp

bin/bgsClient.o: src/bgsClient.cpp
	g++ $(CFLAGS) -o bin/bgsClient.o src/bgsClient.cpp

.PHONY: clean
clean:
	rm -f bin/*
