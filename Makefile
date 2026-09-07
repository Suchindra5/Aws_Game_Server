# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -I include
TARGET = game_server

# Source files
SRCS = src/main.cpp src/SpatialGrid.cpp src/SessionManager.cpp
OBJS = main.o SpatialGrid.o SessionManager.o

# Detect Operating System
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    RM = del /Q
    CLEAN_EXTS = main.o SpatialGrid.o SessionManager.o game_server.exe game_server 2>nul || true
else
    LDFLAGS = -pthread
    RM = rm -f
    CLEAN_EXTS = $(OBJS) $(TARGET)
endif

# Default target
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "[Build] Game server compiled successfully: ./$(TARGET)"

# Pattern rule to compile .cpp files from src/ into .o files in the root
%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts (Cross-platform compatible)
clean:
	$(RM) $(CLEAN_EXTS)
	@echo "[Clean] Build artifacts removed."

# Phony targets
.PHONY: all clean