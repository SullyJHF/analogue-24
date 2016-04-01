#include <pebble.h>

#define WIDTH  144
#define HEIGHT 168

static Window *window;
static Layer *main_clock_layer;

static GPoint centre;

static GPoint minute_end;
#define MINUTE_LENGTH 60

static GPoint hour_end;
#define HOUR_LENGTH 45

static GPoint lower_dot;

// angle from positive x axis from sun is just heliocentric longitudinal angle
// work that shit out for each planet

// to work out for the moon, use geocentric longitudinal angle and draw it from Earth

// ez

struct Planet {
  int id; // id of planet
  float n; // daily motion of planet in degrees / day
  float L; // longitude at epoch
  int orbits; // what planet this one orbits (for moons) this will mostly be the sun's id
  int s; // size (radius of the planet in pixels)
  int r; // distance from orbits to this planet (pixels)
  int l; // current day's orbits-centric longitude (mostly heliocentric, moon will be geocentric)
};

struct Planet sun; // I know the sun isn't a planet, I may rename the struct

struct Planet mercury;
struct Planet venus;
struct Planet earth;
struct Planet mars;
struct Planet jupiter;
struct Planet saturn;
struct Planet uranus;
struct Planet neptune;

struct Planet moon; // also the moon's not a planet

/****** drawing stuff ******/
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_width(ctx, 11);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, centre, minute_end);
  graphics_draw_line(ctx, lower_dot, lower_dot);


  graphics_context_set_stroke_color(ctx, GColorCyan);
  graphics_draw_line(ctx, centre, hour_end);
}

static GPoint set_end(int32_t angle, int length) {
  GPoint result;
  result.y = (-cos_lookup(angle) * length / TRIG_MAX_RATIO) + centre.y;
  result.x = (sin_lookup(angle) * length / TRIG_MAX_RATIO) + centre.x;
  return result;
}

static void update_hands() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  int year = tick_time->tm_year + 1900;
  int month = tick_time->tm_mon + 1;
  int day = tick_time->tm_mday;
  int hour = tick_time->tm_hour;
  int minute = tick_time->tm_min;

  float epoch_JD = 2450320.5; // 0h UT 25th august 1996

  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + 12 * a - 3;

  int current_JD = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;

  int d = current_JD - epoch_JD; // days since 0h UT 25th august 1996

  int32_t hle = (int)(earth.n * d + earth.L) % 360; // sick this works, gives heliocentric longitude of earth

  int32_t minute_frac = TRIG_MAX_ANGLE * minute / 60.0;
  int32_t hour_frac = TRIG_MAX_ANGLE * (hour + minute / 60.0) / 24.0;

  minute_end = set_end(minute_frac, MINUTE_LENGTH);
  hour_end = set_end(hour_frac, HOUR_LENGTH);

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %lu", d, hle);
}

/****** tick handler ******/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_hands();
  layer_mark_dirty(main_clock_layer);
}

/****** window life ******/
static void window_load(Window *window) {
  mercury.n = 4.092385;
  mercury.L = 281.18017;

  earth.n = 0.9855931;
  earth.L = 333.58600;

  centre = GPoint(WIDTH / 2, HEIGHT / 2);
  lower_dot = GPoint(WIDTH / 2, 158);

  minute_end = centre;
  hour_end = centre;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  main_clock_layer = layer_create(bounds);

  layer_set_update_proc(main_clock_layer, canvas_update_proc);
  layer_add_child(window_layer, main_clock_layer);
  update_hands();
}

static void window_unload(Window *window) {
  layer_destroy(main_clock_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  layer_mark_dirty(main_clock_layer);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
