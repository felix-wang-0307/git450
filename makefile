# Variables
CPP := g++
CPPFLAGS := -std=c++17
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=)

# Default target to compile all executables
all: $(OBJS)

run_client:
	./client test test

run_server_M:
	./serverM

# Rule to compile each .cpp file into an executable
%: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

# Clean rule to remove all executables
clean:
	rm -f $(OBJS)
