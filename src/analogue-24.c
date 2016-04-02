#include <pebble.h>

#define WIDTH  144
#define HEIGHT 168

static Window *window;
static Layer *main_clock_layer;

static GPoint centre;

static GPoint minute_end;
#define MINUTE_LENGTH 65

static GPoint hour_end;
#define HOUR_LENGTH 45

struct Planet {
  int id; // id of planet
  float n; // daily motion of planet in degrees / day
  float L; // longitude at epoch
  int o; // what planet this one orbits (for moons) this will mostly be the sun's id
  int s; // size (diameter of the planet in pixels)
  int d; // distance from orbits to this planet (pixels)
  int32_t l; // current day's orbits-centric longitude (mostly heliocentric, moon will be geocentric)
  int x;  // x position in pixels of this planet
  int y;  // y position in pixels of this planet
  GColor colour; // planet's colour
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
static void draw_planet(struct Planet p, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, p.colour);
  graphics_context_set_stroke_width(ctx, p.s);
  GPoint pos = GPoint(p.x, p.y);
  graphics_draw_line(ctx, pos, pos);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_context_set_stroke_width(ctx, 7);

  graphics_context_set_stroke_color(ctx, GColorFromRGB(100, 100, 100));
  graphics_draw_line(ctx, centre, minute_end);

  graphics_context_set_stroke_color(ctx, GColorFromRGB(0, 100, 100));
  graphics_draw_line(ctx, centre, hour_end);

  draw_planet(sun, ctx);
  draw_planet(mercury, ctx);
  draw_planet(venus, ctx);
  draw_planet(earth, ctx);
  draw_planet(mars, ctx);
  draw_planet(jupiter, ctx);
  draw_planet(saturn, ctx);
  draw_planet(uranus, ctx);
  draw_planet(neptune, ctx);
}

static GPoint set_end(int32_t angle, int length) {
  GPoint result;
  result.y = (-cos_lookup(angle) * length / TRIG_MAX_RATIO) + centre.y;
  result.x = (sin_lookup(angle) * length / TRIG_MAX_RATIO) + centre.x;
  return result;
}

static struct Planet get_planet_position(struct Planet p, int d) {
  p.l = TRIG_MAX_ANGLE * ((((int)(p.n * d + p.L) % 360)) / 360.0);
  p.x = (sin_lookup(-p.l) * p.d / TRIG_MAX_RATIO) + centre.x;
  p.y = (-cos_lookup(-p.l) * p.d / TRIG_MAX_RATIO) + centre.y;
  return p;
}

static void calculate_planets() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int year = tick_time->tm_year + 1900;
  int month = tick_time->tm_mon + 1;
  int day = tick_time->tm_mday;

  float epoch_JD = 2450320.5; // 0h UT 25th august 1996

  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + 12 * a - 3;

  int current_JD = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;

  int d = current_JD - epoch_JD; // days since 0h UT 25th august 1996

  sun = get_planet_position(sun, d);
  mercury = get_planet_position(mercury, d);
  venus = get_planet_position(venus, d);
  earth = get_planet_position(earth, d);
  mars = get_planet_position(mars, d);
  jupiter = get_planet_position(jupiter, d);
  saturn = get_planet_position(saturn, d);
  uranus = get_planet_position(uranus, d);
  neptune = get_planet_position(neptune, d);
}

static void update_hands() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  int hour = tick_time->tm_hour;
  int minute = tick_time->tm_min;

  int32_t minute_frac = TRIG_MAX_ANGLE * minute / 60.0;
  int32_t hour_frac = TRIG_MAX_ANGLE * (hour + minute / 60.0) / 24.0;

  minute_end = set_end(minute_frac, MINUTE_LENGTH);
  hour_end = set_end(hour_frac, HOUR_LENGTH);
}

/****** tick handlers ******/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_hands();
  layer_mark_dirty(main_clock_layer);
}

static void day_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  calculate_planets();
  layer_mark_dirty(main_clock_layer);
}

/****** window life ******/
static void window_load(Window *window) {
  /**** set all planet default values ****/
  sun.id = 0;
  sun.n = 0;
  sun.L = 0;
  sun.s = 19;
  sun.colour = GColorYellow;

  mercury.id = 1;
  mercury.n = 4.092385F;
  mercury.L = 281.18017F;
  mercury.s = 3;
  mercury.d = 14;
  mercury.colour = GColorFromRGB(153, 51, 0);

  venus.id = 2;
  venus.n = 1.602159F;
  venus.L = 20.17002F;
  venus.s = 3;
  venus.d = 18;
  venus.colour = GColorFromRGB(153, 153, 153);

  earth.id = 3;
  earth.n = 0.9855931F;
  earth.L = 333.58600F;
  earth.s = 3;
  earth.d = 23;
  earth.colour = GColorGreen;

  mars.id = 4;
  mars.n = 0.5240218F;
  mars.L = 73.77336F;
  mars.s = 3;
  mars.d = 31;
  mars.colour = GColorRed;

  jupiter.id = 5;
  jupiter.n = 0.08310024F;
  jupiter.L = 292.64251F;
  jupiter.s = 9;
  jupiter.d = 40;
  jupiter.colour = GColorFromRGB(255, 204, 102);

  saturn.id = 6;
  saturn.n = 0.03333857F;
  saturn.L = 8.91324F;
  saturn.s = 7;
  saturn.d = 52;
  saturn.colour = GColorFromRGB(255, 204, 204);

  uranus.id = 7;
  uranus.n = 0.01162075F;
  uranus.L = 298.87491F;
  uranus.s = 5;
  uranus.d = 61;
  uranus.colour = GColorFromRGB(102, 204, 255);

  neptune.id = 8;
  neptune.n = 0.005916098F;
  neptune.L = 297.58472F;
  neptune.s = 5;
  neptune.d = 69;
  neptune.colour = GColorFromRGB(26, 178, 255);
  /**** end setting planets ****/

  centre = GPoint(WIDTH / 2, HEIGHT / 2);

  minute_end = centre;
  hour_end = centre;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  main_clock_layer = layer_create(bounds);

  layer_set_update_proc(main_clock_layer, canvas_update_proc);
  layer_add_child(window_layer, main_clock_layer);
  update_hands();
  calculate_planets();
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
  tick_timer_service_subscribe(DAY_UNIT, day_tick_handler);
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
