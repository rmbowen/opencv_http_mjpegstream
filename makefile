CC=g++
CFLAGS=-c -Wall 
LDFLAGS= -lpthread -lopencv_highgui -lopencv_core -lopencv_imgproc
SOURCES=testWebCam.cpp 
OBJECTS=$(SOURCES:%.cpp=%.o)
EXECUTABLE=testWebCam

all: clean $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)

