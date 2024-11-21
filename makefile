# Variables
CPP := g++
CPPFLAGS := -std=c++14
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=)

# Default target to compile all executables
all: $(OBJS)

run_client:
	./client guest guest

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
