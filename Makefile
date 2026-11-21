CC = arm-none-eabi-gcc

CPU = -mcpu=cortex-m4 -mthumb

CFLAGS = $(CPU) -Wall -Wextra -O0 -g -ffreestanding \
				-IDrivers/CMSIS/Device/Include \
        		-IDrivers/CMSIS/CMSIS_5/CMSIS/Core/Include \
        		-Iinc \
        		-DSTM32F429xx
CFLAGS += -Iinclude
CFLAGS += -IDrivers/STM32F4/STM32F4xx_HAL_Driver/Inc
CFLAGS += -IDrivers/BSP/32f429idiscovery-bsp
CFLAGS += -IDrivers/BSP/Components/Common
CFLAGS += -IDrivers/BSP/Components/ili9341
CFLAGS += -IUtilities/Fonts
CFLAGS += -DUSE_HAL_DRIVER

LDFLAGS = -T linker/linker.ld -nostartfiles --specs=nosys.specs
BUILD_DIR = build
TARGET = $(BUILD_DIR)/sonar.elf

SRC = src/main.c \
	  src/servo.c \
	  src/hcsr04.c \
	  startup/startup_stm32F429I.s

SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma2d.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc_ex.c
SRC += Drivers/STM32F4/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c

SRC += Drivers/BSP/32f429idiscovery-bsp/stm32f429i_discovery.c
SRC += Drivers/BSP/32f429idiscovery-bsp/stm32f429i_discovery_sdram.c
SRC += Drivers/BSP/32f429idiscovery-bsp/stm32f429i_discovery_lcd.c

SRC += Drivers/BSP/Components/ili9341/ili9341.c

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