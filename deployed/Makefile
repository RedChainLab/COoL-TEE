default: all

.PHONY: deps

deps:
	sudo apt install -y libboost-all-dev
	sudo apt install -y nasm
	sudo apt install -y tcl
	sudo apt install -y curl
	sudo apt-get install -y libreadline-dev 

	cd deps && tar zxf redis-6.2.6.tar.gz && tar zxf redis-plus-plus-1.2.3.tar.gz && sudo chmod -R 777 * && awk 'NR==36{print "#include <cstdint>"}1' redis-plus-plus-1.2.3/src/sw/redis++/utils.h
	cd deps/redis-6.2.6/deps/hiredis/ && make && sudo make install
	cd deps/redis-plus-plus-1.2.3 && mkdir -p build && cd build && cmake .. && make && sudo make install
	cd kanban/deps/jemalloc ; ./autogen.sh ; cd .. ; sudo make hdr_histogram linenoise jemalloc 
	sudo ldconfig /usr/local/lib

cool:
	cd client ; make all ; cd -
	cd server ; make all ; cd -

desearch:
	cd ds-client ; make all ; cd -
	cd manager ; make all ; cd -
	cd executor ; make all ; cd -
	cd sgx-executor ; make all ; cd -

all: deps desearch cool
	cd kanban ; make 
	cd kanban ; ./src/redis-server ./redis.conf &
	sleep 1
	cd kanban ; ./src/redis-cli FLUSHALL
	pkill redis-server

clean:
	cd ds-client ; make clean ; cd -
	cd manager ; make clean ; cd -
	cd executor ; make clean ; cd -
	cd sgx-executor ; make clean ; cd -
	cd client ; make clean ; cd -
	cd server ; make clean ; cd -

cleanall: clean
	cd kanban ; make clean ; cd -
