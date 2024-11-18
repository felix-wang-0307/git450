# Variables
CPP := g++
CPPFLAGS := -std=c++14
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=)

# Default target to compile all executables
all: $(OBJS)
	$(CPP) $(CPPFLAGS) -o client client.cpp
	$(CPP) $(CPPFLAGS) -o serverA serverA.cpp
	$(CPP) $(CPPFLAGS) -o serverD serverD.cpp
	$(CPP) $(CPPFLAGS) -o serverM serverM.cpp
	$(CPP) $(CPPFLAGS) -o serverR serverR.cpp

run:
	./serverA
	./serverD
	./serverM
	./serverR
	./client guest guest

# Rule to compile each .cpp file into an executable
%: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

# Clean rule to remove all executables
clean:
	rm -f $(OBJS)
