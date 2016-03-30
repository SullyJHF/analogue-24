#include <pebble.h>
#include <time.h>

#define WIDTH  144
#define HEIGHT 168

static Window *window;
static Layer *main_clock_layer;

static GPoint minute_start;
static GPoint minute_end;
#define HOUR_LENGTH 65

static GPoint hour_start;
static GPoint hour_end;
#define MINUTE_LENGTH 40

/****** drawing stuff ******/
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Set the line color to white for the minute hand
  graphics_context_set_stroke_color(ctx, GColorWhite);

  // Set the line color to red for the hour hand
}

/****** tick handler ******/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  int hour = tick_time->tm_hour;
  int minute = tick_time->tm_min;


  APP_LOG(APP_LOG_LEVEL_INFO, "%d:%d", hour, minute);
  // mark the clock layer as dirty to call the update proc
  layer_mark_dirty(main_clock_layer);
}

static void window_load(Window *window) {
  minute_start = GPoint(WIDTH / 2, HEIGHT / 2);
  hour_start = minute_start;
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
