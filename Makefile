TARGETS=client server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

client: client.cpp
	g++ -g -o $@ $< function.cpp

server: server.cpp
	g++ -g -o $@ $< function.cpp

