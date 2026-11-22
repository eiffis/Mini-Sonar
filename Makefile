CC = arm-none-eabi-gcc

CPU = -mcpu=cortex-m4 -mthumb

CUBE = cubeMX
HAL_SRC = $(CUBE)/Drivers/STM32F4xx_HAL_Driver/Src

CFLAGS = $(CPU) -Wall -Wextra -O0 -g -ffreestanding \
				-I$(CUBE)/Core/Inc \
				-I$(CUBE)/Drivers/STM32F4xx_HAL_Driver/Inc \
				-I$(CUBE)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
				-I$(CUBE)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
				-I$(CUBE)/Drivers/CMSIS/Include \
        		-DSTM32F429xx \
        		-DUSE_HAL_DRIVER
CFLAGS += -Iinclude
CFLAGS += -IUtilities/Fonts

LDFLAGS = -T linker/linker.ld -nostartfiles --specs=nosys.specs
BUILD_DIR = build
TARGET = $(BUILD_DIR)/sonar.elf

SRC = src/main.c \
	  src/servo.c \
	  src/hcsr04.c \
	  src/display.c \
	  src/radar.c \
	  src/system_clock.c \
	  Utilities/Fonts/font12.c \
	  Utilities/Fonts/font24.c \
	  startup/startup_stm32F429I.s

SRC += $(CUBE)/Core/Src/gpio.c \
	   $(CUBE)/Core/Src/fmc.c \
	   $(CUBE)/Core/Src/ltdc.c \
	   $(CUBE)/Core/Src/spi.c \
	   $(CUBE)/Core/Src/stm32f4xx_hal_msp.c \
	   $(CUBE)/Core/Src/stm32f4xx_it.c \
	   $(CUBE)/Core/Src/system_stm32f4xx.c

SRC += $(HAL_SRC)/stm32f4xx_hal.c \
	   $(HAL_SRC)/stm32f4xx_hal_cortex.c \
	   $(HAL_SRC)/stm32f4xx_hal_dma.c \
	   $(HAL_SRC)/stm32f4xx_hal_dma_ex.c \
	   $(HAL_SRC)/stm32f4xx_hal_exti.c \
	   $(HAL_SRC)/stm32f4xx_hal_flash.c \
	   $(HAL_SRC)/stm32f4xx_hal_flash_ex.c \
	   $(HAL_SRC)/stm32f4xx_hal_gpio.c \
	   $(HAL_SRC)/stm32f4xx_hal_ltdc.c \
	   $(HAL_SRC)/stm32f4xx_hal_ltdc_ex.c \
	   $(HAL_SRC)/stm32f4xx_hal_pwr.c \
	   $(HAL_SRC)/stm32f4xx_hal_pwr_ex.c \
	   $(HAL_SRC)/stm32f4xx_hal_rcc.c \
	   $(HAL_SRC)/stm32f4xx_hal_rcc_ex.c \
	   $(HAL_SRC)/stm32f4xx_hal_sdram.c \
	   $(HAL_SRC)/stm32f4xx_hal_spi.c \
	   $(HAL_SRC)/stm32f4xx_ll_fmc.c

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
	$(CC) $(CPU) $(LDFLAGS) $(OBJ) -lm -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean