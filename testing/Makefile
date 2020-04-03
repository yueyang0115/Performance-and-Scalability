TARGETS=client

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

client: client.cpp thread_arg.h
	g++ -g -o $@ $< socket.cpp -lpthread
