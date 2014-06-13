CC=g++
CFLAGS=-c -Wall -std=c++11 -O3 -I ./src
LDFLAGS=
SOURCES=./src/main.cpp ./src/Certifier/CertifyServer.cpp ./src/Certifier/CertifyServerClient.cpp ./src/Core/ConsoleUtilities.cpp ./src/Cryptography/Rijndael.cpp ./src/Memory/Buffer.cpp ./src/Network/NetworkServerClient.cpp ./src/Network/NetworkServer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=certifier

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@