#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

//function used to vertically align text on watchface
static void verticalAlignTextLayer(TextLayer *layer) {
    GRect frame = layer_get_frame(text_layer_get_layer(layer));
    GSize content = text_layer_get_content_size(layer);
    layer_set_frame(text_layer_get_layer(layer), 
           GRect(frame.origin.x, frame.origin.y + (frame.size.h - content.h - 5) / 2, 
           frame.size.w, content.h + 3));
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  //hour
  static char out_hour_text[11];
  static char out_hour[11];
  static int out_hour_num[5];
  
  //half
  static char out_half[7];
  
  //oclock
  static char out_oclock[10] = "o'clock";
  
  //mins
  static int out_mins[4];
  static char out_mins_c[5];
  
  //til/past
  static char out_til_past[7];
  
  //final out text
  static char out_text[32];
  
  //get the hours and minutes
  int bhour = tick_time->tm_hour;
  int bmin = tick_time->tm_min;
  
  //enable for testing different times
  //bhour = 12;
  //bmin = 28;

  *out_mins = bmin;
  *out_hour_num = bhour;
  
  //if we're at 30 min mark, then print half
  if (*out_mins == 30){
    strcpy(out_half, "half ");
  }else{
    strcpy(out_half, "");
    //if we're over 30 min mark, then we need the mins remaining til the hour
    if (bmin > 30) {
      *out_mins = (bmin - 60) * -1;
    }else{
      //otherwise, we just get the mins after the hour
      *out_mins = bmin;
    }
  }
  
  //determine if we should use til or past
  if (bmin > 30){
    strcpy(out_til_past, "til ");
    //since it's til, we need to add an hour to the current hour
    *out_hour_num = *out_hour_num + 1;
  }else{
    strcpy(out_til_past, "past ");
  }
  
  //check to see if 12 hour setting and if so and greater than 12 then subtract 12 from hours
  if (!clock_is_24h_style() && *out_hour_num > 12){
    *out_hour_num = *out_hour_num - 12;
  }  
  
  //if hour = 12, then it's noon
  if (*out_hour_num == 12) {
    strcpy(out_hour_text, "noon");
  }
  
  //if hour = 0 then it's midnight
  if (*out_hour_num == 0) {
    strcpy(out_hour_text, "midnight");
  }
  
  //if it's not noon or midnight then we output the number, if it is then we output text for hour
  if ((*out_hour_num != 0) && (*out_hour_num != 12)) {
    snprintf(out_hour,3,"%i",*out_hour_num);
  }else{
    snprintf(out_hour,9,"%s",out_hour_text);
  }
  
  //order for constructing the output...
  //out_mins + out_half + out_til_past + out_hour + out_oclock
  
  //clear out the output before we populate it
  strcpy(out_text, "");
  
  snprintf(out_mins_c,sizeof(out_mins),"%i",*out_mins);
  
  strcat (out_mins_c, " ");
  
  if ((bmin != 30) && (bmin != 0)) {
    strcat (out_text, out_mins_c); 
  }
  
  strcat (out_text, out_half);
  
  if (bmin != 0) {
    strcat (out_text, out_til_past);
  }
  
  strcat (out_hour, " ");
  strcat (out_text, out_hour);
  
  if ((*out_hour_num != 12) && (*out_hour_num != 0)) {
    strcat (out_text, out_oclock);
  }

  // Display this time on the TextLayer
  //text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_time_layer, out_text);
  
  //adjust the layer size so that the text is always vertically aligned
  verticalAlignTextLayer(s_time_layer);
  
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
      GRect(0, PBL_IF_ROUND_ELSE(40, 52), bounds.size.w, 100));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "finding the time...");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_time_layer, GTextOverflowModeWordWrap);
  
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