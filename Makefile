CC = arm-none-eabi-gcc

CPU = -mcpu=cortex-m4 -mthumb

CFLAGS = $(CPU) -Wall -Wextra -O0 -g -ffreestanding \
				-IDrivers/CMSIS/Device/Include \
        		-IDrivers/CMSIS/CMSIS_5/CMSIS/Core/Include \
        		-Iinc \
        		-DSTM32F429xx
CFLAGS += -Iinclude

LDFLAGS = -T linker/linker.ld -nostartfiles --specs=nosys.specs
BUILD_DIR = build
TARGET = $(BUILD_DIR)/sonar.elf

SRC = src/main.c \
	  src/servo.c \
	  startup/startup_stm32F429I.s


OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRC))) \
      $(patsubst %.s,$(BUILD_DIR)/%.o,$(filter %.s,$(SRC)))

all: $(TARGET)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(CPU) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CPU) $(LDFLAGS) $(OBJ) -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean