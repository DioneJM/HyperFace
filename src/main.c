#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer; // 12 hour time layer
static TextLayer *battery_layer; //Battery display 
static TextLayer *date_layer; //Current date display
static TextLayer *inner_rect;

// The layers for the background
static TextLayer *background_second_half;


static void display_batt_attr(BatteryChargeState batt_state){
  static char batt_status[40];
  char percent = '%';
  if(batt_state.is_charging){
    snprintf(batt_status, sizeof(batt_status), "charging..."); 
  }else{
    snprintf(batt_status, sizeof(batt_status), "%d%c", batt_state.charge_percent, percent);
  }
  
  text_layer_set_text(battery_layer, batt_status); 

}


static void update_time(){
  // Get a tm struct
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  static char t_buffer[8];
  static char date_buffer[16];
  strftime(s_buffer, sizeof(s_buffer), "%I:%M", tick_time);
  strftime(t_buffer, sizeof(s_buffer), "%H:%M", tick_time);
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
    update_time();
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  // Background
  background_second_half = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  inner_rect = text_layer_create(GRect(12, 15, bounds.size.w - 24, bounds.size.h - 30));
  text_layer_set_background_color(background_second_half, GColorDarkGray);
  text_layer_set_background_color(inner_rect, GColorLightGray);
  
  
  // Battery
  battery_layer = text_layer_create(GRect(35, 0, 80, 50));
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_text_color(battery_layer, GColorLavenderIndigo );
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_text(battery_layer, "000%");
  
  // Time Displays
  s_time_layer = text_layer_create(GRect(0, bounds.size.h/2 - 30, bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorElectricBlue );
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Date
  date_layer = text_layer_create(GRect(0, bounds.size.h - 17, bounds.size.w, 50));
  text_layer_set_text_color(date_layer, GColorLavenderIndigo );
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);  
  
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(background_second_half));
  layer_add_child(window_layer, text_layer_get_layer(inner_rect));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  // Handlers
  display_batt_attr(battery_state_service_peek());


  
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(battery_layer);
  text_layer_destroy(background_second_half);
  text_layer_destroy(date_layer);
  text_layer_destroy(inner_rect);

}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  //Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Register with BatteryStateService
  battery_state_service_subscribe(display_batt_attr);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Init event handlers
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
