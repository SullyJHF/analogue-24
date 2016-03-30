#include <pebble.h>

#define WIDTH  144
#define HEIGHT 168

static Window *window;
static Layer *main_clock_layer;

static GPoint centre;

static GPoint minute_end;
#define MINUTE_LENGTH 65

static GPoint hour_end;
#define HOUR_LENGTH 40

/****** drawing stuff ******/
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Set the line color to white for the minute hand
  graphics_context_set_stroke_color(ctx, GColorWhite);
  // Set the stroke width
  graphics_context_set_stroke_width(ctx, 7);
  graphics_draw_line(ctx, centre, minute_end);

  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_draw_line(ctx, centre, hour_end);

  // Set the line color to red for the hour hand
}

/****** tick handler ******/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  int hour = tick_time->tm_hour;
  int minute = tick_time->tm_min;

  int32_t minute_angle = TRIG_MAX_ANGLE * minute / 60;
  minute_end.y = (-cos_lookup(minute_angle) * MINUTE_LENGTH / TRIG_MAX_RATIO) + centre.y;
  minute_end.x = (sin_lookup(minute_angle) * MINUTE_LENGTH / TRIG_MAX_RATIO) + centre.x;

  int32_t hour_angle = TRIG_MAX_ANGLE * hour / 24;
  hour_end.y = (-cos_lookup(hour_angle) * HOUR_LENGTH / TRIG_MAX_RATIO) + centre.y;
  hour_end.x = (sin_lookup(hour_angle) * HOUR_LENGTH / TRIG_MAX_RATIO) + centre.x;

  layer_mark_dirty(main_clock_layer);
}

static void window_load(Window *window) {
  centre = GPoint(WIDTH / 2, HEIGHT / 2);

  minute_end = centre;
  hour_end = centre;
  // get the root layer
  Layer *window_layer = window_get_root_layer(window);
  // get the bounds of the root layer
  GRect bounds = layer_get_bounds(window_layer);
  // create the main clock layer
  main_clock_layer = layer_create(bounds);
  // set the update procedure
  layer_set_update_proc(main_clock_layer, canvas_update_proc);
  // add the main clock layer to the root layer
  layer_add_child(window_layer, main_clock_layer);
}

static void window_unload(Window *window) {
  // destroy the main clock layer on exit
  layer_destroy(main_clock_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
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
