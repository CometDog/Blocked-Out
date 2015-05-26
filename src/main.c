/*
Blocked Out
    Copyright (C) 2015 James Downs
    james.j.downs@icloud.com
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    
*/

#include "pebble.h"
#include "gcolor_definitions.h"
#include "libs/pebble-assist.h"
#include "elements.h"

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);

  while(t != NULL) {
    switch(t->key) {
    case COUNTRY:
      country = (int)t->value->int8;
      break;
    case TEMPERATURE:
      degree = (int)t->value->int16;
      break;
    case TEMP_LOW:
      low = (int)t->value->int16;
      break;
    case TEMP_HIGH:
      high = (int)t->value->int16;
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "] %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
  
  if (country == 1) {
    degree = (32 + degree) * 1.8;
    low = (32 + low) * 1.8;
    high = (32 + high) * 1.8;
  }

  degree = degree + 10;
  low = low + 10;
  high = high + 10;
  
  if (degree < 100) {
    print_int(s_weather_buffer, "%d°", degree);
  }
  else {
    print_int(s_weather_buffer, "%d", degree);
  }
  
  if (low < 100) {
    print_int(s_low_buffer, "L:%d°", low);
  }
  else {
    print_int(s_low_buffer, "L:%d", low);
  }
  
  if (high < 100) {
    print_int(s_high_buffer, "H:%d°", high);
  }
  else {
    print_int(s_high_buffer, "H:%d", high);
  }

  text_layer_set_text(s_weather_label, s_weather_buffer);
  text_layer_set_text(s_low_label, s_low_buffer);
  text_layer_set_text(s_high_label, s_high_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

char *upcase(char *str)
{
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 0x20;
        }
    }
    return str;
}

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  if (finished && again == true) {
    running = false;
    again = false;
    do_reverse = false;
    layer_mark_dirty(s_background_layer);
    do_animation();
    do_hour1 = false;
    do_hour2 = false;
    do_minute1 = false;
    do_minute2 = false;
  }
}

static void shake_animation() {
  GRect shake_start, shake_finish;
  
  shake_start = GRect(0,73,144,0);
  shake_finish = GRect(0,73,144,28);
  
  if (shake_reverse != true) {
    s_shake_animation = property_animation_create_layer_frame(s_lht_label, &shake_start, &shake_finish);
  }
  else {
    s_shake_animation = property_animation_create_layer_frame(s_lht_label, &shake_finish, &shake_start);
  }
  animation_set_duration((Animation*)s_shake_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_shake_animation, ANIM_DELAY_SUN);
  animation_set_curve((Animation*)s_shake_animation, AnimationCurveLinear);
  animation_schedule((Animation*)s_shake_animation);
}

static void info_animation() {
  GRect info_start, info_finish;
  GRect date_1_start, date_1_finish;
  GRect date_2_start, date_2_finish;
  
  info_start = GRect(0,0,144,0);
  info_finish = GRect(0,0,144,168);
  
  date_1_start = GRect(0,0,0,168);
  date_1_finish = GRect(0,0,72,168);
  
  date_2_start = GRect(144,0,0,168);
  date_2_finish = GRect(72,0,72,168);
  
  s_info_animation = property_animation_create_layer_frame(s_info_layer, &info_start, &info_finish);
  animation_set_duration((Animation*)s_info_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_info_animation, ANIM_DELAY_INFO);
  animation_set_curve((Animation*)s_info_animation, AnimationCurveLinear);
  animation_schedule((Animation*)s_info_animation);
  
  s_date_1_animation = property_animation_create_layer_frame(s_date_1_layer, &date_1_start, &date_1_finish);
  animation_set_duration((Animation*)s_date_1_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_date_1_animation, ANIM_DELAY_INFO);
  animation_set_curve((Animation*)s_date_1_animation, AnimationCurveLinear);
  animation_schedule((Animation*)s_date_1_animation);
  
  s_date_2_animation = property_animation_create_layer_frame(s_date_2_layer, &date_2_start, &date_2_finish);
  animation_set_duration((Animation*)s_date_2_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_date_2_animation, ANIM_DELAY_INFO);
  animation_set_curve((Animation*)s_date_2_animation, AnimationCurveLinear);
  animation_schedule((Animation*)s_date_2_animation);
}

static void do_animation() {
  if (do_reverse) {
    running = true;
  }
  
  GRect hour1_start, hour1_finish;
  GRect hour2_start, hour2_finish;
  GRect minute1_start, minute1_finish;
  GRect minute2_start, minute2_finish;
  
  hour1_start = GRect(-144, 0, BOX_X, BOX_Y);
  hour1_finish = GRect(0, 0, BOX_X, BOX_Y);
  
  hour2_start = GRect(81, -168, BOX_X, BOX_Y);
  hour2_finish = GRect(81, 0, BOX_X, BOX_Y);

  minute1_start = GRect(0, 266, BOX_X, BOX_Y);
  minute1_finish = GRect(0, 98, BOX_X, BOX_Y);

  minute2_start = GRect(225, 98, BOX_X, BOX_Y);
  minute2_finish = GRect(81, 98, BOX_X, BOX_Y);

  if (do_hour1 == true) {
    if (do_reverse == true) {
      s_hour1_animation = property_animation_create_layer_frame(s_hour1_parent, &hour1_finish, &hour1_start);
    }
    else {
      s_hour1_animation = property_animation_create_layer_frame(s_hour1_parent, &hour1_start, &hour1_finish);
    }
    animation_set_duration((Animation*)s_hour1_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_hour1_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_hour1_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_hour1_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_hour1_animation);
  }
  if (do_hour2 == true) {
    if (do_reverse == true) {
      s_hour2_animation = property_animation_create_layer_frame(s_hour2_parent, &hour2_finish, &hour2_start);
    }
    else {
      s_hour2_animation = property_animation_create_layer_frame(s_hour2_parent, &hour2_start, &hour2_finish);
    }
    animation_set_duration((Animation*)s_hour2_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_hour2_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_hour2_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_hour2_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_hour2_animation);
  }
  if (do_minute1 == true) {
    if (do_reverse == true) {
      s_minute1_animation = property_animation_create_layer_frame(s_minute1_parent, &minute1_finish, &minute1_start);
    }
    else {
      s_minute1_animation = property_animation_create_layer_frame(s_minute1_parent, &minute1_start, &minute1_finish);
    }
    animation_set_duration((Animation*)s_minute1_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_minute1_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_minute1_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_minute1_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_minute1_animation);
  }
  if (do_minute2 == true) {
    if (do_reverse == true) {
      s_minute2_animation = property_animation_create_layer_frame(s_minute2_parent, &minute2_finish, &minute2_start);
    }
    else {
      s_minute2_animation = property_animation_create_layer_frame(s_minute2_parent, &minute2_start, &minute2_finish);
    }
    animation_set_duration((Animation*)s_minute2_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_minute2_animation, ANIM_DELAY);
    animation_set_curve((Animation*)s_minute2_animation, AnimationCurveEaseInOut);
    animation_set_handlers((Animation*)s_minute2_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
    animation_schedule((Animation*)s_minute2_animation);
  }
  if (do_reverse == true) {
    again = true;
  }
}

static void update_bg(Layer *layer, GContext *ctx) {
  
  if (running == false) {
    gbitmap_destroy_safe(s_hour1_bitmap);
    gbitmap_destroy_safe(s_hour2_bitmap);
    gbitmap_destroy_safe(s_minute1_bitmap);
    gbitmap_destroy_safe(s_minute2_bitmap);
  
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);
  
    s_hour1_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[hour1]);
    s_hour2_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[hour2]);
    s_minute1_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[minute1]);
    s_minute2_bitmap = gbitmap_create_with_resource(TIME_RESOURCE_IDS[minute2]);
  
    bitmap_layer_set_bitmap(s_hour1_layer, s_hour1_bitmap);
    bitmap_layer_set_bitmap(s_hour2_layer, s_hour2_bitmap);
    bitmap_layer_set_bitmap(s_minute1_layer, s_minute1_bitmap);
    bitmap_layer_set_bitmap(s_minute2_layer, s_minute2_bitmap);
  }
}

static void update_date() {
  time_t epoch = time(NULL); 
  struct tm *t = localtime(&epoch);
  
  print_time(s_day_buffer, "%a", t);
  print_time(s_month_buffer, "%b", t);
  print_time(s_date_buffer, "%d", t); 
  
  upcase(s_day_buffer);
  upcase(s_month_buffer);
  upcase(s_date_buffer);
  
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_text(s_month_label, s_month_buffer);
  text_layer_set_text(s_date_label, s_date_buffer);
}

static void update_time() {
  time_t epoch = time(NULL); 
  struct tm *t = localtime(&epoch);
  
  if (clock_is_24h_style() == true) {
    hour = t->tm_hour;
  }
  else {
    hour = t->tm_hour % 12;
  }
  
  if (clock_is_24h_style() == false && (hour == 12 || hour == 0)) {
    hour1 = 1;
    hour2 = 2;
  }
  else {
    hour1 = hour / 10;
    hour2 = hour - (hour1 * 10);
  }
  minute1 = t->tm_min / 10;
  minute2 = t->tm_min - (minute1 * 10);
  
  if (first_run == 3) {
    do_minute2 = true;
    if (minute2 == 0) {
     do_minute1 = true;
     
     if (minute1 == 0) {
      do_hour2 = true;
       
       if (hour2 == 0) {
        do_hour1 = true;
       
        if (clock_is_24h_style() == false && hour2 == 1 && hour1 != 1) {
          do_hour1 = true;
        }
       }
     }
    }
    do_reverse = true;
    if (animations == true) {
      do_animation();
    }
    else {
      layer_mark_dirty(s_background_layer);
      layer_set_update_proc(s_background_layer, update_bg);
    }
  }
  else if (first_run < 3) {
    first_run += 1;
    layer_mark_dirty(s_background_layer);
    layer_set_update_proc(s_background_layer, update_bg);
  }
}

static void update_bat(Layer *layer, GContext *ctx) {
  bat = battery_state_service_peek().charge_percent / 10;
  int16_t Y_bat = bat;
  int16_t green_height;
  int16_t yellow_height;
  int16_t red_height;
  int16_t green_Y = 10;
  int16_t yellow_Y = 10;
  int16_t red_Y = 10;
  
  while (Y_bat < 10) {
    green_Y += 1;
    if (Y_bat < 4) {
      yellow_Y += 1;
    }
    Y_bat += 1;
  }
  
  green_height = bat * 7;
  green_Y = 28 + (7 * green_Y);
  
  if (bat < 4) {
    bat = bat + 6;
    yellow_height = (bat * 14) - 70;
    yellow_Y = 28 + (14 * yellow_Y) - 70;
  }
  else {
    yellow_height = 70;
    yellow_Y = 98;
  }
  
  red_height = 70;
  red_Y = 98;
  
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, GRect(68,0,8,red_height), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(68,red_Y,8,70), 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, GColorYellow);
  graphics_fill_rect(ctx, GRect(68,0,8,yellow_height), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(68,yellow_Y,8,70), 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, GColorGreen);
  graphics_fill_rect(ctx, GRect(68,0,8,green_height), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(68,green_Y,8,70), 0, GCornerNone);
}

static void update_bt(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(64,76,16,16), 2, GCornersAll);
  
  graphics_context_set_fill_color(ctx, GColorElectricUltramarine);
  graphics_fill_rect(ctx, GRect(65,77,14,14), 2, GCornersAll);
}

static void timer_callback(void *data) {
  animations = false;
}

static void timer_2_callback(void *data) {
  shake_reverse = true;
  shake_animation();
  sun_on = false;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  if (sun_on == false) {
    shake_reverse = false;
    shake_animation();
    sun_on = true;
  }
  animations = true;
  app_timer_cancel(timer);
  app_timer_cancel(timer_2);
  timer = app_timer_register(180 * 1000, timer_callback, NULL);
  timer_2 = app_timer_register(3 * 1000, timer_2_callback, NULL);
}

static void bt_handler(bool connected) {
  if (connected) {
    vibes_short_pulse();
    layer_show(s_bluetooth_layer);
  }
  else {
    vibes_double_pulse();
    layer_hide(s_bluetooth_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  GRect bounds = window_get_bounds(window);
  
  window_set_background_color(window, GColorDarkGray);
  
  s_info_font = fonts_load_resource_font(RESOURCE_ID_ROBOTO_CONDENSED_BOLD_17);
  
  s_background_layer = layer_create(bounds);
  s_info_layer = layer_create(GRect(0,0,144,0));
  s_date_1_layer = layer_create(GRect(0,0,0,168));
  s_date_2_layer = layer_create(GRect(144,0,0,168));
  s_lht_label = layer_create(GRect(0,73,144,0));
  s_bluetooth_layer = layer_create(bounds);
  s_battery_layer = layer_create(bounds);
  
  s_hour1_parent = layer_create(GRect(-144,0,BOX_X,BOX_Y));
  s_hour2_parent = layer_create(GRect(72,-168,BOX_X,BOX_Y));
  s_minute1_parent = layer_create(GRect(0,252,BOX_X,BOX_Y));
  s_minute2_parent = layer_create(GRect(216,84,BOX_X,BOX_Y));
  
  s_weather_label = text_layer_create(GRect(0,73,62,23));
  s_day_label = text_layer_create(GRect(0,73,62,23));
  s_month_label = text_layer_create(GRect(10,73,62,23));
  s_date_label = text_layer_create(GRect(10,73,62,23));
  s_low_label = text_layer_create(GRect(0,0,62,23));
  s_high_label = text_layer_create(GRect(82,0,62,23));
  
  s_hour1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_hour2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_minute1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_minute2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  
  bitmap_layer_set_compositing_mode(s_hour1_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_hour2_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_minute1_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_minute2_layer, GCompOpSet);
  
  text_layer_set_colors(s_weather_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_day_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_month_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_date_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_low_label, GColorWhite, GColorDarkGray);
  text_layer_set_colors(s_high_label, GColorWhite, GColorDarkGray);
  
  text_layer_set_text_alignment(s_weather_label, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_day_label, GTextAlignmentRight);
  text_layer_set_text_alignment(s_month_label, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_label, GTextAlignmentRight);
  text_layer_set_text_alignment(s_low_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_high_label, GTextAlignmentCenter);
  
  text_layer_set_text(s_weather_label, "...");
  text_layer_set_text(s_low_label, "LOW");
  text_layer_set_text(s_high_label, "HIGH");
  
  text_layer_set_font(s_weather_label, s_info_font);
  text_layer_set_font(s_day_label, s_info_font);
  text_layer_set_font(s_month_label, s_info_font);
  text_layer_set_font(s_date_label, s_info_font);
  text_layer_set_font(s_low_label, s_info_font);
  text_layer_set_font(s_high_label, s_info_font);
  
  layer_set_update_proc(s_bluetooth_layer, update_bt);
  layer_set_update_proc(s_battery_layer, update_bat);
  
  layer_add_to_window(s_background_layer, window);
  layer_add_to_window(s_info_layer, window);
  layer_add_to_window(s_date_1_layer, window);
  layer_add_to_window(s_date_2_layer, window);
  layer_add_to_window(s_lht_label, window);
  
  layer_add_to_layer(s_bluetooth_layer, s_info_layer);
  layer_add_to_layer(s_battery_layer, s_info_layer);
  
  layer_add_to_layer(s_hour1_parent, s_background_layer);
  layer_add_to_layer(s_hour2_parent, s_background_layer);
  layer_add_to_layer(s_minute1_parent, s_background_layer);
  layer_add_to_layer(s_minute2_parent, s_background_layer);
  
  bitmap_layer_add_to_layer(s_hour1_layer, s_hour1_parent);
  bitmap_layer_add_to_layer(s_hour2_layer, s_hour2_parent);
  bitmap_layer_add_to_layer(s_minute1_layer, s_minute1_parent);
  bitmap_layer_add_to_layer(s_minute2_layer, s_minute2_parent);
  
  text_layer_add_to_layer(s_weather_label, s_date_1_layer);
  text_layer_add_to_layer(s_day_label, s_date_1_layer);
  text_layer_add_to_layer(s_month_label, s_date_2_layer);
  text_layer_add_to_layer(s_date_label, s_date_2_layer);
  text_layer_add_to_layer(s_low_label, s_lht_label);
  text_layer_add_to_layer(s_high_label, s_lht_label);
  
  bt_connected = bluetooth_connection_service_peek();
  if (bt_connected) {
    layer_show(s_bluetooth_layer);
  }
  else {
    layer_hide(s_bluetooth_layer);
  }
  
  update_time();
  update_date();
}

static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_info_font);
  
  layer_destroy_safe(s_background_layer);
  layer_destroy_safe(s_info_layer);
  layer_destroy_safe(s_date_1_layer);
  layer_destroy_safe(s_date_2_layer);
  layer_destroy_safe(s_lht_label);
  layer_destroy_safe(s_bluetooth_layer);
  layer_destroy_safe(s_battery_layer);
  
  layer_destroy_safe(s_hour1_parent);
  layer_destroy_safe(s_hour2_parent);
  layer_destroy_safe(s_minute1_parent);
  layer_destroy_safe(s_minute2_parent);
  
  bitmap_layer_destroy_safe(s_hour1_layer);
  bitmap_layer_destroy_safe(s_hour2_layer);
  bitmap_layer_destroy_safe(s_minute1_layer);
  bitmap_layer_destroy_safe(s_minute2_layer);
  
  text_layer_destroy_safe(s_weather_label);
  text_layer_destroy_safe(s_day_label);
  text_layer_destroy_safe(s_month_label);
  text_layer_destroy_safe(s_date_label);
  text_layer_destroy_safe(s_low_label);
  text_layer_destroy_safe(s_high_label);
  
  gbitmap_destroy_safe(s_hour1_bitmap);
  gbitmap_destroy_safe(s_hour2_bitmap);
  gbitmap_destroy_safe(s_minute1_bitmap);
  gbitmap_destroy_safe(s_minute2_bitmap);
}

static void init() {
  s_main_window = window_create();
  window_handlers(s_main_window, main_window_load, main_window_unload);
  window_stack_push(s_main_window, true);
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  
  timer = app_timer_register(180 * 1000, timer_callback, NULL);
  
  info_animation();
  do_hour1 = true;
  do_hour2 = true;
  do_minute1 = true;
  do_minute2 = true;
  do_animation();
  do_hour1 = false;
  do_hour2 = false;
  do_minute1 = false;
  do_minute2 = false;
}

static void deinit() {
  animation_unschedule_all();
  accel_tap_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy_safe(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}