.PHONY:all
all:udpclient udpserver

udpserver:UDPServer.cc
	g++ -o $@ $^ -std=c++17 -lpthread
udpclient:UDPClient.cc
	g++ -o $@ $^ -std=c++17 -static -lpthread

.PHONY:clean
clean:
	rm -f udpclient udpserver