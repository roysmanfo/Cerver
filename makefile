# Makefile for HTTP Server Project (Windows cmd)

# Compiler
CC = gcc

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = obj
BIN_DIR = bin
WEB_DIR = website

# Executable name
EXE_NAME = http_server
TARGET = $(BIN_DIR)/$(EXE_NAME).exe

# Compiler flags
CFLAGS = -Wall -I$(INCLUDE_DIR)

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Default target: build all
all: $(TARGET)

# Alias for `all`
build: all

# Linking object files to create the final executable
$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	@echo [LINKING] creating executable ...
	@$(CC) $(OBJ_FILES) -o $(TARGET) -lws2_32 -mconsole
	@echo [DONE]

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo [COMPILING] $<
	@$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BUILD_DIR):
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(BIN_DIR):
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)

# Run the application with arguments
run: $(TARGET)
	@taskkill /F /IM $(EXE_NAME).exe > nul 2>&1 || cls
	@cd $(WEB_DIR)
	@$(TARGET) $(ARGS)

# Clean build artifacts
clean:
	@if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	@if exist $(BIN_DIR) rmdir /S /Q $(BIN_DIR)

# Rebuild everything from scratch
rebuild: clean all

# Display help information
help:
	@echo Usage:
	@echo.  
	@echo   make                        Build the project
	@echo   make run ARGS="arguments"   Build and run the project with arguments
	@echo   make clean                  Remove build artifacts
	@echo   make rebuild                Clean and rebuild the project
	@echo   make help                   Display this help message

.PHONY: all run clean rebuild help
