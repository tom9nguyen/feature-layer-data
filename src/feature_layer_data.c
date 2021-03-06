/* Inspired by peapod's progress bar layer: https://github.com/Kathatrine/peapod */
#include <pebble.h>

#define SPEED_MAX 5000
#define SPEED_MIN 50
#define DEFAULT_SPEED 750

typedef Layer ProgressBarLayer;

static Window *s_main_window;
static TextLayer *s_done_text, *s_instruction_text;
static AppTimer *s_progress_timer;
static ProgressBarLayer *s_progress_bar;
static unsigned int s_current_speed = DEFAULT_SPEED;

typedef struct {
  unsigned int progress; // how full the progress bar is
} ProgressData;

static void progress_timer_callback(void *data) {
  ProgressData *d = layer_get_data(s_progress_bar);
  if (d->progress > 129) {
    s_progress_timer = NULL;
    text_layer_set_text(s_done_text, "Done!");
  } else {
    s_progress_timer = app_timer_register(s_current_speed, progress_timer_callback, NULL);
    layer_mark_dirty(s_progress_bar);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  ProgressData *data = layer_get_data(s_progress_bar);
  data->progress = 0;
  s_current_speed = DEFAULT_SPEED;
  if (s_progress_timer) {
    app_timer_cancel(s_progress_timer);
  }
  s_progress_timer = app_timer_register(s_current_speed, progress_timer_callback, NULL);
  text_layer_set_text(s_done_text, "");
  layer_mark_dirty(s_progress_bar);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_current_speed > SPEED_MIN) {
    s_current_speed -= 50;
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_current_speed < SPEED_MAX) {
    s_current_speed += 50;
  }
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void progress_bar_layer_update(ProgressBarLayer *bar, GContext *ctx) {
  ProgressData *data = (ProgressData *)layer_get_data(bar);

  // Outline the progress bar
  graphics_context_set_stroke_color(ctx, GColorBlack);
  GRect bounds = layer_get_bounds(bar);
  graphics_draw_round_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 4);

  // Fill the progress bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, data->progress, bounds.size.h), 4, GCornersAll);
  ++data->progress;
}

static ProgressBarLayer* progress_bar_layer_create(void) {
  ProgressBarLayer *progress_bar_layer = layer_create_with_data(GRect(PBL_IF_RECT_ELSE(6, 25), 
                                                                      120, 130, 8), 
                                                                sizeof(ProgressData));
  layer_set_update_proc(progress_bar_layer, progress_bar_layer_update);
  layer_mark_dirty(progress_bar_layer);

  return progress_bar_layer;
}

static void progress_bar_destroy(ProgressBarLayer *progress_bar_layer) {
  layer_destroy(progress_bar_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_instruction_text = text_layer_create(GRect(PBL_IF_RECT_ELSE(0, 25), PBL_IF_RECT_ELSE(0, 30), 
                                               bounds.size.w, 73));
  layer_add_child(window_layer, text_layer_get_layer(s_instruction_text));
  text_layer_set_text(s_instruction_text, "[UP] increase speed\n[SEL] reset\n[DN] decrease speed");

  s_progress_bar = progress_bar_layer_create();
  layer_add_child(window_layer, s_progress_bar);

  s_done_text = text_layer_create(GRect(0, bounds.size.h / 2, bounds.size.w, 20));
  text_layer_set_text_alignment(s_done_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_done_text));

  // Start the progress timer
  s_progress_timer = app_timer_register(s_current_speed, progress_timer_callback, NULL);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_done_text);
  progress_bar_destroy(s_progress_bar);
}

static void init() {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
