.PHONY:all
all:tcpclient tcpserver

tcpclient:TCPClient.cc
	g++ -o $@ $^ -std=c++17
tcpserver:TCPServer.cc
	g++ -o $@ $^ -std=c++17 -lpthread

.PHONY:clean
clean:
	rm -f tcpclient tcpserver