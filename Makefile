TARGETS=client server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

client: client.cpp thread_arg.h ThreadPool.h
	g++ -g -o $@ $< function.cpp -lpthread

server: server.cpp thread_arg.h ThreadPool.h
	g++ -g -o $@ $< function.cpp -lpthread

