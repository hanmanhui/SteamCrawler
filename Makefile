PROJECT = SteamCrawler

CC = g++
CFLAGS = -c -Wall
LDFLAGS = 

SRC = $(PROJECT).cpp CurlConnector.cpp MySQLConnector.cpp
SRC += SteamUserCrawler.cpp SteamFriendsCrawler.cpp SteamGameCrawler.cpp SteamUserGameCrawler.cpp
OBJ = $(SRC:.cpp=.o)

# Gumbo Parser
CFLAGS += $(shell pkg-config --cflags gumbo)
LDFLAGS += $(shell pkg-config --libs gumbo)

# libCurl
CFLAGS += $(shell pkg-config --cflags libcurl)
LDFLAGS += $(shell pkg-config --libs libcurl)

# Mysql Cpp Connector
CFLAGS += -I/usr/local/include
LDFLAGS += -I/usr/local/include/cppconn -lmysqlcppconn

# Boost
LDFLAGS += -lboost_system -lboost_thread

all : $(SRC) $(PROJECT)

$(PROJECT) : $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.cpp.o :
	$(CC) $(CFLAGS) $< -o $@

clean : 
	rm $(OBJ) $(PROJECT)
