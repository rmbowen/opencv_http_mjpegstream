CC=g++
CFLAGS=-c -Wall 
LDFLAGS= -lpthread -lopencv_highgui -lopencv_core -lopencv_imgproc
SOURCES=testWebCamServer.cpp testWebCamClient.cpp
OBJECTS=$(SOURCES:%.cpp=%.o)
EXECS=testWebCamServer testWebCamClient

all: clean $(EXECS)

testWebCamServer: testWebCamServer.o 
	$(CC) $< $(LDFLAGS) -o $@

testWebCamClient: testWebCamClient.o
	$(CC) $< $(LDFLAGS) -o $@
	
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECS)