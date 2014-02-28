PROJECT = SteamCrawler

CC = g++
CFLAGS = -c -Wall
LDFLAGS = 

SRC = $(PROJECT).cpp
OBJ = $(SRC:.cpp=.o)

# Gumbo Parser
CFLAGS += $(shell pkg-config --cflags gumbo)
LDFLAGS += $(shell pkg-config --libs gumbo)

# libCurl
LDFLAGS += -lcurl

all : $(SRC) $(PROJECT)

$(PROJECT) : $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.cpp.o :
	$(CC) $(CFLAGS) $< -o $@

clean : 
	rm $(OBJ) $(PROJECT)
