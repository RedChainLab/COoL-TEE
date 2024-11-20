all: out
	g++ -o ../out/server -I../common -pthread server.cpp ../common/json.cpp

multicast: out
	g++ -o ../out/server -I../common -pthread multicast_recv.cpp ../common/json.cpp

out:
	mkdir -p ../out