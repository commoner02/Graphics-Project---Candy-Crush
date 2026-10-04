CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lglfw -lGLEW -lGL -lGLU

SRCS = src/main.cpp src/Mesh.cpp src/Primitives.cpp src/Shader.cpp src/Camera.cpp src/Shape.cpp src/Board.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = candy_pyramid

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
