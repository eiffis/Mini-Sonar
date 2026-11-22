#include "radar.h"
#include "display.h"
#include <math.h>
#include <stddef.h>

#define RADAR_RANGE_CM 100.0f
#define RADAR_MIN_CM 2.0f
#define SENSOR_MAX_CM 400.0f

#define RADAR_HALF_ANGLE_DEG 45
#define RADAR_DIRECTION 1

#define PULSE_MIN_US 1000U
#define PULSE_STEP_US 10U
#define RADAR_STEPS 101
#define RADAR_CENTER_STEP 50

#define CENTER_X 160
#define CENTER_Y 232
#define RADIUS_PX 200
#define RING_COUNT 4
#define ARC_SEGMENTS 64
#define BLIP_HALF_SIZE 2
#define NO_BLIP (-1)

#define TEXT_Y 3U
#define DISTANCE_X 4U
#define ANGLE_X 197U
#define TITLE_X 143U
#define TITLE_Y 9U

#define DEG_TO_RAD 0.017453292f

typedef struct {
  int16_t x;
  int16_t y;
} point_t;

static point_t sweep_ends[RADAR_STEPS];
static point_t blips[RADAR_STEPS];
static int16_t last_step = -1;

static int16_t round_to_int(float value) {
  return (int16_t)(value >= 0.0f ? value + 0.5f : value - 0.5f);
}

static point_t polar(float radius, float angle_deg) {
  float angle = angle_deg * DEG_TO_RAD;
  point_t point = {
      (int16_t)(CENTER_X + round_to_int(radius * sinf(angle))),
      (int16_t)(CENTER_Y - round_to_int(radius * cosf(angle))),
  };

  return point;
}

static float step_angle(int16_t step) {
  return (float)(RADAR_DIRECTION * RADAR_HALF_ANGLE_DEG *
                 (RADAR_CENTER_STEP - step)) /
         (float)RADAR_CENTER_STEP;
}

static uint32_t pixel_index(int32_t x, int32_t y) {
  return display_index((uint32_t)x, (uint32_t)y);
}

static int is_on_screen(int32_t x, int32_t y) {
  return x >= 0 && y >= 0 && x < (int32_t)DISPLAY_WIDTH &&
         y < (int32_t)DISPLAY_HEIGHT;
}

static void walk_line(volatile uint16_t *target,
                      const volatile uint16_t *source, point_t from, point_t to,
                      uint16_t color) {
  int32_t x = from.x;
  int32_t y = from.y;
  int32_t dx = to.x > from.x ? to.x - from.x : from.x - to.x;
  int32_t dy = to.y > from.y ? from.y - to.y : to.y - from.y;
  int32_t step_x = from.x < to.x ? 1 : -1;
  int32_t step_y = from.y < to.y ? 1 : -1;
  int32_t error = dx + dy;

  while (1) {
    if (is_on_screen(x, y)) {
      uint32_t index = pixel_index(x, y);

      target[index] = source != NULL ? source[index] : color;
    }
    if (x == to.x && y == to.y) {
      break;
    }

    int32_t doubled = 2 * error;

    if (doubled >= dy) {
      error += dy;
      x += step_x;
    }
    if (doubled <= dx) {
      error += dx;
      y += step_y;
    }
  }
}

static void draw_grid(void) {
  static const char *const labels[RING_COUNT] = {"25", "50", "75", "100"};
  const point_t center = {CENTER_X, CENTER_Y};

  for (int32_t ring = 1; ring <= RING_COUNT; ring++) {
    float radius = (float)(RADIUS_PX * ring) / (float)RING_COUNT;
    point_t previous = polar(radius, -(float)RADAR_HALF_ANGLE_DEG);

    for (int32_t i = 1; i <= ARC_SEGMENTS; i++) {
      float angle = -(float)RADAR_HALF_ANGLE_DEG +
                    (float)(2 * RADAR_HALF_ANGLE_DEG * i) / (float)ARC_SEGMENTS;
      point_t current = polar(radius, angle);

      walk_line(DISPLAY_BACK, NULL, previous, current, COLOR_DARK_GREEN);
      previous = current;
    }
  }

  for (int32_t i = -2; i <= 2; i++) {
    float angle = (float)(RADAR_HALF_ANGLE_DEG * i) / 2.0f;

    walk_line(DISPLAY_BACK, NULL, center, polar((float)RADIUS_PX, angle),
              COLOR_DARK_GREEN);
  }

  for (int32_t ring = 1; ring <= RING_COUNT; ring++) {
    int32_t radius = RADIUS_PX * ring / RING_COUNT;

    display_draw_text(DISPLAY_BACK, &Font12, CENTER_X + 3,
                      (uint16_t)(CENTER_Y - radius + 2), labels[ring - 1],
                      COLOR_GREEN, COLOR_BLACK);
  }
}

static void restore_blip(point_t blip) {
  for (int32_t y = blip.y - BLIP_HALF_SIZE; y <= blip.y + BLIP_HALF_SIZE; y++) {
    for (int32_t x = blip.x - BLIP_HALF_SIZE; x <= blip.x + BLIP_HALF_SIZE;
         x++) {
      if (is_on_screen(x, y)) {
        uint32_t index = pixel_index(x, y);

        DISPLAY_FRONT[index] = DISPLAY_BACK[index];
      }
    }
  }
}

static void draw_blip(point_t blip) {
  for (int32_t y = blip.y - BLIP_HALF_SIZE; y <= blip.y + BLIP_HALF_SIZE; y++) {
    for (int32_t x = blip.x - BLIP_HALF_SIZE; x <= blip.x + BLIP_HALF_SIZE;
         x++) {
      if (is_on_screen(x, y)) {
        DISPLAY_FRONT[pixel_index(x, y)] = COLOR_RED;
      }
    }
  }
}

static void format_distance(char *text, float distance_cm) {
  static const char no_echo[] = "  --- cm";

  if (distance_cm < 0.0f || distance_cm >= SENSOR_MAX_CM) {
    for (uint32_t i = 0U; i < sizeof(no_echo); i++) {
      text[i] = no_echo[i];
    }
    return;
  }

  uint32_t tenths = (uint32_t)(distance_cm * 10.0f + 0.5f);
  uint32_t whole = tenths / 10U;

  text[0] = whole >= 100U ? (char)('0' + whole / 100U) : ' ';
  text[1] = whole >= 10U ? (char)('0' + (whole / 10U) % 10U) : ' ';
  text[2] = (char)('0' + whole % 10U);
  text[3] = '.';
  text[4] = (char)('0' + tenths % 10U);
  text[5] = ' ';
  text[6] = 'c';
  text[7] = 'm';
  text[8] = '\0';
}

static void format_angle(char *text, int16_t step) {
  int32_t offset = RADAR_DIRECTION * (RADAR_CENTER_STEP - step);
  int32_t magnitude = offset < 0 ? -offset : offset;
  int32_t degrees = (magnitude * RADAR_HALF_ANGLE_DEG + RADAR_CENTER_STEP / 2) /
                    RADAR_CENTER_STEP;

  text[0] = offset < 0 ? '-' : '+';
  text[1] = (char)('0' + degrees / 10);
  text[2] = (char)('0' + degrees % 10);
  text[3] = ' ';
  text[4] = 'd';
  text[5] = 'e';
  text[6] = 'g';
  text[7] = '\0';
}

void radar_init(void) {
  draw_grid();

  for (uint32_t i = 0U; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
    DISPLAY_FRONT[i] = DISPLAY_BACK[i];
  }

  for (int16_t step = 0; step < RADAR_STEPS; step++) {
    sweep_ends[step] = polar((float)RADIUS_PX, step_angle(step));
    blips[step].x = NO_BLIP;
    blips[step].y = NO_BLIP;
  }

  display_draw_text(DISPLAY_FRONT, &Font12, TITLE_X, TITLE_Y, "SONAR",
                    COLOR_GREEN, COLOR_BLACK);
}

void radar_update(uint16_t pulse_us, float distance_cm) {
  const point_t center = {CENTER_X, CENTER_Y};
  char distance_text[9];
  char angle_text[8];
  int32_t step = pulse_us > PULSE_MIN_US
                     ? (int32_t)((pulse_us - PULSE_MIN_US) / PULSE_STEP_US)
                     : 0;

  if (step >= RADAR_STEPS) {
    step = RADAR_STEPS - 1;
  }

  if (last_step >= 0) {
    walk_line(DISPLAY_FRONT, DISPLAY_BACK, center, sweep_ends[last_step], 0U);
  }

  if (blips[step].x != NO_BLIP) {
    restore_blip(blips[step]);
    blips[step].x = NO_BLIP;
    blips[step].y = NO_BLIP;
  }

  if (distance_cm >= RADAR_MIN_CM && distance_cm <= RADAR_RANGE_CM) {
    float radius = distance_cm / RADAR_RANGE_CM * (float)RADIUS_PX;

    blips[step] = polar(radius, step_angle((int16_t)step));
  }

  walk_line(DISPLAY_FRONT, NULL, center, sweep_ends[step], COLOR_GREEN);

  for (int32_t i = 0; i < RADAR_STEPS; i++) {
    if (blips[i].x != NO_BLIP) {
      draw_blip(blips[i]);
    }
  }

  last_step = (int16_t)step;

  format_distance(distance_text, distance_cm);
  format_angle(angle_text, (int16_t)step);
  display_draw_text(DISPLAY_FRONT, &Font24, DISTANCE_X, TEXT_Y, distance_text,
                    COLOR_WHITE, COLOR_BLACK);
  display_draw_text(DISPLAY_FRONT, &Font24, ANGLE_X, TEXT_Y, angle_text,
                    COLOR_WHITE, COLOR_BLACK);
}
