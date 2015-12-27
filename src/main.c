#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

/*
static char * get_hours(int hours) {
  
  // deal with the hours
  switch(hours) {
    case 12:
      return "noon";
      break;
    case 0:
      return "midnight";
      break;
    default:
      return 0;
  }
}
*/

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                           "%H:%M" : "%I:%M", tick_time);
  
  //hour
  static char out_hour_text[9];
  static char out_hour[9];
  static int out_hour_num[3];
  
  //half
  static char out_half[5];
  
  //oclock
  static char out_oclock[8];
  
  //mins
  static int out_mins[2];
  static char out_mins_c[3];
  
  //til/past
  static char out_til_past[5];
  
  
  //final out text
  static char out_text[24];
  
  //get the hours and minutes
  int bhour = tick_time->tm_hour;
  int bmin = tick_time->tm_min;
  
  // deal with the hours
  switch(bhour) {
    case 12:
      strcpy(out_hour_text, "noon");
      break;
    case 0:
      strcpy(out_hour_text, "midnight");
      break;
    default:
        if (bhour > 12){
          *out_hour_num = bhour - 12;
        }else{
          *out_hour_num = bhour;
        }   
        strcpy(out_oclock, "o'clock");
  }
  
  if (bmin == 30){
    strcpy(out_half, "half");
  }
  
  if (*out_hour_num != 0) {
    snprintf(out_hour,30,"%s %i",out_hour_text,*out_hour_num);
  }else{
    snprintf(out_hour,30,"%s",out_hour_text);
  }
  
  //out_mins + out_half + out_til_past + out_hour + out_oclock
  
  snprintf(out_mins_c,sizeof(out_mins_c),"%i",*out_mins);
  
  strncat (out_text, out_mins_c, 20);
  strncat (out_text, out_half, 20);
  strncat (out_text, out_til_past, 20);
  strncat (out_text, out_hour, 20);
  strncat (out_text, out_oclock, 20);
  
  // Display this time on the TextLayer
  //text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_time_layer, out_text);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_time_layer, GTextOverflowModeWordWrap);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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