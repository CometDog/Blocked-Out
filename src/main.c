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
#ifdef PBL_COLOR
  #include "gcolor_definitions.h"
#endif
#include "libs/pebble-assist.h"
#include "elements.h"

/********************************************
            Additional Functions
********************************************/

static char *upcase(char *str)
{
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 0x20;
        }
    }
    return str;
}

static void conditions_select(int16_t cond) {
  time_t epoch = time(NULL); 
  struct tm *t = localtime(&epoch);
  
  if (t->tm_hour >= 20 || t->tm_hour <= 6) {
    night = true;
  }
  int16_t conditions = cond;
  
  if (conditions >= 200 && conditions <= 232) {
    conditions = 9;
  }
  else if (conditions >= 300 && conditions <= 321) {
    conditions = 7;
  }
  else if (conditions >= 500 && conditions <= 622) {
    conditions = 8;
  }
  else if (conditions >= 701 && conditions <= 781) {
    conditions = 10;
  }
  else if (conditions == 800) {
    conditions = 0;
    if (night == true) {
      conditions += 3;
    }
  }
  else if (conditions >= 801 && conditions <= 802) {
    conditions = 2;
    if (night == true) {
      conditions += 3;
    }
  }
  else if (conditions >= 803 && conditions <= 804) {
    conditions = 6;
  }
  else {
    conditions = 10;
  }
  
  if (i == 0) {
    condcon1 = conditions;
  }
  else if (i == 1) {
    condcon2 = conditions;
  }
  else if (i == 2) {
    condcon3 = conditions;
  }
  else if (i == 3) {
    condcon4 = conditions;
  }
  i += 1 % 3;
}

static void weather_conditions() {
  
  conditions_select(conditions1);
  conditions_select(conditions2);
  conditions_select(conditions3);
  conditions_select(conditions4);

  gbitmap_destroy_safe(s_weather1_bitmap);
  gbitmap_destroy_safe(s_weather2_bitmap);
  gbitmap_destroy_safe(s_weather3_bitmap);
  gbitmap_destroy_safe(s_weather4_bitmap);
  
  s_weather1_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[condcon1]);
  s_weather2_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[condcon2]);
  s_weather3_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[condcon3]);
  s_weather4_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[condcon4]);
  
  bitmap_layer_set_bitmap(s_weather1_layer, s_weather1_bitmap);
  bitmap_layer_set_bitmap(s_weather2_layer, s_weather2_bitmap);
  bitmap_layer_set_bitmap(s_weather3_layer, s_weather3_bitmap);
  bitmap_layer_set_bitmap(s_weather4_layer, s_weather4_bitmap);
}

/********************************************
          Receive Data From Phone
********************************************/

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);

  while(t != NULL) {
    switch(t->key) {
    case COUNTRY:
      country = (int)t->value->int8;
      break;
    case TEMPERATURE_1:
      degree1 = (int)t->value->int16;
      break;
    case TEMPERATURE_2:
      degree2 = (int)t->value->int16;
      break;
    case TEMPERATURE_3:
      degree3 = (int)t->value->int16;
      break;
    case TEMPERATURE_4:
      degree4 = (int)t->value->int16;
      break;
    case CONDITIONS_1:
      conditions1 = (int)t->value->int16;
      break;
    case CONDITIONS_2:
      conditions2 = (int)t->value->int16;
      break;
    case CONDITIONS_3:
      conditions3 = (int)t->value->int16;
      break;
    case CONDITIONS_4:
      conditions4 = (int)t->value->int16;
      break;
    case HUMIDITY:
      humidity = (int)t->value->int16;
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "] %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
  
  weather_conditions();
  
  if (country == 1) {
    degree1 = (1.8 * degree1) + 42;
    degree2 = (1.8 * degree2) + 42;
    degree3 = (1.8 * degree3) + 42;
    degree4 = (1.8 * degree4) + 42;
  }
  
  if (degree1 < 100) {
    print_int(s_weather1_buffer, "%d°", degree1);
    print_int(s_weather2_buffer, "%d°", degree2);
    print_int(s_weather3_buffer, "%d°", degree3);
    print_int(s_weather4_buffer, "%d°", degree4);
  }
  else {
    print_int(s_weather1_buffer, "%d", degree1);
    print_int(s_weather2_buffer, "%d", degree2);
    print_int(s_weather3_buffer, "%d", degree3);
    print_int(s_weather4_buffer, "%d", degree4);
  }
  
  print_int(s_humidity_buffer, "H:%d%%", humidity);

  text_layer_set_text(s_weather_label, s_weather1_buffer);
  text_layer_set_text(s_weather1_label, s_weather1_buffer);
  text_layer_set_text(s_weather2_label, s_weather2_buffer);
  text_layer_set_text(s_weather3_label, s_weather3_buffer);
  text_layer_set_text(s_weather4_label, s_weather4_buffer);
  
  text_layer_set_text(s_conditions_label, "+12 HRS");
  text_layer_set_text(s_humidity_label, s_humidity_buffer);
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

/********************************************
                Animations
********************************************/

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
  
  s_weather1_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[conditions1]);
  s_weather2_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[conditions2]);
  s_weather3_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[conditions3]);
  s_weather4_bitmap = gbitmap_create_with_resource(WEATHER_RESOURCE_IDS[conditions4]);
  
  GRect shake_start, shake_finish;
  
  shake_start = GRect(0,73,144,0);
  shake_finish = GRect(0,73,144,28);
  
  if (shake_reverse != true) {
    s_shake_animation = property_animation_create_layer_frame(s_ch_layer, &shake_start, &shake_finish);
  }
  else {
    s_shake_animation = property_animation_create_layer_frame(s_ch_layer, &shake_finish, &shake_start);
  }
  animation_set_duration((Animation*)s_shake_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_shake_animation, ANIM_DELAY_SUN);
  animation_set_curve((Animation*)s_shake_animation, AnimationCurveLinear);
  animation_schedule((Animation*)s_shake_animation);
  
  GRect weather1_start, weather1_finish;
  GRect weather2_start, weather2_finish;
  GRect weather3_start, weather3_finish;
  GRect weather4_start, weather4_finish;
  
  weather1_start = GRect(-0, -168, BOX_X, BOX_Y);
  weather1_finish = GRect(0, 0, BOX_X, BOX_Y);
  
  weather2_start = GRect(82, -168, BOX_X, BOX_Y);
  weather2_finish = GRect(82, 0, BOX_X, BOX_Y);

  weather3_start = GRect(0, 266, BOX_X, BOX_Y);
  weather3_finish = GRect(0, 98, BOX_X, BOX_Y);

  weather4_start = GRect(82, 266, BOX_X, BOX_Y);
  weather4_finish = GRect(82, 98, BOX_X, BOX_Y);

    if (shake_reverse == true) {
      s_weather1_animation = property_animation_create_layer_frame(s_weather1_parent, &weather1_finish, &weather1_start);
    }
    else {
      s_weather1_animation = property_animation_create_layer_frame(s_weather1_parent, &weather1_start, &weather1_finish);
    }
    animation_set_duration((Animation*)s_weather1_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_weather1_animation, ANIM_DELAY_SUN);
    animation_set_curve((Animation*)s_weather1_animation, AnimationCurveEaseInOut);
    animation_schedule((Animation*)s_weather1_animation);
  
    if (shake_reverse == true) {
      s_weather2_animation = property_animation_create_layer_frame(s_weather2_parent, &weather2_finish, &weather2_start);
    }
    else {
      s_weather2_animation = property_animation_create_layer_frame(s_weather2_parent, &weather2_start, &weather2_finish);
    }
    animation_set_duration((Animation*)s_weather2_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_weather2_animation, ANIM_DELAY_SUN);
    animation_set_curve((Animation*)s_weather2_animation, AnimationCurveEaseInOut);
    animation_schedule((Animation*)s_weather2_animation);

    if (shake_reverse == true) {
      s_weather3_animation = property_animation_create_layer_frame(s_weather3_parent, &weather3_finish, &weather3_start);
    }
    else {
      s_weather3_animation = property_animation_create_layer_frame(s_weather3_parent, &weather3_start, &weather3_finish);
    }
    animation_set_duration((Animation*)s_weather3_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_weather3_animation, ANIM_DELAY_SUN);
    animation_set_curve((Animation*)s_weather3_animation, AnimationCurveEaseInOut);
    animation_schedule((Animation*)s_weather3_animation);
    
    if (shake_reverse == true) {
      s_weather4_animation = property_animation_create_layer_frame(s_weather4_parent, &weather4_finish, &weather4_start);
    }
    else {
      s_weather4_animation = property_animation_create_layer_frame(s_weather4_parent, &weather4_start, &weather4_finish);
    }
    animation_set_duration((Animation*)s_weather4_animation, ANIM_DURATION);
    animation_set_delay((Animation*)s_weather4_animation, ANIM_DELAY_SUN);
    animation_set_curve((Animation*)s_weather4_animation, AnimationCurveEaseInOut);
    animation_schedule((Animation*)s_weather4_animation);
    
  gbitmap_destroy_safe(s_weather1_bitmap);
  gbitmap_destroy_safe(s_weather2_bitmap);
  gbitmap_destroy_safe(s_weather3_bitmap);
  gbitmap_destroy_safe(s_weather4_bitmap);
}

static void info_animation() {
  GRect info_start, info_finish;
  GRect date_1_start, date_1_finish;
  GRect date_2_start, date_2_finish;
  
  info_start = GRect(0,0,144,0);
  info_finish = GRect(0,0,144,168);
  
  date_1_start = GRect(-72,0,72,168);
  date_1_finish = GRect(0,0,72,168);
  
  date_2_start = GRect(144,0,72,168);
  date_2_finish = GRect(72,0,72,168);
  
  s_info_animation = property_animation_create_layer_frame(s_info_layer, &info_start, &info_finish);
  animation_set_duration((Animation*)s_info_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_info_animation, ANIM_DELAY_INFO);
  animation_set_curve((Animation*)s_info_animation, AnimationCurveLinear);
  animation_schedule((Animation*)s_info_animation);
  
  s_date_1_animation = property_animation_create_layer_frame(s_date_1_layer, &date_1_start, &date_1_finish);
  animation_set_duration((Animation*)s_date_1_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_date_1_animation, ANIM_DELAY_INFO);
  animation_set_curve((Animation*)s_date_1_animation, AnimationCurveEaseInOut);
  animation_schedule((Animation*)s_date_1_animation);
  
  s_date_2_animation = property_animation_create_layer_frame(s_date_2_layer, &date_2_start, &date_2_finish);
  animation_set_duration((Animation*)s_date_2_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_date_2_animation, ANIM_DELAY_INFO);
  animation_set_curve((Animation*)s_date_2_animation, AnimationCurveEaseInOut);
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
  
  hour1_start = GRect(-0, -168, BOX_X, BOX_Y);
  hour1_finish = GRect(0, 0, BOX_X, BOX_Y);
  
  hour2_start = GRect(82, -168, BOX_X, BOX_Y);
  hour2_finish = GRect(82, 0, BOX_X, BOX_Y);

  minute1_start = GRect(0, 266, BOX_X, BOX_Y);
  minute1_finish = GRect(0, 98, BOX_X, BOX_Y);

  minute2_start = GRect(82, 266, BOX_X, BOX_Y);
  minute2_finish = GRect(82, 98, BOX_X, BOX_Y);

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

/********************************************
         Update Screen Information
********************************************/

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
  
  if (bat == 0) {
    bat = 1;
  }
  
  int8_t Y_bat = bat % 2;
  

  int16_t bat_height;
  int16_t bat_height_2;
  if (bat == 0) {
    bat_height = (bat / 2) * 14;
  }
  else {
    bat_height = ((bat + 1) / 2) * 14;
  }
  bat_height_2 = 70 - bat_height;
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(71,98,3,bat_height), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(71,0,3,70), 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(71,0,3,bat_height_2), 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(65,77,14,14), 2, GCornersAll);
  #ifdef PBL_COLOR
    if (bat <= 1) {
      graphics_context_set_fill_color(ctx, GColorRed);
      graphics_fill_rect(ctx, GRect(66,78,12,12), 2, GCornersAll);
    }
    else {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, GRect(66,78,12,12), 2, GCornersAll);
    }
  #else
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(66,78,12,12), 2, GCornersAll);
  #endif
}

static void update_bt(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(65,77,14,14), 2, GCornersAll);
  
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorElectricUltramarine);
    graphics_fill_rect(ctx, GRect(66,78,12,12), 2, GCornersAll);
  #endif
}

/********************************************
            Function Handlers
********************************************/

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
  timer_2 = app_timer_register(2 * 1000, timer_2_callback, NULL);
}

static void bt_handler(bool connected) {
  if (connected) {
    vibes_short_pulse();
    layer_hide(s_bluetooth_layer);
  }
  else {
    vibes_double_pulse();
    layer_show(s_bluetooth_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

/********************************************
             Window Load/Unload
********************************************/

static void main_window_load(Window *window) {
  GRect bounds = window_get_bounds(window);
  
  window_set_background_color(window, GColorBlack);
  
  s_info_font = fonts_load_resource_font(RESOURCE_ID_ROBOTO_CONDENSED_BOLD_17);
  
  s_background_layer = layer_create(bounds);
  s_info_layer = layer_create(GRect(0,0,144,0));
  s_date_1_layer = layer_create(GRect(0,0,0,168));
  s_date_2_layer = layer_create(GRect(144,0,0,168));
  s_ch_layer = layer_create(GRect(0,73,144,0));
  s_bluetooth_layer = layer_create(bounds);
  s_battery_layer = layer_create(bounds);
  
  s_hour1_parent = layer_create(GRect(0,-168,BOX_X,BOX_Y));
  s_hour2_parent = layer_create(GRect(72,-168,BOX_X,BOX_Y));
  s_minute1_parent = layer_create(GRect(0,252,BOX_X,BOX_Y));
  s_minute2_parent = layer_create(GRect(72,252,BOX_X,BOX_Y));
  
  s_weather1_parent = layer_create(GRect(0,-168,BOX_X,BOX_Y));
  s_weather2_parent = layer_create(GRect(72,-168,BOX_X,BOX_Y));
  s_weather3_parent = layer_create(GRect(0,252,BOX_X,BOX_Y));
  s_weather4_parent = layer_create(GRect(72,252,BOX_X,BOX_Y));
  
  s_weather_label = text_layer_create(GRect(0,73,BOX_X,23));
  s_weather1_label = text_layer_create(GRect(0,43,BOX_X,23));
  s_weather2_label = text_layer_create(GRect(0,43,BOX_X,23));
  s_weather3_label = text_layer_create(GRect(0,43,BOX_X,23));
  s_weather4_label = text_layer_create(GRect(0,43,BOX_X,23));
  
  s_day_label = text_layer_create(GRect(0,73,62,23));
  s_month_label = text_layer_create(GRect(10,73,62,23));
  s_date_label = text_layer_create(GRect(10,73,62,23));
  s_conditions_label = text_layer_create(GRect(82,0,62,23));
  s_humidity_label = text_layer_create(GRect(0,0,62,23));
  
  s_hour1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_hour2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_minute1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_minute2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  
  s_weather1_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_weather2_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_weather3_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  s_weather4_layer = bitmap_layer_create(GRect(0,0,BOX_X,BOX_Y));
  
  #ifdef PBL_COLOR
    bitmap_layer_set_compositing_mode(s_hour1_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(s_hour2_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(s_minute1_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(s_minute2_layer, GCompOpSet);
    
    bitmap_layer_set_compositing_mode(s_weather1_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(s_weather2_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(s_weather3_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(s_weather4_layer, GCompOpSet);
  #endif
  
  text_layer_set_colors(s_weather_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_weather1_label, GColorBlack, GColorClear);
  text_layer_set_colors(s_weather2_label, GColorBlack, GColorClear);
  text_layer_set_colors(s_weather3_label, GColorBlack, GColorClear);
  text_layer_set_colors(s_weather4_label, GColorBlack, GColorClear);
  text_layer_set_colors(s_day_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_month_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_date_label, GColorWhite, GColorClear);
  text_layer_set_colors(s_conditions_label, GColorWhite, GColorBlack);
  text_layer_set_colors(s_humidity_label, GColorWhite, GColorBlack);

  text_layer_set_text_alignment(s_weather_label, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_weather1_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_weather2_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_weather3_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_weather4_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_day_label, GTextAlignmentRight);
  text_layer_set_text_alignment(s_month_label, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_label, GTextAlignmentRight);
  text_layer_set_text_alignment(s_conditions_label, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_humidity_label, GTextAlignmentCenter);
  
  text_layer_set_text(s_weather_label, "...");
  text_layer_set_text(s_weather1_label, "...");
  text_layer_set_text(s_weather2_label, "...");
  text_layer_set_text(s_weather3_label, "...");
  text_layer_set_text(s_weather4_label, "...");
  text_layer_set_text(s_conditions_label, "COND");
  text_layer_set_text(s_humidity_label, "HUMID");
  
  text_layer_set_font(s_weather_label, s_info_font);
  text_layer_set_font(s_weather1_label, s_info_font);
  text_layer_set_font(s_weather2_label, s_info_font);
  text_layer_set_font(s_weather3_label, s_info_font);
  text_layer_set_font(s_weather4_label, s_info_font);
  text_layer_set_font(s_day_label, s_info_font);
  text_layer_set_font(s_month_label, s_info_font);
  text_layer_set_font(s_date_label, s_info_font);
  text_layer_set_font(s_conditions_label, s_info_font);
  text_layer_set_font(s_humidity_label, s_info_font);
  
  layer_set_update_proc(s_bluetooth_layer, update_bt);
  layer_set_update_proc(s_battery_layer, update_bat);
  
  layer_add_to_window(s_background_layer, window);
  layer_add_to_window(s_info_layer, window);
  layer_add_to_window(s_date_1_layer, window);
  layer_add_to_window(s_date_2_layer, window);
  layer_add_to_window(s_ch_layer, window);
  
  layer_add_to_layer(s_bluetooth_layer, s_info_layer);
  layer_add_to_layer(s_battery_layer, s_info_layer);
  
  layer_add_to_layer(s_hour1_parent, s_background_layer);
  layer_add_to_layer(s_hour2_parent, s_background_layer);
  layer_add_to_layer(s_minute1_parent, s_background_layer);
  layer_add_to_layer(s_minute2_parent, s_background_layer);
  
  layer_add_to_layer(s_weather1_parent, s_background_layer);
  layer_add_to_layer(s_weather2_parent, s_background_layer);
  layer_add_to_layer(s_weather3_parent, s_background_layer);
  layer_add_to_layer(s_weather4_parent, s_background_layer);
  
  bitmap_layer_add_to_layer(s_hour1_layer, s_hour1_parent);
  bitmap_layer_add_to_layer(s_hour2_layer, s_hour2_parent);
  bitmap_layer_add_to_layer(s_minute1_layer, s_minute1_parent);
  bitmap_layer_add_to_layer(s_minute2_layer, s_minute2_parent);
  
  bitmap_layer_add_to_layer(s_weather1_layer, s_weather1_parent);
  bitmap_layer_add_to_layer(s_weather2_layer, s_weather2_parent);
  bitmap_layer_add_to_layer(s_weather3_layer, s_weather3_parent);
  bitmap_layer_add_to_layer(s_weather4_layer, s_weather4_parent);
  
  text_layer_add_to_layer(s_weather_label, s_date_1_layer);
  text_layer_add_to_layer(s_weather1_label, s_weather1_parent);
  text_layer_add_to_layer(s_weather2_label, s_weather2_parent);
  text_layer_add_to_layer(s_weather3_label, s_weather3_parent);
  text_layer_add_to_layer(s_weather4_label, s_weather4_parent);
  text_layer_add_to_layer(s_day_label, s_date_1_layer);
  text_layer_add_to_layer(s_month_label, s_date_2_layer);
  text_layer_add_to_layer(s_date_label, s_date_2_layer);
  text_layer_add_to_layer(s_conditions_label, s_ch_layer);
  text_layer_add_to_layer(s_humidity_label, s_ch_layer);
  
  bt_connected = bluetooth_connection_service_peek();
  if (bt_connected) {
    layer_hide(s_bluetooth_layer);
  }
  else {
    layer_show(s_bluetooth_layer);
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
  layer_destroy_safe(s_ch_layer);
  layer_destroy_safe(s_bluetooth_layer);
  layer_destroy_safe(s_battery_layer);
  
  layer_destroy_safe(s_hour1_parent);
  layer_destroy_safe(s_hour2_parent);
  layer_destroy_safe(s_minute1_parent);
  layer_destroy_safe(s_minute2_parent);
  
  layer_destroy_safe(s_weather1_parent);
  layer_destroy_safe(s_weather2_parent);
  layer_destroy_safe(s_weather3_parent);
  layer_destroy_safe(s_weather4_parent);
  
  bitmap_layer_destroy_safe(s_hour1_layer);
  bitmap_layer_destroy_safe(s_hour2_layer);
  bitmap_layer_destroy_safe(s_minute1_layer);
  bitmap_layer_destroy_safe(s_minute2_layer);
  
  bitmap_layer_destroy_safe(s_weather1_layer);
  bitmap_layer_destroy_safe(s_weather2_layer);
  bitmap_layer_destroy_safe(s_weather3_layer);
  bitmap_layer_destroy_safe(s_weather4_layer);
  
  text_layer_destroy_safe(s_weather_label);
  text_layer_destroy_safe(s_weather1_label);
  text_layer_destroy_safe(s_weather2_label);
  text_layer_destroy_safe(s_weather3_label);
  text_layer_destroy_safe(s_weather4_label);
  text_layer_destroy_safe(s_day_label);
  text_layer_destroy_safe(s_month_label);
  text_layer_destroy_safe(s_date_label);
  text_layer_destroy_safe(s_conditions_label);
  text_layer_destroy_safe(s_humidity_label);
  
  gbitmap_destroy_safe(s_hour1_bitmap);
  gbitmap_destroy_safe(s_hour2_bitmap);
  gbitmap_destroy_safe(s_minute1_bitmap);
  gbitmap_destroy_safe(s_minute2_bitmap);
  
  gbitmap_destroy_safe(s_weather1_bitmap);
  gbitmap_destroy_safe(s_weather2_bitmap);
  gbitmap_destroy_safe(s_weather3_bitmap);
  gbitmap_destroy_safe(s_weather4_bitmap);
}

/********************************************
               (De)Initialize
********************************************/

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