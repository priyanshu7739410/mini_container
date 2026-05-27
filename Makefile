# Makefile for Mini Container Runtime in C++

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
TARGET = mycontainer

SRCS = runtime/main.cpp \
       runtime/container.cpp \
       runtime/filesystem.cpp \
       runtime/cgroups.cpp \
       runtime/process.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build successful! Target: $(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Clean completed."
