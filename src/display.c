#include "display.h"
#include "fmc.h"
#include "gpio.h"
#include "ltdc.h"
#include "main.h"
#include "spi.h"
#include <string.h>

#define SDRAM_TIMEOUT 0xFFFFU
#define SDRAM_MODE_CAS3_SINGLE_WRITE 0x0230U
#define SDRAM_REFRESH_COUNT 1386U

#define ILI_DELAY 0x80U
#define ILI_END 0x00U

static const uint8_t ili9341_init_sequence[] = {
    0xCA, 3,         0xC3, 0x08, 0x50, 0xCF, 3,    0x00,      0xC1, 0x30, 0xED,
    4,    0x64,      0x03, 0x12, 0x81, 0xE8, 3,    0x85,      0x00, 0x78, 0xCB,
    5,    0x39,      0x2C, 0x00, 0x34, 0x02, 0xF7, 1,         0x20, 0xEA, 2,
    0x00, 0x00,      0xB1, 2,    0x00, 0x1B, 0xB6, 2,         0x0A, 0xA2, 0xC0,
    1,    0x10,      0xC1, 1,    0x10, 0xC5, 2,    0x45,      0x15, 0xC7, 1,
    0x90, 0x36,      1,    0xC8, 0xF2, 1,    0x00, 0xB0,      1,    0xC2, 0xB6,
    4,    0x0A,      0xA7, 0x27, 0x04, 0x2A, 4,    0x00,      0x00, 0x00, 0xEF,
    0x2B, 4,         0x00, 0x00, 0x01, 0x3F, 0xF6, 3,         0x01, 0x00, 0x06,
    0x2C, ILI_DELAY, 0x26, 1,    0x01, 0xE0, 15,   0x0F,      0x29, 0x24, 0x0C,
    0x0E, 0x09,      0x4E, 0x78, 0x3C, 0x09, 0x13, 0x05,      0x17, 0x11, 0x00,
    0xE1, 15,        0x00, 0x16, 0x1B, 0x04, 0x11, 0x07,      0x31, 0x33, 0x42,
    0x05, 0x0C,      0x0A, 0x28, 0x2F, 0x0F, 0x11, ILI_DELAY, 0x29, 0,    0x2C,
    0,    ILI_END,
};

static void sdram_command(uint32_t mode, uint32_t refresh, uint32_t reg) {
  FMC_SDRAM_CommandTypeDef command = {0};

  command.CommandMode = mode;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = refresh;
  command.ModeRegisterDefinition = reg;

  if (HAL_SDRAM_SendCommand(&hsdram1, &command, SDRAM_TIMEOUT) != HAL_OK) {
    Error_Handler();
  }
}

static void sdram_start(void) {
  sdram_command(FMC_SDRAM_CMD_CLK_ENABLE, 1U, 0U);
  HAL_Delay(1U);
  sdram_command(FMC_SDRAM_CMD_PALL, 1U, 0U);
  sdram_command(FMC_SDRAM_CMD_AUTOREFRESH_MODE, 4U, 0U);
  sdram_command(FMC_SDRAM_CMD_LOAD_MODE, 1U, SDRAM_MODE_CAS3_SINGLE_WRITE);

  if (HAL_SDRAM_ProgramRefreshRate(&hsdram1, SDRAM_REFRESH_COUNT) != HAL_OK) {
    Error_Handler();
  }
}

static void ili9341_write(GPIO_PinState dcx, uint8_t value) {
  HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, dcx);
  HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);
  if (HAL_SPI_Transmit(&hspi5, &value, 1U, HAL_MAX_DELAY) != HAL_OK) {
    Error_Handler();
  }
  HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);
}

static void ili9341_start(void) {
  const uint8_t *entry = ili9341_init_sequence;

  while (*entry != ILI_END) {
    uint8_t command = *entry++;
    uint8_t info = *entry++;
    uint8_t length = info & (uint8_t)~ILI_DELAY;

    ili9341_write(GPIO_PIN_RESET, command);
    for (uint8_t i = 0U; i < length; i++) {
      ili9341_write(GPIO_PIN_SET, *entry++);
    }
    if ((info & ILI_DELAY) != 0U) {
      HAL_Delay(200U);
    }
  }
}

void display_fill(volatile uint16_t *buffer, uint16_t color) {
  for (uint32_t i = 0U; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
    buffer[i] = color;
  }
}

static void draw_char(volatile uint16_t *buffer, const sFONT *font, uint16_t x,
                      uint16_t y, char c, uint16_t color, uint16_t background) {
  uint32_t row_bytes = (font->Width + 7U) / 8U;

  if (c < ' ' || c > '~') {
    c = '?';
  }

  const uint8_t *glyph =
      &font->table[(uint32_t)(c - ' ') * font->Height * row_bytes];

  for (uint32_t row = 0U; row < font->Height; row++) {
    uint32_t bits = 0U;

    for (uint32_t i = 0U; i < row_bytes; i++) {
      bits = (bits << 8U) | *glyph++;
    }
    for (uint32_t column = 0U; column < font->Width; column++) {
      uint32_t mask = 1UL << (row_bytes * 8U - 1U - column);

      buffer[display_index(x + column, y + row)] =
          (bits & mask) != 0U ? color : background;
    }
  }
}

void display_init(void) {
  MX_GPIO_Init();

  /* The gyroscope shares SPI5: keep it deselected, along with the LCD. */
  HAL_GPIO_WritePin(NCS_MEMS_SPI_GPIO_Port, NCS_MEMS_SPI_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);

  MX_FMC_Init();
  sdram_start();
  display_fill(DISPLAY_FRONT, COLOR_BLACK);
  display_fill(DISPLAY_BACK, COLOR_BLACK);

  MX_LTDC_Init();
  MX_SPI5_Init();
  ili9341_start();
}

void display_draw_text(volatile uint16_t *buffer, const sFONT *font, uint16_t x,
                       uint16_t y, const char *text, uint16_t color,
                       uint16_t background) {
  if ((uint32_t)y + font->Height > DISPLAY_HEIGHT) {
    return;
  }

  while (*text != '\0' && (uint32_t)x + font->Width <= DISPLAY_WIDTH) {
    draw_char(buffer, font, x, y, *text++, color, background);
    x = (uint16_t)(x + font->Width);
  }
}

void display_show_text(const char *text, uint16_t y) {
  const sFONT *font = &Font24;
  uint32_t length = strlen(text);
  uint32_t max_length = DISPLAY_WIDTH / font->Width;

  if (length > max_length) {
    length = max_length;
  }

  uint16_t x = (uint16_t)((DISPLAY_WIDTH - length * font->Width) / 2U);

  display_draw_text(DISPLAY_FRONT, font, x, y, text, COLOR_WHITE, COLOR_BLACK);
}
