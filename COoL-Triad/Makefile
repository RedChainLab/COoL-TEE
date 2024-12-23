APP_NAME=triad
SO_PATH=node/libtriad.so
TIME_AUTH_DIR=time_authority
TIME_AUTH_PATH=$(TIME_AUTH_DIR)/server
CLIENT_DIR=client
CLIENT_PATH=$(CLIENT_DIR)/client

all: build

build: $(SO_PATH) $(APP_NAME) $(TIME_AUTH_PATH) $(CLIENT_PATH)

run: build
	./$(APP_NAME)

$(SO_PATH):
	cd node && make

$(APP_NAME):
	g++ -o $(APP_NAME) -Wl,-rpath,node -Inode -I/opt/intel/sgxsdk/include user/main.cpp -Lnode -ltriad

$(TIME_AUTH_PATH):
	cd $(TIME_AUTH_DIR) && make build

$(CLIENT_PATH):
	cd $(CLIENT_DIR) && make

clean:
	rm -f $(APP_NAME)

cleanall: clean
	cd node && make clean
	cd $(TIME_AUTH_DIR) && make clean
	rm -f $(SO_PATH)

.PHONY: all build run clean