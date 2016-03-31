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

  int hour = tick_time->tm_hour;
  int minute = tick_time->tm_min;

  int32_t minute_frac = TRIG_MAX_ANGLE * minute / 60.0;
  int32_t hour_frac = TRIG_MAX_ANGLE * (hour + minute / 60.0) / 24.0;

  minute_end = set_end(minute_frac, MINUTE_LENGTH);
  hour_end = set_end(hour_frac, HOUR_LENGTH);
}

/****** tick handler ******/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_hands();
  layer_mark_dirty(main_clock_layer);
}

/****** window life ******/
static void window_load(Window *window) {
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
