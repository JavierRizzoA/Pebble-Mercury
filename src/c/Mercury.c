#include <pebble.h>
#include "Mercury.h"
#include "utils/MathUtils.h"
#include "utils/BinaryImageMaskData.h"

static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static GRect bounds;

static struct tm *prv_tick_time;

static struct BinaryImageMaskData *dial = NULL;
static struct BinaryImageMaskData *digits = NULL;
static struct BinaryImageMaskData *digits_big = NULL;
static int current_date = -1;
static int current_day = -1;
static ClaySettings settings;
static DialSpec *ds = NULL;
static int offset_y = 0;

//#define LOG
//#define LOGTIME
//#define LOGPERF
//#define QUICKTEST

//#define HOUR 22
//#define MINUTE 6
//#define SECOND 30
//#define DAY 6
//#define DATE 6

//#define DIGITAL true
//#define SECONDS true
//#define FONT 1
//#define BGC1 GColorCobaltBlue
//#define BGC2 GColorPastelYellow
//#define TC1 GColorWhite
//#define TC2 GColorOrange
//#define HHC GColorWhite
//#define HHBC GColorBlack
//#define MHC GColorWhite
//#define MHBC GColorBlack
//#define SHC GColorOrange
//#define BWBGC1 GColorWhite
//#define BWBGC2 GColorBlack
//#define BWTC1 GColorBlack
//#define BWTC2 GColorWhite

static void prv_save_settings(void);
static void prv_default_settings(void);
static void prv_load_settings(void);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void update_date();
static void update_moonphase();
static int get_moonphase_index();
static void update_digital_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void draw_fancy_hand(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color);
static void draw_line_hand_with_border(GContext *ctx, int angle, int length, int back_length, GColor color, GColor border_color);
static void draw_center(GContext *ctx, GColor minutes_border, GColor minutes_color, GColor seconds_color);
static void canvas_update_proc(Layer *layer, GContext *ctx);
static void bg_update_proc(Layer *layer, GContext *ctx);
static void draw_dial();
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);
static int get_font();
static bool is_digital();
static bool is_round();
static bool is_large_screen();
static bool byte_get_bit(uint8_t *byte, uint8_t bit);
static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value);
static void set_pixel_color(GBitmapDataRowInfo info, GPoint point, GColor color);
static void set_marker_positions(DialSpec* ds);
static void unobstructed_change_handler(AnimationProgress progress, void *context);
static GColor flip_color_bw(GColor color);
static GColor handle_background_pattern(int x, int y, bool is_in_bg1);

static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_default_settings(void) {
  settings.EnableDate = true;
  settings.EnablePebbleLogo = true;
  settings.EnableWatchModel = true;
  settings.ForcedWatchModel = -1;
  settings.EnableMoonphase = true;
#ifdef SECONDS
  settings.EnableSecondsHand = SECONDS;
#else
  settings.EnableSecondsHand = true;
#endif
#ifdef DIGITAL
  settings.DigitalWatch = DIGITAL;
#else
  settings.DigitalWatch = false;
#endif
#ifdef BGC1
  settings.BackgroundColor1 = BGC1;
#else
  settings.BackgroundColor1 = GColorCobaltBlue;
#endif
#ifdef BGC2
  settings.BackgroundColor2 = BGC2;
#else
  settings.BackgroundColor2 = GColorPastelYellow;
#endif
#ifdef TC1
  settings.TextColor1 = TC1;
#else
  settings.TextColor1 = GColorWhite;
#endif
#ifdef TC2
  settings.TextColor2 = TC2;
#else
  settings.TextColor2 = GColorOrange;
#endif
#ifdef HHC
  settings.HoursHandColor = HHC;
#else
  settings.HoursHandColor = GColorWhite;
#endif
#ifdef HHBC
  settings.HoursHandBorderColor = HHBC;
#else
  settings.HoursHandBorderColor = GColorBlack;
#endif
#ifdef MHC
  settings.MinutesHandColor = MHC;
#else
  settings.MinutesHandColor = GColorWhite;
#endif
#ifdef MHBC
  settings.MinutesHandBorderColor = MHBC;
#else
  settings.MinutesHandBorderColor = GColorBlack;
#endif
#ifdef SHC
  settings.SecondsHandColor = SHC;
#else
  settings.SecondsHandColor = GColorOrange;
#endif
#ifdef BWBGC1
  settings.BWBackgroundColor1 = BWBGC1;
#else
  settings.BWBackgroundColor1 = GColorWhite;
#endif
#ifdef BWBGC2
  settings.BWBackgroundColor2 = BWBGC2;
#else
  settings.BWBackgroundColor2 = GColorBlack;
#endif
#ifdef BWTC1
  settings.BWTextColor1 = BWTC1;
#else
  settings.BWTextColor1 = GColorBlack;
#endif
#ifdef BWTC2
  settings.BWTextColor2 = BWTC2;
#else
  settings.BWTextColor2 = GColorWhite;
#endif
#ifdef BWHHC
  settings.BWHoursHandColor = BWHHC;
#else
  settings.BWHoursHandColor = GColorWhite;
#endif
#ifdef BWHHBC
  settings.BWHoursHandBorderColor = BWHHBC;
#else
  settings.BWHoursHandBorderColor = GColorBlack;
#endif
#ifdef BWMHC
  settings.BWMinutesHandColor = BWMHC;
#else
  settings.BWMinutesHandColor = GColorWhite;
#endif
#ifdef BWMHBC
  settings.BWMinutesHandBorderColor = BWMHBC;
#else
  settings.BWMinutesHandBorderColor = GColorBlack;
#endif
#ifdef BWSHC
  settings.BWSecondsHandColor = BWSHC;
#else
  settings.BWSecondsHandColor = GColorWhite;
#endif
#ifdef BWSHBC
  settings.BWSecondsHandBorderColor = BWSHBC;
#else
  settings.BWSecondsHandBorderColor = GColorBlack;
#endif
#ifdef FONT
  settings.Font = FONT;
#else
  settings.Font = 1;
#endif
  settings.FixedAngle = false;
  settings.Angle = 40;

#ifdef BP1
  settings.BackgroundPattern1 = BP1;
#else
  settings.BackgroundPattern1 = SOLID;
#endif
#ifdef BP2
  settings.BackgroundPattern2 = BP2;
#else
  settings.BackgroundPattern2 = SOLID;
#endif
#ifdef SBGC1
  settings.SecondaryBackgroundColor1 = SBGC1;
#else
  settings.SecondaryBackgroundColor1 = GColorWhite;
#endif
#ifdef SBGC2
  settings.SecondaryBackgroundColor2 = SBGC2;
#else
  settings.SecondaryBackgroundColor2 = GColorOrange;
#endif
#ifdef NPUT
  settings.NoPatternUnderText = NPUT;
#else
  settings.NoPatternUnderText = false;
#endif
}

static void prv_load_settings(void) {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  Tuple *enable_seconds_t = dict_find(iter, MESSAGE_KEY_EnableSecondsHand);
  Tuple *enable_date_t = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_pebble_logo_t = dict_find(iter, MESSAGE_KEY_EnablePebbleLogo);
  Tuple *enable_watch_model_t = dict_find(iter, MESSAGE_KEY_EnableWatchModel);
  Tuple *forced_watch_model_t = dict_find(iter, MESSAGE_KEY_ForcedWatchModel);
  Tuple *enable_moonphase_t = dict_find(iter, MESSAGE_KEY_EnableMoonphase);
  Tuple *digital_watch_t = dict_find(iter, MESSAGE_KEY_DigitalWatch);
  Tuple *bg_color1_t = dict_find(iter, MESSAGE_KEY_BackgroundColor1);
  Tuple *bg_color2_t = dict_find(iter, MESSAGE_KEY_BackgroundColor2);
  Tuple *text_color1_t = dict_find(iter, MESSAGE_KEY_TextColor1);
  Tuple *text_color2_t = dict_find(iter, MESSAGE_KEY_TextColor2);
  Tuple *hours_color_t = dict_find(iter, MESSAGE_KEY_HoursHandColor);
  Tuple *hours_border_t = dict_find(iter, MESSAGE_KEY_HoursHandBorderColor);
  Tuple *minutes_color_t = dict_find(iter, MESSAGE_KEY_MinutesHandColor);
  Tuple *minutes_border_t = dict_find(iter, MESSAGE_KEY_MinutesHandBorderColor);
  Tuple *seconds_color_t = dict_find(iter, MESSAGE_KEY_SecondsHandColor);
  Tuple *bwbg_color1_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor1);
  Tuple *bwbg_color2_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor2);
  Tuple *bwtext_color1_t = dict_find(iter, MESSAGE_KEY_BWTextColor1);
  Tuple *bwtext_color2_t = dict_find(iter, MESSAGE_KEY_BWTextColor2);
  Tuple *bwhours_color_t = dict_find(iter, MESSAGE_KEY_BWHoursHandColor);
  Tuple *bwhours_border_t = dict_find(iter, MESSAGE_KEY_BWHoursHandBorderColor);
  Tuple *bwminutes_color_t = dict_find(iter, MESSAGE_KEY_BWMinutesHandColor);
  Tuple *bwminutes_border_t = dict_find(iter, MESSAGE_KEY_BWMinutesHandBorderColor);
  Tuple *bwseconds_color_t = dict_find(iter, MESSAGE_KEY_BWSecondsHandColor);
  Tuple *bwseconds_border_t = dict_find(iter, MESSAGE_KEY_BWSecondsHandBorderColor);
  Tuple *font_t = dict_find(iter, MESSAGE_KEY_Font);
  Tuple *fixed_angle_t = dict_find(iter, MESSAGE_KEY_FixedAngle);
  Tuple *angle_t = dict_find(iter, MESSAGE_KEY_Angle);
  Tuple *bg_pattern1_t = dict_find(iter, MESSAGE_KEY_BackgroundPattern1);
  Tuple *bg_pattern2_t = dict_find(iter, MESSAGE_KEY_BackgroundPattern2);
  Tuple *secondary_bg_color1_t = dict_find(iter, MESSAGE_KEY_SecondaryBackgroundColor1);
  Tuple *secondary_bg_color2_t = dict_find(iter, MESSAGE_KEY_SecondaryBackgroundColor2);
  Tuple *no_pattern_under_text_t = dict_find(iter, MESSAGE_KEY_NoPatternUnderText);

  if (enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;

    tick_timer_service_unsubscribe();
    if (settings.EnableSecondsHand && !settings.DigitalWatch) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    } else {
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
  }
  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
  }
  if (enable_pebble_logo_t) {
    settings.EnablePebbleLogo = enable_pebble_logo_t->value->int32 == 1;
  }
  if (enable_watch_model_t) {
    settings.EnableWatchModel = enable_watch_model_t->value->int32 == 1;
  }
  if (forced_watch_model_t) {
    settings.ForcedWatchModel = atoi(forced_watch_model_t->value->cstring);
  }
  if (enable_moonphase_t) {
    settings.EnableMoonphase = enable_moonphase_t->value->int32 == 1;
  }
  if (digital_watch_t) {
    settings.DigitalWatch = digital_watch_t->value->int32 == 1;

    tick_timer_service_unsubscribe();
    if (settings.EnableSecondsHand && !settings.DigitalWatch) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    } else {
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
  }
  if (bg_color1_t) {
    settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32);
  }
  if (bg_color2_t) {
    settings.BackgroundColor2 = GColorFromHEX(bg_color2_t->value->int32);
  }
  if (text_color1_t) {
    settings.TextColor1 = GColorFromHEX(text_color1_t->value->int32);
  }
  if (text_color2_t) {
    settings.TextColor2 = GColorFromHEX(text_color2_t->value->int32);
  }
  if (hours_color_t) {
    settings.HoursHandColor = GColorFromHEX(hours_color_t->value->int32);
  }
  if (hours_border_t) {
    settings.HoursHandBorderColor = GColorFromHEX(hours_border_t->value->int32);
  }
  if (minutes_color_t) {
    settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32);
  }
  if (minutes_border_t) {
    settings.MinutesHandBorderColor = GColorFromHEX(minutes_border_t->value->int32);
  }
  if (seconds_color_t) {
    settings.SecondsHandColor = GColorFromHEX(seconds_color_t->value->int32);
  }
  if (bwbg_color1_t) {
    settings.BWBackgroundColor1 = GColorFromHEX(bwbg_color1_t->value->int32);
  }
  if (bwbg_color2_t) {
    settings.BWBackgroundColor2 = GColorFromHEX(bwbg_color2_t->value->int32);
  }
  if (bwtext_color1_t) {
    settings.BWTextColor1 = GColorFromHEX(bwtext_color1_t->value->int32);
  }
  if (bwtext_color2_t) {
    settings.BWTextColor2 = GColorFromHEX(bwtext_color2_t->value->int32);
  }
  if (bwhours_color_t) {
    settings.BWHoursHandColor = GColorFromHEX(bwhours_color_t->value->int32);
  }
  if (bwhours_border_t) {
    settings.BWHoursHandBorderColor = GColorFromHEX(bwhours_border_t->value->int32);
  }
  if (bwminutes_color_t) {
    settings.BWMinutesHandColor = GColorFromHEX(bwminutes_color_t->value->int32);
  }
  if (bwminutes_border_t) {
    settings.BWMinutesHandBorderColor = GColorFromHEX(bwminutes_border_t->value->int32);
  }
  if (bwseconds_color_t) {
    settings.BWSecondsHandColor = GColorFromHEX(bwseconds_color_t->value->int32);
  }
  if (bwseconds_border_t) {
    settings.BWSecondsHandBorderColor = GColorFromHEX(bwseconds_border_t->value->int32);
  }
  if (font_t) {
    settings.Font = font_t->value->int32;
  }
  if (fixed_angle_t) {
    settings.FixedAngle = fixed_angle_t->value->int32 == 1;
  }
  if (angle_t) {
    settings.Angle = angle_t->value->int32;
  }
  if (bg_pattern1_t) {
    settings.BackgroundPattern1 = atoi(bg_pattern1_t->value->cstring);
  }
  if (bg_pattern2_t) {
    settings.BackgroundPattern2 = atoi(bg_pattern2_t->value->cstring);
  }
  if (secondary_bg_color1_t) {
    settings.SecondaryBackgroundColor1 = GColorFromHEX(secondary_bg_color1_t->value->int32);
  }
  if (secondary_bg_color2_t) {
    settings.SecondaryBackgroundColor2 = GColorFromHEX(secondary_bg_color2_t->value->int32);
  }
  if (no_pattern_under_text_t) {
    settings.NoPatternUnderText = no_pattern_under_text_t->value->int32 == 1;
  }

  prv_save_settings();
  draw_dial();
  unobstructed_change_handler(100, NULL);

#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_DEBUG, "BG dirty because of inbox");
#endif

  layer_mark_dirty(s_canvas_layer);
  layer_mark_dirty(s_bg_layer);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Receive inbox: %d ms", elapsed_ms);
#endif
}

static void set_marker_positions(DialSpec* ds) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  int margin = 2;
  if (is_large_screen()) {
    margin = 8;
  } else if (is_round()) {
    margin = 4;
  }
  int half_digit_width = ds->marker_size.w / 2;
  int half_digit_height = ds->marker_size.h / 2;
  int half_screen_width = bounds.size.w / 2 - margin - half_digit_width;
  int half_screen_height = bounds.size.h / 2 - margin - half_digit_height;

  for (int i = 0; i < 12; i++) {
    int x = -1;
    int y = -1;

    int angle = abs(90 - ((i * 360 / 12) % 180));

    if (is_round()) {
      x = half_screen_width * cosine(angle);
      y = half_screen_height * sine(angle);
    } else {
      if ((float)(half_screen_width) * tangent(angle) <= (float)(half_screen_height)) {
        x = half_screen_width;
        y = (half_screen_width) * tangent(angle);
      } else {
        x = (int)((float)(half_screen_height) / (float)tangent(angle) + 0.5);
        y = half_screen_height;
      }
    }

    if (i >= 0 && i <= 6) {
      x += bounds.size.w / 2;
    } else {
      x = bounds.size.w / 2 - x;
    }

    if (i >= 9 || i <= 3) {
      y = bounds.size.h / 2 - y;
    } else {
      y += bounds.size.h / 2;
    }

    ds->markers[i] = GPoint(x, y);
  }

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to set marker positions: %d ms", elapsed_ms);
#endif
}

DialSpec* get_dial_spec() {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  DialSpec *ds = (DialSpec*) malloc(sizeof(DialSpec));

  int half_screen_width = bounds.size.w / 2;
  int half_screen_height = bounds.size.h / 2;

  // Common resource settings
  ds->logo_res = RESOURCE_ID_LOGO;
  ds->logo_size = GSize(38, 12);
  ds->logo = GPoint(half_screen_width, bounds.size.h * 0.19 + ds->logo_size.h / 2);
  ds->models_res = RESOURCE_ID_MODELS;
  ds->model_size = GSize(71, 5);
  ds->model = GPoint(half_screen_width, bounds.size.h * 0.19 + ds->logo_size.h + 1 + ds->model_size.h / 2);
  ds->day_res = RESOURCE_ID_DAYS;
  ds->day_size = GSize(30, 10);
  ds->moonphase_res = RESOURCE_ID_MOONPHASES;
  ds->moonphase_size = GSize(10, 10);
  ds->moonphase = GPoint(half_screen_width, bounds.size.h * 0.19 + ds->logo_size.h + 1 + ds->model_size.h + 1 + ds->moonphase_size.h / 2 + 5);


  // Font-dependant resources and positions (date, markers, digital time)
  if (get_font() == 1) {
    ds->date_box_res = RESOURCE_ID_DATE_BOX1;
    ds->date_box_size = GSize(34, 22);
    ds->digit_res = RESOURCE_ID_DIGITS1;
    ds->digit_size = GSize(10, 14);
    ds->marker_res = RESOURCE_ID_MARKERS1;
    ds->marker_size = GSize(22, 19);

    ds->digital_box_res = RESOURCE_ID_DIGITAL_BOX1;
    ds->digital_box_size = GSize(116, 45);
    ds->digital_colon_res = RESOURCE_ID_DIGITAL_COLON1;
    ds->digital_colon_size = GSize(4, 20);
    ds->digit_big_res = RESOURCE_ID_DIGITS_BIG1;
    ds->digit_big_size = GSize(20, 28);
  } else if (get_font() == 2) {
    ds->date_box_res = RESOURCE_ID_DATE_BOX2;
    ds->date_box_size = GSize(30, 18);
    ds->digit_res = RESOURCE_ID_DIGITS2;
    ds->digit_size = GSize(10, 10);
    ds->marker_res = RESOURCE_ID_MARKERS2;
    ds->marker_size = GSize(22, 15);

    ds->digital_box_res = RESOURCE_ID_DIGITAL_BOX2;
    ds->digital_box_size = GSize(108, 36);
    ds->digital_colon_res = RESOURCE_ID_DIGITAL_COLON2;
    ds->digital_colon_size = GSize(2, 12);
    ds->digit_big_res = RESOURCE_ID_DIGITS_BIG2;
    ds->digit_big_size = GSize(20, 20);
  } else {
    ds->date_box_res = RESOURCE_ID_DATE_BOX3;
    ds->date_box_size = GSize(30, 17);
    ds->digit_res = RESOURCE_ID_DIGITS3;
    ds->digit_size = GSize(10, 11);
    ds->marker_res = RESOURCE_ID_MARKERS3;
    ds->marker_size = GSize(26, 16);

    ds->digital_box_res = RESOURCE_ID_DIGITAL_BOX3;
    ds->digital_box_size = GSize(108, 34);
    ds->digital_colon_res = RESOURCE_ID_DIGITAL_COLON3;
    ds->digital_colon_size = GSize(2, 14);
    ds->digit_big_res = RESOURCE_ID_DIGITS_BIG3;
    ds->digit_big_size = GSize(20, 22);
  }

  // Calculate positions for markers.
  set_marker_positions(ds);

  // Date box position (only for analog watch).
  ds->date_box = GPoint(half_screen_width, bounds.size.h * 0.72);

  if (!is_digital()) {
    // Date positioning for analog watch.
    int date_y = bounds.size.h * 0.72;
    ds->date1 = GPoint(half_screen_width - ds->digit_size.w / 2 - 1, date_y);
    ds->date2 = GPoint(half_screen_width + ds->digit_size.w / 2 + 1, date_y);
    ds->date_single = GPoint(half_screen_width, date_y);
  } else {
    // Digital watch date uses different digit font.
    ds->digit_res = RESOURCE_ID_DIGITS_DIGITAL;
    ds->digit_size = GSize(8, 10);

    // Digital time positioning.
    int digital_time_x = 0;
    int digital_time_y = 0;
    if (!is_round()) {
      digital_time_y = bounds.size.h * 0.82;
      if (is_large_screen()) {
        digital_time_x = bounds.size.w - ds->digital_box_size.w / 2 - 8;
      } else {
        digital_time_x = bounds.size.w - ds->digital_box_size.w / 2 - 8;
      }
    } else {
      digital_time_y = bounds.size.h * 0.7;
      digital_time_x = half_screen_width;
    }

    ds->digital_box = GPoint(digital_time_x, digital_time_y);
    ds->digital_colon = GPoint(digital_time_x, digital_time_y);
    ds->digital_time2 = GPoint(digital_time_x - ds->digital_colon_size.w / 2 - 4 - ds->digit_big_size.w / 2, digital_time_y);
    ds->digital_time1 = GPoint(digital_time_x - ds->digital_colon_size.w / 2 - 4 - ds->digit_big_size.w - 4 - ds->digit_big_size.w / 2, digital_time_y);
    ds->digital_time3 = GPoint(digital_time_x + ds->digital_colon_size.w / 2 + 4 + ds->digit_big_size.w / 2, digital_time_y);
    ds->digital_time4 = GPoint(digital_time_x + ds->digital_colon_size.w / 2 + 4 + ds->digit_big_size.w + 4 + ds->digit_big_size.w / 2, digital_time_y);

    // Digital day + date positioning.
    int digital_date_y = digital_time_y - ds->digital_box_size.h / 2 - 1 - ds->digit_size.h / 2;
    int day_date_width = ds->day_size.w + 4 + ds->digit_size.w * 2 + 2;
    int digital_date_x = half_screen_width;
    if (!is_round()) {
      digital_date_x = digital_time_x + ds->digital_box_size.w / 2 - 4 - day_date_width / 2;
    }

    ds->day = GPoint(digital_date_x - day_date_width / 2 + ds->day_size.w / 2, digital_date_y);
    ds->date1 = GPoint(digital_date_x + day_date_width / 2 - ds->digit_size.w - 2 - ds->digit_size.w / 2, digital_date_y);
    ds->date2 = GPoint(digital_date_x + day_date_width / 2  - ds->digit_size.w / 2, digital_date_y);
  }

  return ds;

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to get dial spec: %d ms", elapsed_ms);
#endif
}

static int get_font() {
  return settings.Font;
}

static bool is_digital() {
  return settings.DigitalWatch;
}

static bool is_round() {
#ifdef PBL_ROUND
  return true;
#else
  return false;
#endif
}

static bool is_large_screen() {
  if (PBL_PLATFORM_TYPE_CURRENT == PlatformTypeEmery) {
    return true;
  } else {
    return false;
  }
}

static void update_date() {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  binary_image_mask_data_clear_region(dial, GRect(ds->date1.x, ds->date1.y, ds->digit_size.w, ds->digit_size.h), true);
  binary_image_mask_data_clear_region(dial, GRect(ds->date2.x, ds->date2.y, ds->digit_size.w, ds->digit_size.h), true);
  int d1 = current_date / 10;
  int d2 = current_date % 10;
  binary_image_mask_data_draw(dial, digits, ds->date1, GRect(d1*ds->digit_size.w, 0, ds->digit_size.w, ds->digit_size.h), true);
  binary_image_mask_data_draw(dial, digits, ds->date2, GRect(d2*ds->digit_size.w, 0, ds->digit_size.w, ds->digit_size.h), true);

  if (is_digital()) {
    binary_image_mask_data_clear_region(dial, GRect(ds->day.x, ds->day.y, ds->day_size.w, ds->day_size.h), true);
    BinaryImageMaskData *day = binary_image_mask_data_create_from_resource(GSize(ds->day_size.w, ds->day_size.h * 7), ds->day_res);
    binary_image_mask_data_draw(dial, day, ds->day, GRect(0, current_day*ds->day_size.h, ds->day_size.w, ds->day_size.h), true);
    binary_image_mask_data_destroy(day);
    day = NULL;
  }

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to update date: %d ms", elapsed_ms);
#endif
}

static int get_moonphase_index() {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  const double synodic_month = 29.53;
  struct tm ref = {0};
  ref.tm_year = 2000 - 1900;
  ref.tm_mon = 0;
  ref.tm_mday = 6;
  time_t ref_time = mktime(&ref);

  time_t target_time = mktime(prv_tick_time);
  double days_since_ref = difftime(target_time, ref_time) / (60.0 * 60.0 * 24.0);

  double moon_age = fmod(days_since_ref, synodic_month);
  if (moon_age < 0) moon_age += synodic_month;

  double phase_fraction = moon_age / synodic_month;

  int index = (int)(phase_fraction * 20.0);
  if (index > 19) index = 19;

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to get moonphase index: %d ms", elapsed_ms);
#endif

  return index;
}

static void update_moonphase() {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  BinaryImageMaskData *moonphases = binary_image_mask_data_create_from_resource(GSize(ds->moonphase_size.w, ds->moonphase_size.h * 20), ds->moonphase_res);
  binary_image_mask_data_clear_region(dial, GRect(ds->moonphase.x, ds->moonphase.y, ds->moonphase_size.w, ds->moonphase_size.h), true);
#ifdef MOONPHASE
  int phase = MOONPHASE;
#else
  int phase = get_moonphase_index();
#endif
  binary_image_mask_data_draw(dial, moonphases, ds->moonphase, GRect(0, phase*ds->moonphase_size.h, ds->moonphase_size.w, ds->moonphase_size.h), true);
  binary_image_mask_data_destroy(moonphases);
  moonphases = NULL;

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to update moonphase: %d ms", elapsed_ms);
#endif
}

static void update_digital_time() {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time1.x, ds->digital_time1.y, ds->digit_big_size.w, ds->digit_big_size.h), true);
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time2.x, ds->digital_time2.y, ds->digit_big_size.w, ds->digit_big_size.h), true);
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time3.x, ds->digital_time3.y, ds->digit_big_size.w, ds->digit_big_size.h), true);
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time4.x, ds->digital_time4.y, ds->digit_big_size.w, ds->digit_big_size.h), true);

#ifdef QUICKTEST
  int seconds = prv_tick_time->tm_sec;
#endif
  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour;

  if (!clock_is_24h_style()) {
    hours = hours % 12;

    if (hours == 0) {
      hours = 12;
    }
  }
#ifdef HOUR
  hours = HOUR;
#endif

#ifdef MINUTE
  minutes = MINUTE;
#endif

#ifdef SECOND
#ifdef QUICKTEST
  seconds = SECOND;
#endif
#endif

#ifdef QUICKTEST
  minutes = seconds;
  seconds = 0;
#endif

  int h1 = hours / 10;
  int h2 = hours % 10;
  int m1 = minutes / 10;
  int m2 = minutes % 10;


  binary_image_mask_data_draw(dial, digits_big, ds->digital_time1, GRect(h1*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h), true);
  binary_image_mask_data_draw(dial, digits_big, ds->digital_time2, GRect(h2*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h), true);
  binary_image_mask_data_draw(dial, digits_big, ds->digital_time3, GRect(m1*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h), true);
  binary_image_mask_data_draw(dial, digits_big, ds->digital_time4, GRect(m2*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h), true);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to update digital time: %d ms", elapsed_ms);
#endif
}

static void unobstructed_change_handler(AnimationProgress progress, void *context) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  Layer *window_layer = window_get_root_layer(s_window);
  GRect fullscreen = layer_get_frame(window_layer);
  GRect obstructed = layer_get_unobstructed_bounds(window_layer);

  int offset = (fullscreen.size.h - obstructed.size.h);
  if (!is_digital()) {
    offset = offset / 2;
  }

  if (offset != -offset_y) {
    offset_y = -offset;
    fullscreen.origin.y = 0 - offset;
    layer_set_frame(window_layer, fullscreen);
  }

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to handle unobstructed: %d ms", elapsed_ms);
#endif
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  if (tick_time->tm_min == prv_tick_time->tm_min
      && tick_time->tm_hour == prv_tick_time->tm_hour
      && (tick_time->tm_sec == prv_tick_time->tm_sec || !settings.EnableSecondsHand || settings.DigitalWatch)) {
#ifdef LOG
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Tick handler returning due to no time diff.");
#endif
    return;
  }

  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);

#ifdef LOGTIME
  int seconds = prv_tick_time->tm_sec;
  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour;
  APP_LOG(APP_LOG_LEVEL_INFO, "Time: %d:%d:%d", hours, minutes, seconds);
#endif

  if (settings.EnableDate || settings.EnableMoonphase) {
    int date = prv_tick_time->tm_mday;
    if (current_date != date) {
#ifdef DATE
      current_date = DATE;
#else
      current_date = date;
#endif

#ifdef DAY
      current_day = DAY;
#else
      current_day = prv_tick_time->tm_wday;
#endif
      if (settings.EnableDate) {
        update_date();
      }

      if (settings.EnableMoonphase) {
        update_moonphase();
      }
    }
  }

  if (settings.DigitalWatch) {
    update_digital_time();
  }

#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Canvas & BG dirty because of tick");
#endif

  layer_mark_dirty(s_canvas_layer);
  layer_mark_dirty(s_bg_layer);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time handle tick: %d ms", elapsed_ms);
#endif
}


static void draw_fancy_hand(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  int p1l = 15;
  int p2l = length;
  GPoint p1 = polar_to_point_offset(origin, angle, p1l);
  GPoint p2 = polar_to_point_offset(origin, angle, p2l);

  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_fill_color(ctx, border_color);
  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, origin, p1);
  graphics_context_set_stroke_width(ctx, 7);
  graphics_draw_line(ctx, p1, p2);
  graphics_context_set_stroke_color(ctx, fill_color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, p1, p2);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to draw fancy hand: %d ms", elapsed_ms);
#endif
}

static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1 = polar_to_point_offset(origin, angle + 180, back_length);
  GPoint p2 = polar_to_point_offset(origin, angle, length);
  graphics_context_set_antialiased(ctx, false);

  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, p1, p2);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to draw line hand: %d ms", elapsed_ms);
#endif
}

static void draw_line_hand_with_border(GContext *ctx, int angle, int length, int back_length, GColor color, GColor border_color) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1 = polar_to_point_offset(origin, angle + 180, back_length);
  GPoint p2 = polar_to_point_offset(origin, angle, length);
  graphics_context_set_antialiased(ctx, false);

  graphics_context_set_fill_color(ctx, border_color);
  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, p1, p2);

  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, p1, p2);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to draw line hand: %d ms", elapsed_ms);
#endif
}

static void draw_center(GContext *ctx, GColor minutes_border, GColor minutes_color, GColor seconds_color) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_fill_color(ctx, minutes_border);
  graphics_fill_circle(ctx, origin, 6);
  graphics_context_set_fill_color(ctx, seconds_color);
  graphics_fill_circle(ctx, origin, 4);
  graphics_context_set_fill_color(ctx, minutes_color);
  graphics_fill_circle(ctx, origin, 2);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to draw center: %d ms", elapsed_ms);
#endif
}


static void canvas_update_proc(Layer *layer, GContext *ctx) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  int seconds = prv_tick_time->tm_sec;
  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour % 12;

#ifdef HOUR
  hours = HOUR;
#endif

#ifdef MINUTE
  minutes = MINUTE;
#endif

#ifdef SECOND
  seconds = SECOND;
#endif

  if (!settings.EnableSecondsHand) {
    seconds = 0;
  }

#ifdef QUICKTEST
  minutes = seconds;
  seconds = 0;
#endif

  if (!settings.DigitalWatch) {
    int minutes_angle = (minutes * 6) + (seconds / 10) - 90;
    int hours_angle = (hours * 30) + (minutes / 2) + (seconds / 120) - 90;

#ifdef PBL_COLOR
    draw_fancy_hand(ctx, minutes_angle, bounds.size.w / 2 - 10, settings.MinutesHandColor, settings.MinutesHandBorderColor);
    draw_fancy_hand(ctx, hours_angle, bounds.size.w / 2 - 30, settings.HoursHandColor, settings.HoursHandBorderColor);
#else
    draw_fancy_hand(ctx, minutes_angle, bounds.size.w / 2 - 10, settings.BWMinutesHandColor, settings.BWMinutesHandBorderColor);
    draw_fancy_hand(ctx, hours_angle, bounds.size.w / 2 - 30, settings.BWHoursHandColor, settings.BWHoursHandBorderColor);
#endif

    if (settings.EnableSecondsHand) {
      int seconds_angle = (seconds * 6) - 90;
#ifdef PBL_COLOR
      draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 5, 15, settings.SecondsHandColor);
#else
      draw_line_hand_with_border(ctx, seconds_angle, bounds.size.w / 2 - 5, 15, settings.BWSecondsHandColor, settings.BWSecondsHandBorderColor);
#endif
    }

#ifdef PBL_COLOR
    draw_center(ctx, settings.HoursHandBorderColor, settings.HoursHandColor, settings.SecondsHandColor);
#else
    draw_center(ctx, settings.BWHoursHandBorderColor, settings.BWHoursHandColor, settings.BWSecondsHandBorderColor);
#endif
  }

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to draw canvas: %d ms", elapsed_ms);
#endif
}

static bool byte_get_bit(uint8_t *byte, uint8_t bit) {
  return ((*byte) >> bit) & 1;
}

static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value) {
  *byte ^= (-value ^ *byte) & (1 << bit);
}

static void set_pixel_color(GBitmapDataRowInfo info, GPoint point, GColor color) {
#if defined(PBL_COLOR)
  // Write the pixel's byte color
  memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
  // Find the correct byte, then set the appropriate bit
  uint8_t byte = point.x / 8;
  uint8_t bit = point.x % 8;
  byte_set_bit(&info.data[byte], bit, gcolor_equal(color, GColorWhite) ? 1 : 0);
#endif
}

static GColor flip_color_bw(GColor color) {
  if (color.argb == 0xFF) {
    return GColorBlack;
  }
  return GColorWhite;
}

static GColor handle_background_pattern(int x, int y, bool is_in_bg1) {
  enum BackgroundPattern pattern = (is_in_bg1) ? settings.BackgroundPattern1 : settings.BackgroundPattern2;
#ifdef PBL_COLOR
  GColor color1 = (is_in_bg1) ? settings.BackgroundColor1 : settings.BackgroundColor2;
  GColor color2 = (is_in_bg1) ? settings.SecondaryBackgroundColor1 : settings.SecondaryBackgroundColor2;
#else
  GColor color1 = (is_in_bg1) ? settings.BWBackgroundColor1 : settings.BWBackgroundColor2;
  GColor color2 = (is_in_bg1) ? flip_color_bw(settings.BWBackgroundColor1) : flip_color_bw(settings.BWBackgroundColor2);
#endif

  GColor color = color1;

  if (pattern != SOLID && color1.argb != color2.argb && (!settings.NoPatternUnderText || !binary_image_mask_data_is_filled_or_adjacent(dial, x, y, 1))) {
    switch (pattern) {
      case DITHER_STRONG:
        if (y % 2 == 0 && (((y % 4) + x) % 4) == 0) {
          color = color2;
        }
        break;
      case DITHER_LIGHT:
        if ((y * bounds.size.w + x) % 23 == 0) {
          color = color2;
        }
        break;
      case VERTICAL_STRONG:
        if (x % 4 == 0) {
          color = color2;
        }
        break;
      case VERTICAL_LIGHT:
        if (x % 6 == 0) {
          color = color2;
        }
        break;
      case HORIZONTAL_STRONG:
        if (y % 4 == 0) {
          color = color2;
        }
        break;
      case HORIZONTAL_LIGHT:
        if (y % 6 == 0) {
          color = color2;
        }
        break;
      case DIAGONAL:
        if ((y + x) % 6 == 0) {
          color = color2;
        }
        break;
      case DIAGONAL_MIRROR:
        if ((bounds.size.w -y + x) % 6 == 0) {
          color = color2;
        }
        break;
      case CHECKERBOARD:
        if (((y / 10) % 2 == 0 && (x / 10) % 2 == 0)
            || ((y / 10) % 2 == 1 && (x / 10) % 2 == 1)) {
          color = color2;
        }
        break;
      case SOLID:
      default:
        break;
    }
  }

  return color;
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Time is %d:%d:%d", prv_tick_time->tm_hour, prv_tick_time->tm_min, prv_tick_time->tm_sec);
#endif

  int minutes = prv_tick_time->tm_min;
  int seconds = prv_tick_time->tm_sec;

  GBitmap *fb = graphics_capture_frame_buffer(ctx);

#ifdef MINUTE
  minutes = MINUTE;
#endif

#ifdef SECOND
  seconds = SECOND;
#endif

  if (!settings.EnableSecondsHand) {
    seconds = 0;
  }

#ifdef QUICKTEST
  minutes = seconds;
  seconds = 0;
#endif

  int angle = settings.FixedAngle
    ? (settings.Angle)
    : ((minutes * 6) + (seconds / 10)); // 6º per minute
  angle -= 90; // Align so 0º points up like

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p = polar_to_point_offset(origin, angle, bounds.size.h);

  int cx = origin.x;
  int cy = origin.y;
  int px = p.x;
  int py = p.y;

  for(int y = MAX(0, offset_y); y < bounds.size.h + offset_y; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
    size_t row_offset = (size_t)dial->size.w * (y - offset_y);

    for (int x = info.min_x; x <= info.max_x; x++) {
      // Cross product test for side of line
      int cross = ((x - cx) * (py - cy)) - ((y - cy) * (px - cx));
      bool is_in_bg1 = (cross < 0);

      size_t index = row_offset + x;
      bool is_in_dial = (dial->data[index >> 3] >> (index & 7)) & 1;

      GColor color;
      if (is_in_dial) {
#ifdef PBL_COLOR
        color = is_in_bg1 ? settings.TextColor1 : settings.TextColor2;
#else
        color = is_in_bg1 ? settings.BWTextColor1 : settings.BWTextColor2;
#endif
      } else {
        color = handle_background_pattern(x, y, is_in_bg1);
      }

      set_pixel_color(info, GPoint(x, y), color);
    }
  }
  graphics_release_frame_buffer(ctx, fb);

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to update BG: %d ms", elapsed_ms);
#endif
}

static void draw_dial() {
#ifdef LOGPERF
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
#endif

  if (dial != NULL) {
    binary_image_mask_data_destroy(dial);
    dial = NULL;
  }

  if (digits != NULL) {
    binary_image_mask_data_destroy(digits);
    digits = NULL;
  }
  if (digits_big != NULL) {
    binary_image_mask_data_destroy(digits_big);
    digits_big = NULL;
  }

  if (ds != NULL) {
    free(ds);
    ds = NULL;
  }
  ds = get_dial_spec();

  dial = binary_image_mask_data_create(bounds.size);
  digits = binary_image_mask_data_create_from_resource(GSize(ds->digit_size.w * 10, ds->digit_size.h), ds->digit_res);

  if (!settings.DigitalWatch) {
    BinaryImageMaskData *markers = binary_image_mask_data_create_from_resource(GSize(ds->marker_size.w, ds->marker_size.h * 12), ds->marker_res);
    for (int i = 0; i < 12; i++) {
      binary_image_mask_data_draw(dial, markers, ds->markers[i], GRect(0, ds->marker_size.h * i, ds->marker_size.w, ds->marker_size.h), true);
    }
    binary_image_mask_data_destroy(markers);
    markers = NULL;
  } else {
    digits_big = binary_image_mask_data_create_from_resource(GSize(ds->digit_big_size.w * 10, ds->digit_big_size.h), ds->digit_big_res);
  }


  if (settings.EnablePebbleLogo) {
    BinaryImageMaskData *logo = binary_image_mask_data_create_from_resource(ds->logo_size, ds->logo_res);
    binary_image_mask_data_draw(dial, logo, ds->logo, GRect(0, 0, ds->logo_size.w, ds->logo_size.h), true);
    binary_image_mask_data_destroy(logo);
    logo = NULL;
  }

  if (settings.EnableWatchModel) {
    BinaryImageMaskData *models = binary_image_mask_data_create_from_resource(GSize(ds->model_size.w, ds->model_size.h * MODEL_COUNT), ds->models_res);
    int index = settings.ForcedWatchModel;

    if (index == -1) {
      switch(watch_info_get_model()) {
        case WATCH_INFO_MODEL_PEBBLE_ORIGINAL:
          index = 0;
          break;
        case WATCH_INFO_MODEL_PEBBLE_STEEL:
          index = 1;
          break;
        case WATCH_INFO_MODEL_PEBBLE_TIME:
          index = 2;
          break;
        case WATCH_INFO_MODEL_PEBBLE_TIME_STEEL:
          index = 3;
          break;
        case WATCH_INFO_MODEL_PEBBLE_TIME_ROUND_14:
        case WATCH_INFO_MODEL_PEBBLE_TIME_ROUND_20:
          index = 4;
          break;
        case WATCH_INFO_MODEL_PEBBLE_2_HR:
          index = 6;
          break;
        case WATCH_INFO_MODEL_PEBBLE_2_SE:
          index = 7;
          break;
          // There's a bug in the SDK that requires this preprocessor condition
          // https://discord.com/channels/221364737269694464/264746316477759489/1443423122517921844
#ifndef PBL_PLATFORM_APLITE
        case WATCH_INFO_MODEL_COREDEVICES_C2D:
          index = 9;
          break;
#endif
        case WATCH_INFO_MODEL_PEBBLE_TIME_2:
#ifndef PBL_PLATFORM_APLITE
        case WATCH_INFO_MODEL_COREDEVICES_CT2:
#endif
          index = 10;
          break;
        default:
          index = 8;
          break;
      }
    }

    binary_image_mask_data_draw(dial, models, ds->model, GRect(0, ds->model_size.h * index, ds->model_size.w, ds->model_size.h), true);
    binary_image_mask_data_destroy(models);
    models = NULL;
  }

  if (settings.EnableDate && !settings.DigitalWatch) {
    BinaryImageMaskData *date_box = binary_image_mask_data_create_from_resource(ds->date_box_size, ds->date_box_res);
    binary_image_mask_data_draw(dial, date_box, ds->date_box, GRect(0, 0, ds->date_box_size.w, ds->date_box_size.h), true);
    binary_image_mask_data_destroy(date_box);
    date_box = NULL;
  }

  if (settings.EnableDate) {
    update_date();
  }

  if (settings.EnableMoonphase) {
    update_moonphase();
  }

  if (settings.DigitalWatch) {
    BinaryImageMaskData *digital_box = binary_image_mask_data_create_from_resource(ds->digital_box_size, ds->digital_box_res);
    binary_image_mask_data_draw(dial, digital_box, ds->digital_box, GRect(0, 0, ds->digital_box_size.w, ds->digital_box_size.h), true);
    binary_image_mask_data_destroy(digital_box);
    digital_box = NULL;

    BinaryImageMaskData *digital_colon = binary_image_mask_data_create_from_resource(ds->digital_colon_size, ds->digital_colon_res);
    binary_image_mask_data_draw(dial, digital_colon, ds->digital_colon, GRect(0, 0, ds->digital_colon_size.w, ds->digital_colon_size.h), true);
    binary_image_mask_data_destroy(digital_colon);
    digital_colon = NULL;

    update_digital_time();
  }

#ifdef LOGPERF
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to draw dial: %d ms", elapsed_ms);
#endif
}

static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);
#ifdef DATE
  current_date = DATE;
#else
  current_date = prv_tick_time->tm_mday;
#endif

#ifdef DAY
  current_day = DAY;
#else
  current_day = prv_tick_time->tm_wday;
#endif
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  draw_dial();

  s_bg_layer = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_bg_layer);
  layer_add_child(window_layer, s_canvas_layer);

  tick_timer_service_unsubscribe();
  if (settings.EnableSecondsHand && !settings.DigitalWatch) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }

  unobstructed_change_handler(100, NULL);

  UnobstructedAreaHandlers unobstructed_handlers = {
    .change = unobstructed_change_handler
  };

  unobstructed_area_service_subscribe(unobstructed_handlers, NULL);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  s_canvas_layer = NULL;
  layer_destroy(s_bg_layer);
  s_bg_layer = NULL;
  if (dial != NULL) {
    binary_image_mask_data_destroy(dial);
    dial = NULL;
  }
  if (digits != NULL) {
    binary_image_mask_data_destroy(digits);
    digits = NULL;
  }
  if (digits_big != NULL) {
    binary_image_mask_data_destroy(digits_big);
    digits_big = NULL;
  }
  if (ds != NULL) {
    free(ds);
    ds = NULL;
  }
}

static void prv_init(void) {
  prv_load_settings();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, false);

  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 0);
}

static void prv_deinit(void) {
  window_destroy(s_window);
  s_window = NULL;
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
