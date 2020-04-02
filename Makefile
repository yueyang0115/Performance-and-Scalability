TARGETS=client server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

client: client.cpp thread_arg.h
	g++ -g -o $@ $< socket.cpp -lpthread

server: server.cpp thread_arg.h
	g++ -g -o $@ $< socket.cpp -lpthread

