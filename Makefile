# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -I include
TARGET = game_server

# Source files
SRCS = src/main.cpp src/SpatialGrid.cpp src/SessionManager.cpp
OBJS = main.o SpatialGrid.o SessionManager.o

# Default target
all: $(TARGET)

# Link object files to create the final executable
# When linking the target, append -lws2_32 at the end:
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lws2_32
	@echo "[Build] Game server compiled successfully: ./$(TARGET)"

# Pattern rule to compile .cpp files from src/ into .o files in the root
%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts (Windows compatible)
clean:
	del /Q main.o SpatialGrid.o SessionManager.o game_server.exe game_server 2>nul || true
	@echo "[Clean] Build artifacts removed."

# Phony targets
.PHONY: all clean