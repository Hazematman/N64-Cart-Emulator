SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
BIN_NAME := n64_cart.elf
RAW := n64_cart.bin

SRC_C := $(SRC_DIR)/main.c $(SRC_DIR)/printf.c $(SRC_DIR)/gpio.c $(SRC_DIR)/d1_regs.c \
	 $(SRC_DIR)/uart.c
SRC_S := $(SRC_DIR)/start.S

EXE := $(BIN_DIR)/$(BIN_NAME)
SRC := $(wildcard $(SRC_DIR)/*.c)
#OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_C)) $(patsubst $(SRC_DIR)/%.S,$(OBJ_DIR)/%.o,$(SRC_S))

CC := riscv64-unknown-elf-gcc
OBJCOPY := riscv64-unknown-elf-objcopy
CPPFLAGS := -march=rv64g --std=gnu99 -msmall-data-limit=0 -MMD -MP
CFLAGS   := -Wall
LDFLAGS  := -march=rv64g -ffreestanding -nostdlib -msmall-data-limit=0 -T linker.ld
LDLIBS   :=  

COMPILE.o = $(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(RAW)

$(RAW): $(EXE)
	$(OBJCOPY) -O binary $< $@

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(COMPILE.o)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S | $(OBJ_DIR)
	$(COMPILE.o)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(RAW)

-include $(OBJ:.o=.d)
