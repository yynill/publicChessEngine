CC = gcc
SRC_DIR = src
BUILD_DIR = build

# Use the correct paths for SDL2, SDL2_image, and SDL2_ttf
SDL2_PATH := $(shell brew --prefix sdl2)
SDL2_IMAGE_PATH := $(shell brew --prefix sdl2_image)
SDL2_TTF_PATH := $(shell brew --prefix sdl2_ttf)

# Compiler flags, including SDL2, SDL2_image, and SDL2_ttf headers
CFLAGS = -I$(SDL2_PATH)/include/SDL2 \
         -I$(SDL2_IMAGE_PATH)/include/SDL2 \
         -I$(SDL2_TTF_PATH)/include/SDL2 \
         -Isrc/rendering

# Linker flags for SDL2, SDL2_image, and SDL2_ttf libraries
LDFLAGS = -L$(SDL2_PATH)/lib \
          -L$(SDL2_IMAGE_PATH)/lib \
          -L$(SDL2_TTF_PATH)/lib \
          -lSDL2 -lSDL2_image -lSDL2_ttf

# Find all .c files in the SRC_DIR and its subdirectories
SRC = $(shell find $(SRC_DIR) -name '*.c')
# Convert the SRC paths to object file paths in the BUILD_DIR
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

EXEC = main

all: $(BUILD_DIR)/$(EXEC)

$(BUILD_DIR)/$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/$(EXEC)

clear:
	rm -rf $(BUILD_DIR)

.PHONY: all clean clear
