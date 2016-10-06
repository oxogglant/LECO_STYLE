#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_day_layer, *s_bt_layer;
static GFont s_time_font, s_bt_font;
static int s_battery_level;
static Layer *s_battery_layer;

static void update_time() {
  // get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%B %e", tick_time);
  
  static char day_buffer[16];
  strftime(day_buffer, sizeof(day_buffer), "%A", tick_time);
  
  
  //Display this time on the TextLayer 
  text_layer_set_text(s_time_layer, s_buffer+(('0' == s_buffer[0])?1:0));
  text_layer_set_text(s_date_layer,date_buffer);
  text_layer_set_text(s_day_layer,day_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 144.0F);
  
  // Draw the background
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
  
}

static void bluetooth_callback(bool connected) {
  // Show text if disconnected
  layer_set_hidden(text_layer_get_layer(s_bt_layer),connected);
  
  if(!connected) {
    //Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void main_window_load(Window *window){
  // Create GFonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_48));
  s_bt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_16));
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
    GRect(0, PBL_IF_BW_ELSE(48, 48), bounds.size.w,50));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer,GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the Date TextLayer
  s_date_layer = text_layer_create(GRect(0,108,144,30));
  text_layer_set_text_color(s_date_layer,GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  //Add the date layer to window
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_date_layer));
  
  // Create the Day TextLayer
  s_day_layer = text_layer_create(GRect(0,132,144,30));
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_background_color(s_day_layer,GColorClear);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  // Add the day layer to window
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_day_layer));
  
  // Create the battery meter layer
  s_battery_layer = layer_create(GRect(0,0,144,3));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  
  //Add battery layer to window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create the Bluetooh connection layer
  s_bt_layer = text_layer_create( GRect(0,13,144,30));
  text_layer_set_text_color(s_bt_layer,GColorWhite);
  text_layer_set_background_color(s_bt_layer, GColorClear);
  text_layer_set_text_alignment(s_bt_layer,GTextAlignmentCenter);
  text_layer_set_font(s_bt_layer, s_bt_font);
  text_layer_set_text(s_bt_layer, "DISCONNECTED");
  
  // Add the BT layer to the window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bt_layer));
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window) {
 
  //Destroy GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_bt_font);
  
    // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_bt_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
    //set background color
  window_set_background_color(s_main_window, GColorBlack);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window,(WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the window on the watch, with animated = true
  window_stack_push(s_main_window, true);  
    
  //register with TickTimerService
tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  // Make sure time is displayed from the start
  update_time();
  
  // Make sure battery is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit(){
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}