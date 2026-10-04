#!/usr/bin/env bash
set -e

echo "Compiling 3D Geometric Shape-Match..."
g++ -std=c++17 -O2 \
    -Iinclude \
    src/main.cpp \
    src/Mesh.cpp \
    src/Primitives.cpp \
    src/Shader.cpp \
    src/Camera.cpp \
    src/Shape.cpp \
    src/Board.cpp \
    -o candy_pyramid \
    -lglfw -lGLEW -lGL -lGLU

echo "Compilation successful."
echo "Launching candy_pyramid..."
./candy_pyramid
