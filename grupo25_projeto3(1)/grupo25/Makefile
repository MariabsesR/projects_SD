#Grupo 25:
#António Estêvão, Nº 58203
#Jacky Xu, Nº 58218
#Maria Rocha, Nº 58208

BIN_DIR = binary
INC_DIR = include
SRC_DIR = source
OBJ_DIR = object
LIB_DIR = lib
LIBS = -ltable -lprotobuf-c

CC = gcc
CFLAGS = -Wall -g -MMD -MP -MF $(OBJ_DIR)/$*.d -I$(INC_DIR)
LDFLAGS = -L./lib

	

# List the object files for the library components (excluding sdmessage.pb-c.c)
LIB_OBJECTS = $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o 
LIB_OBJECTS += $(OBJ_DIR)/sdmessage.pb-c.o

# Source files for table_client
CLIENT_SOURCES = $(SRC_DIR)/table_client.c $(SRC_DIR)/client_stub.c $(SRC_DIR)/network_client.c $(SRC_DIR)/message.c
CLIENT_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SOURCES))
CLIENT_TARGET = $(BIN_DIR)/table_client

# Source files for table_server
SERVER_SOURCES = $(SRC_DIR)/table_server.c $(SRC_DIR)/network_server.c $(SRC_DIR)/table_skel.c $(SRC_DIR)/message.c
SERVER_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SERVER_SOURCES))
SERVER_TARGET = $(BIN_DIR)/table_server

LIB_TARGET = $(LIB_DIR)/libtable.a

.PHONY: all libtable table-client table-server clean

all:  libtable proto  table-client table-server

libtable: $(LIB_TARGET)

proto: $(SRC_DIR)/sdmessage.pb-c.c

$(OBJ_DIR)/sdmessage.pb-c.o: sdmessage.proto
$(SRC_DIR)/sdmessage.pb-c.c: sdmessage.proto | $(SRC_DIR)
	protoc-c --c_out=$(SRC_DIR) sdmessage.proto
	$(CC) -c $(SRC_DIR)/sdmessage.pb-c.c -o $(OBJ_DIR)/sdmessage.pb-c.o
	mv $(SRC_DIR)/sdmessage.pb-c.h $(INC_DIR)/

table-client: CFLAGS += -DCLIENT
table-client: $(CLIENT_TARGET)

table-server: CFLAGS += -DSERVER
table-server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS) $(LIB_TARGET)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
	
$(CLIENT_TARGET): $(CLIENT_OBJECTS) $(LIB_TARGET)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

$(LIB_TARGET): $(LIB_OBJECTS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR) $(LIB_DIR) $(BIN_DIR):
	mkdir -p $@


clean:
	rm -f $(LIB_DIR)/*
	rm -f $(BIN_DIR)/table_client $(BIN_DIR)/table_server
	rm -f $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/table_client.o $(OBJ_DIR)/table_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/message.o $(OBJ_DIR)/stats.o 
	rm -f $(SRC_DIR)/sdmessage.pb-c.c $(INC_DIR)/sdmessage.pb-c.h $(OBJ_DIR)/sdmessage.pb-c.o 
	rm -f $(OBJ_DIR)/*.d