TARGETS=server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

server: server.cpp thread_arg.h
	g++ -g -o $@ $< socket.cpp -lpthread

