CC=g++
CFLAGS=-c -Wall -O3 -I ./src
LDFLAGS=
SOURCES=./src/main.cpp ./src/Core/ConsoleUtilities.cpp ./src/Network/NetworkServerClient.cpp ./src/Network/NetworkServer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=app

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

#	g++ -I ./src $(SOURCES) -o app
