/*
Blocked Out
    Copyright (C) 2015 James Downs
    james.j.downs@icloud.com
*/

#define COUNTRY 0
#define TEMPERATURE 1

#define BOX_X 63
#define BOX_Y 70

#define ANIM_DURATION 500
#define ANIM_DELAY 300

static Window *s_main_window;
static Layer *s_background_layer, *s_bluetooth_layer, *s_battery_layer;
static TextLayer *s_weather_label, *s_day_label, *s_month_label, *s_date_label;
static Layer *s_hour1_parent, *s_hour2_parent, *s_minute1_parent, *s_minute2_parent;
static BitmapLayer *s_hour1_layer, *s_hour2_layer, *s_minute1_layer, *s_minute2_layer;
static GBitmap *s_hour1_bitmap, *s_hour2_bitmap, *s_minute1_bitmap, *s_minute2_bitmap; 
static PropertyAnimation *s_hour1_animation, *s_hour2_animation, *s_minute1_animation, *s_minute2_animation;

static GFont s_info_font;

static AppTimer *timer = NULL;

static int8_t hour;
static int8_t hour1;
static int8_t hour2;
static int8_t minute1;
static int8_t minute2;

static int8_t country;
static int16_t degree;

static int16_t bat;

static int8_t first_run = 1;
  
static bool do_hour1;
static bool do_hour2;
static bool do_minute1;
static bool do_minute2;
static bool do_reverse;
static bool again = false;
static bool running = false;
static bool animations = true;

static bool bt_connected;

static char s_weather_buffer[] = "XXXX";
static char s_day_buffer[] = "XXX";
static char s_month_buffer[] = "XXX";
static char s_date_buffer[] = "XX";

const int TIME_RESOURCE_IDS[] = {
  RESOURCE_ID_ZERO,
  RESOURCE_ID_ONE,
  RESOURCE_ID_TWO,
  RESOURCE_ID_THREE,
  RESOURCE_ID_FOUR,
  RESOURCE_ID_FIVE,
  RESOURCE_ID_SIX,
  RESOURCE_ID_SEVEN,
  RESOURCE_ID_EIGHT,
  RESOURCE_ID_NINE
};

static void do_animation();
static void update_bg(Layer *layer, GContext *ctx);