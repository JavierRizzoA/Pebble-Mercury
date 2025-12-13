#include <pebble.h>
#include "Mercury.h"
#include "utils/MathUtils.h"
#include "utils/BinaryImageMaskData.h"

static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static GRect bounds;

static struct tm *prv_tick_time;

static struct BinaryImageMaskData *dial;
static struct BinaryImageMaskData *digits;
static struct BinaryImageMaskData *digits_big;
static int current_date = -1;
static int current_day = -1;
static ClaySettings settings;
static DialSpec *ds;
static int offset_y = 0;

//#define LOG
//#define LOGTIME
//#define QUICKTEST

//#define HOUR 22
//#define MINUTE 6
//#define SECOND 30
//#define DAY 6
//#define DATE 6

//#define DIGITAL true
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
static void update_digital_time(struct tm *tick_time);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void draw_fancy_hand(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color);
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
static bool byte_get_bit(uint8_t *byte, uint8_t bit);
static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value);
static void set_pixel_color(GBitmapDataRowInfo info, GPoint point, GColor color);

static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_default_settings(void) {
  settings.EnableSecondsHand = true;
  settings.EnableDate = true;
  settings.EnablePebbleLogo = true;
  settings.EnableWatchModel = true;
  settings.EnableMoonphase = true;
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
#ifdef FONT
  settings.Font = FONT;
#else
  settings.Font = 1;
#endif
  settings.FixedAngle = false;
  settings.Angle = 40;
}

static void prv_load_settings(void) {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Received message");
#endif

  Tuple *enable_seconds_t = dict_find(iter, MESSAGE_KEY_EnableSecondsHand);
  Tuple *enable_date_t = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_pebble_logo_t = dict_find(iter, MESSAGE_KEY_EnablePebbleLogo);
  Tuple *enable_watch_model_t = dict_find(iter, MESSAGE_KEY_EnableWatchModel);
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
  Tuple *font_t = dict_find(iter, MESSAGE_KEY_Font);
  Tuple *fixed_angle_t = dict_find(iter, MESSAGE_KEY_FixedAngle);
  Tuple *angle_t = dict_find(iter, MESSAGE_KEY_Angle);

  if(enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;
    tick_timer_service_unsubscribe();
    if (settings.EnableSecondsHand && !settings.DigitalWatch) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    } else {
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
  }
  if(enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
  }
  if(enable_pebble_logo_t) {
    settings.EnablePebbleLogo = enable_pebble_logo_t->value->int32 == 1;
  }
  if(enable_watch_model_t) {
    settings.EnableWatchModel = enable_watch_model_t->value->int32 == 1;
  }
  if(enable_moonphase_t) {
    settings.EnableMoonphase = enable_moonphase_t->value->int32 == 1;
  }
  if(digital_watch_t) {
    settings.DigitalWatch = digital_watch_t->value->int32 == 1;
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
  if (font_t) {
    settings.Font = font_t->value->int32;
  }
  if (fixed_angle_t) {
    settings.FixedAngle = fixed_angle_t->value->int32 == 1;
  }
  if (angle_t) {
    settings.Angle = angle_t->value->int32;
  }

  prv_save_settings();
  draw_dial();
  layer_mark_dirty(s_bg_layer);
}



DialSpec* get_dial_spec(enum DialType dial_type) {
  DialSpec *ds = (DialSpec*) malloc(sizeof(DialSpec));

  ds->logo_size = GSize(38, 12);
  ds->model_size = GSize(71, 5);
  ds->day_size = GSize(30, 10);
  ds->moonphase_size = GSize(10, 10);
  ds->logo_res = RESOURCE_ID_LOGO;
  ds->models_res = RESOURCE_ID_MODELS;
  ds->day_res = RESOURCE_ID_DAYS;
  ds->moonphase_res = RESOURCE_ID_MOONPHASES;

  ds->moonphase = GPoint(67, 52);

  if (!is_round()) {
    ds->logo = GPoint(53, 27);
    ds->model = GPoint(36, 40);
  }
  else {
    ds->logo = GPoint(71, 39);
    ds->model = GPoint(54, 52);
  }

  if (get_font() == 1) {
    ds->date_box_size = GSize(34, 22);
    ds->digit_size = GSize(10, 14);
    ds->marker_size = GSize(22, 19);

    ds->date_box_res = RESOURCE_ID_DATE_BOX1;
    ds->digit_res = RESOURCE_ID_DIGITS1;
    ds->marker_res = RESOURCE_ID_MARKERS1;

    ds->digital_box_size = GSize(116, 45);
    ds->digital_colon_size = GSize(4, 20);
    ds->digit_big_size = GSize(20, 28);

    ds->digital_box_res = RESOURCE_ID_DIGITAL_BOX1;
    ds->digital_colon_res = RESOURCE_ID_DIGITAL_COLON1;
    ds->digit_big_res = RESOURCE_ID_DIGITS_BIG1;
  } else if (get_font() == 2) {
    ds->date_box_size = GSize(30, 18);
    ds->digit_size = GSize(10, 10);
    ds->marker_size = GSize(22, 15);

    ds->date_box_res = RESOURCE_ID_DATE_BOX2;
    ds->digit_res = RESOURCE_ID_DIGITS2;
    ds->marker_res = RESOURCE_ID_MARKERS2;

    ds->digital_box_size = GSize(108, 36);
    ds->digital_colon_size = GSize(2, 12);
    ds->digit_big_size = GSize(20, 20);

    ds->digital_box_res = RESOURCE_ID_DIGITAL_BOX2;
    ds->digital_colon_res = RESOURCE_ID_DIGITAL_COLON2;
    ds->digit_big_res = RESOURCE_ID_DIGITS_BIG2;
  } else if (get_font() == 3) {
    ds->date_box_size = GSize(30, 17);
    ds->digit_size = GSize(10, 11);
    ds->marker_size = GSize(26, 16);

    ds->date_box_res = RESOURCE_ID_DATE_BOX3;
    ds->digit_res = RESOURCE_ID_DIGITS3;
    ds->marker_res = RESOURCE_ID_MARKERS3;

    ds->digital_box_size = GSize(108, 34);
    ds->digital_colon_size = GSize(2, 14);
    ds->digit_big_size = GSize(20, 22);

    ds->digital_box_res = RESOURCE_ID_DIGITAL_BOX3;
    ds->digital_colon_res = RESOURCE_ID_DIGITAL_COLON3;
    ds->digit_big_res = RESOURCE_ID_DIGITS_BIG3;
  } else {
    free(ds);
    return ds;
  }

  if (is_digital()) {
    ds->digit_res = RESOURCE_ID_DIGITS_DIGITAL;
    ds->digit_size = GSize(8, 10);
  }

  switch (dial_type) {
    case FONT1:
    case FONT1_DIGITAL:
      ds->markers[0] = GPoint(61, 2);
      ds->markers[1] = GPoint(105, 2);
      ds->markers[2] = GPoint(120, 39);
      ds->markers[3] = GPoint(120, 74);
      ds->markers[4] = GPoint(120, 110);
      ds->markers[5] = GPoint(105, 147);
      ds->markers[6] = GPoint(61, 147);
      ds->markers[7] = GPoint(17, 147);
      ds->markers[8] = GPoint(2, 110);
      ds->markers[9] = GPoint(2, 74);
      ds->markers[10] = GPoint(2, 39);
      ds->markers[11] = GPoint(17, 2);

      ds->date_box = GPoint(55, 114);
      ds->date_single = GPoint(67, 118);
      ds->day = GPoint(84, 105);
      if (!is_digital()) {
        ds->date1 = GPoint(61, 118);
        ds->date2 = GPoint(73, 118);
      } else {
        ds->date1 = GPoint(118, 105);
        ds->date2 = GPoint(128, 105);
      }

      ds->digital_box = GPoint(24, 116);
      ds->digital_time1 = GPoint(32, 124);
      ds->digital_time2 = GPoint(56, 124);
      ds->digital_time3 = GPoint(88, 124);
      ds->digital_time4 = GPoint(112, 124);
      ds->digital_colon = GPoint(80, 128);

      break;
    case FONT1_ROUND:
    case FONT1_ROUND_DIGITAL:
      ds->markers[0] = GPoint(79, 5);
      ds->markers[1] = GPoint(118, 20);
      ds->markers[2] = GPoint(143, 45);
      ds->markers[3] = GPoint(152, 80);
      ds->markers[4] = GPoint(143, 115);
      ds->markers[5] = GPoint(118, 140);
      ds->markers[6] = GPoint(79, 155);
      ds->markers[7] = GPoint(40, 140);
      ds->markers[8] = GPoint(15, 115);
      ds->markers[9] = GPoint(6, 80);
      ds->markers[10] = GPoint(15, 45);
      ds->markers[11] = GPoint(40, 20);

      ds->date_box = GPoint(73, 115);
      ds->date_single = GPoint(85, 119);
      ds->day = GPoint(64, 94);
      if (!is_digital()) {
        ds->date1 = GPoint(79, 119);
        ds->date2 = GPoint(91, 119);
      } else {
        ds->date1 = GPoint(100, 94);
        ds->date2 = GPoint(108, 94);
      }

      ds->digital_box = GPoint(32, 105);
      ds->digital_time1 = GPoint(40, 113);
      ds->digital_time2 = GPoint(64, 113);
      ds->digital_time3 = GPoint(96, 113);
      ds->digital_time4 = GPoint(120, 113);
      ds->digital_colon = GPoint(88, 117);

      break;
    case FONT2:
    case FONT2_DIGITAL:
      ds->markers[0] = GPoint(61, 2);
      ds->markers[1] = GPoint(105, 2);
      ds->markers[2] = GPoint(120, 41);
      ds->markers[3] = GPoint(120, 76);
      ds->markers[4] = GPoint(120, 112);
      ds->markers[5] = GPoint(105, 151);
      ds->markers[6] = GPoint(61, 151);
      ds->markers[7] = GPoint(17, 151);
      ds->markers[8] = GPoint(2, 112);
      ds->markers[9] = GPoint(2, 76);
      ds->markers[10] = GPoint(2, 41);
      ds->markers[11] = GPoint(17, 2);

      ds->date_box = GPoint(57, 116);
      ds->date_single = GPoint(67, 118);
      ds->day = GPoint(84, 109);
      if (!is_digital()) {
        ds->date1 = GPoint(61, 120);
        ds->date2 = GPoint(73, 120);
      } else {
        ds->date1 = GPoint(118, 109);
        ds->date2 = GPoint(128, 109);
      }

      ds->digital_box = GPoint(28, 120);
      ds->digital_time1 = GPoint(37, 128);
      ds->digital_time2 = GPoint(59, 128);
      ds->digital_time3 = GPoint(85, 128);
      ds->digital_time4 = GPoint(107, 128);
      ds->digital_colon = GPoint(81, 132);

      break;
    case FONT2_ROUND:
    case FONT2_ROUND_DIGITAL:
      ds->markers[0] = GPoint(79, 5);
      ds->markers[1] = GPoint(118, 22);
      ds->markers[2] = GPoint(143, 47);
      ds->markers[3] = GPoint(152, 82);
      ds->markers[4] = GPoint(143, 117);
      ds->markers[5] = GPoint(118, 142);
      ds->markers[6] = GPoint(79, 161);
      ds->markers[7] = GPoint(40, 142);
      ds->markers[8] = GPoint(15, 117);
      ds->markers[9] = GPoint(6, 82);
      ds->markers[10] = GPoint(15, 47);
      ds->markers[11] = GPoint(40, 22);

      ds->date_box = GPoint(75, 117);
      ds->date_single = GPoint(85, 121);
      ds->day = GPoint(64, 98);
      if (!is_digital()) {
        ds->date1 = GPoint(79, 121);
        ds->date2 = GPoint(91, 121);
      } else {
        ds->date1 = GPoint(100, 98);
        ds->date2 = GPoint(108, 98);
      }

      ds->digital_box = GPoint(36, 109);
      ds->digital_time1 = GPoint(45, 117);
      ds->digital_time2 = GPoint(67, 117);
      ds->digital_time3 = GPoint(93, 117);
      ds->digital_time4 = GPoint(115, 117);
      ds->digital_colon = GPoint(89, 121);

      break;
    case FONT3:
    case FONT3_DIGITAL:
      ds->markers[0] = GPoint(61, 2);
      ds->markers[1] = GPoint(105, 2);
      ds->markers[2] = GPoint(120, 41);
      ds->markers[3] = GPoint(120, 76);
      ds->markers[4] = GPoint(120, 112);
      ds->markers[5] = GPoint(105, 150);
      ds->markers[6] = GPoint(61, 150);
      ds->markers[7] = GPoint(17, 150);
      ds->markers[8] = GPoint(2, 112);
      ds->markers[9] = GPoint(2, 76);
      ds->markers[10] = GPoint(2, 41);
      ds->markers[11] = GPoint(17, 2);

      ds->date_box = GPoint(57, 117);
      ds->date_single = GPoint(67, 120);
      ds->day = GPoint(83, 110);
      if (!is_digital()) {
        ds->date1 = GPoint(61, 120);
        ds->date2 = GPoint(73, 120);
      } else {
        ds->date1 = GPoint(118, 110);
        ds->date2 = GPoint(128, 110);
      }

      ds->digital_box = GPoint(28, 121);
      ds->digital_time1 = GPoint(37, 127);
      ds->digital_time2 = GPoint(59, 127);
      ds->digital_time3 = GPoint(85, 127);
      ds->digital_time4 = GPoint(107, 127);
      ds->digital_colon = GPoint(81, 131);

      break;
    case FONT3_ROUND:
    case FONT3_ROUND_DIGITAL:
      ds->markers[0] = GPoint(79, 5);
      ds->markers[1] = GPoint(118, 22);
      ds->markers[2] = GPoint(143, 47);
      ds->markers[3] = GPoint(152, 82);
      ds->markers[4] = GPoint(143, 117);
      ds->markers[5] = GPoint(118, 142);
      ds->markers[6] = GPoint(79, 161);
      ds->markers[7] = GPoint(40, 142);
      ds->markers[8] = GPoint(15, 117);
      ds->markers[9] = GPoint(6, 82);
      ds->markers[10] = GPoint(15, 47);
      ds->markers[11] = GPoint(40, 22);

      ds->date_box = GPoint(75, 118);
      ds->date_single = GPoint(85, 121);
      ds->day = GPoint(64, 99);
      if (!is_digital()) {
        ds->date1 = GPoint(79, 121);
        ds->date2 = GPoint(91, 121);
      } else {
        ds->date1 = GPoint(100, 99);
        ds->date2 = GPoint(108, 99);
      }

      ds->digital_box = GPoint(36, 110);
      ds->digital_time1 = GPoint(45, 116);
      ds->digital_time2 = GPoint(67, 116);
      ds->digital_time3 = GPoint(93, 116);
      ds->digital_time4 = GPoint(115, 116);
      ds->digital_colon = GPoint(89, 120);

      break;
    default:
      free(ds);
      break;
  }
  return ds;
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

static void update_date() {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Updating Date");
#endif

  binary_image_mask_data_clear_region(dial, GRect(ds->date1.x, ds->date1.y, ds->digit_size.w, ds->digit_size.h));
  binary_image_mask_data_clear_region(dial, GRect(ds->date2.x, ds->date2.y, ds->digit_size.w, ds->digit_size.h));
  int d1 = current_date / 10;
  int d2 = current_date % 10;
  binary_image_mask_data_draw(dial, digits, ds->date1, GRect(d1*ds->digit_size.w, 0, ds->digit_size.w, ds->digit_size.h));
  binary_image_mask_data_draw(dial, digits, ds->date2, GRect(d2*ds->digit_size.w, 0, ds->digit_size.w, ds->digit_size.h));

  if (is_digital()) {
    binary_image_mask_data_clear_region(dial, GRect(ds->day.x, ds->day.y, ds->day_size.w, ds->day_size.h));
    BinaryImageMaskData *day = binary_image_mask_data_create_from_resource(GSize(ds->day_size.w, ds->day_size.h * 7), ds->day_res);
    binary_image_mask_data_draw(dial, day, ds->day, GRect(0, current_day*ds->day_size.h, ds->day_size.w, ds->day_size.h));
    binary_image_mask_data_destroy(day);
  }
}

static int get_moonphase_index() {
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

  return index;
}

static void update_moonphase() {
  BinaryImageMaskData *moonphases = binary_image_mask_data_create_from_resource(GSize(ds->moonphase_size.w, ds->moonphase_size.h * 20), ds->moonphase_res);
  binary_image_mask_data_clear_region(dial, GRect(ds->moonphase.x, ds->moonphase.y, ds->moonphase_size.w, ds->moonphase_size.h));
#ifdef MOONPHASE
  int phase = MOONPHASE;
#else
  int phase = get_moonphase_index();
#endif
  binary_image_mask_data_draw(dial, moonphases, ds->moonphase, GRect(0, phase*ds->moonphase_size.h, ds->moonphase_size.w, ds->moonphase_size.h));
  binary_image_mask_data_destroy(moonphases);
}

static void update_digital_time(struct tm *tick_time) {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Updating Digital Time");
#endif
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time1.x, ds->digital_time1.y, ds->digit_big_size.w, ds->digit_big_size.h));
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time2.x, ds->digital_time2.y, ds->digit_big_size.w, ds->digit_big_size.h));
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time3.x, ds->digital_time3.y, ds->digit_big_size.w, ds->digit_big_size.h));
  binary_image_mask_data_clear_region(dial, GRect(ds->digital_time4.x, ds->digital_time4.y, ds->digit_big_size.w, ds->digit_big_size.h));

#ifdef QUICKTEST
  int seconds = tick_time->tm_sec;
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


  binary_image_mask_data_draw(dial, digits_big, ds->digital_time1, GRect(h1*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h));
  binary_image_mask_data_draw(dial, digits_big, ds->digital_time2, GRect(h2*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h));
  binary_image_mask_data_draw(dial, digits_big, ds->digital_time3, GRect(m1*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h));
  binary_image_mask_data_draw(dial, digits_big, ds->digital_time4, GRect(m2*ds->digit_big_size.w, 0, ds->digit_big_size.w, ds->digit_big_size.h));
}

static void unobstructed_change_handler(AnimationProgress progress, void *context) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect fullscreen = layer_get_frame(window_layer);
  GRect obstructed = layer_get_unobstructed_bounds(window_layer);

  int offset = (fullscreen.size.h - obstructed.size.h);
  if (!is_digital()) {
    offset = offset / 2;
  }
  offset_y = -offset;
  fullscreen.origin.y = 0 - offset;
  layer_set_frame(window_layer, fullscreen);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);

#ifdef LOGTIME
  int seconds = tick_time->tm_sec;
  int minutes = tick_time->tm_min;
  int hours = tick_time->tm_hour;
  APP_LOG(APP_LOG_LEVEL_INFO, "Time: %d:%d:%d", hours, minutes, seconds);
#endif

  if (settings.EnableDate) {
    int date = tick_time->tm_mday;
    if (current_date != date) {
#ifdef DATE
      current_date = DATE;
#else
      current_date = date;
#endif

#ifdef DAY
      current_day = DAY;
#else
      current_day = tick_time->tm_wday;
#endif
      update_date();
    }
  }

  if (settings.DigitalWatch) {
    update_digital_time(tick_time);
  }

  if (settings.EnableMoonphase) {
    update_moonphase();
  }

  layer_mark_dirty(s_canvas_layer);
  layer_mark_dirty(s_bg_layer);
}


static void draw_fancy_hand(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
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
}

static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1 = polar_to_point_offset(origin, angle + 180, back_length);
  GPoint p2 = polar_to_point_offset(origin, angle, length);
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, p1, p2);

#ifdef PBL_BW
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, p1, p2);
#endif
}

static void draw_center(GContext *ctx, GColor minutes_border, GColor minutes_color, GColor seconds_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_fill_color(ctx, minutes_border);
  graphics_fill_circle(ctx, origin, 6);
  graphics_context_set_fill_color(ctx, seconds_color);
  graphics_fill_circle(ctx, origin, 4);
  graphics_context_set_fill_color(ctx, minutes_color);
  graphics_fill_circle(ctx, origin, 2);
}


static void canvas_update_proc(Layer *layer, GContext *ctx) {
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
    int minutes_angle = ((float)minutes / 60 * 360) + ((float)seconds / 60 * 360 / 60) - 90;
#ifdef PBL_COLOR
    draw_fancy_hand(ctx, minutes_angle, bounds.size.w / 2 - 10, settings.MinutesHandColor, settings.MinutesHandBorderColor);
#else
    draw_fancy_hand(ctx, minutes_angle, bounds.size.w / 2 - 10, GColorWhite, GColorBlack);
#endif

    int hours_angle = ((float)hours / 12 * 360) + ((float)minutes / 60 * 360 / 12) + ((float)seconds / 60 * 360 / 60 / 12)  - 90;

#ifdef PBL_COLOR
    draw_fancy_hand(ctx, hours_angle, bounds.size.w / 2 - 30, settings.HoursHandColor, settings.HoursHandBorderColor);
#else
    draw_fancy_hand(ctx, hours_angle, bounds.size.w / 2 - 30, GColorWhite, GColorBlack);
#endif

    if (settings.EnableSecondsHand) {
      int seconds_angle = ((float)seconds / 60 * 360) - 90;
#ifdef PBL_COLOR
      draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 5, 15, settings.SecondsHandColor);
#else
      draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 5, 15, GColorBlack);
#endif
    }

#ifdef PBL_COLOR
    draw_center(ctx, settings.HoursHandBorderColor, settings.HoursHandColor, settings.SecondsHandColor);
#else
    draw_center(ctx, GColorBlack, GColorWhite, GColorBlack);
#endif
  }
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


static void bg_update_proc(Layer *layer, GContext *ctx) {
#ifdef LOG
  time_t start_sec, end_sec;
  uint16_t start_ms, end_ms;
  time_ms(&start_sec, &start_ms);
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

  int angle = 360 -settings.Angle + 90;
  if (!settings.FixedAngle) {
    angle = 360 - ((float)minutes / 60 * 360) - ((float)seconds / 60 * 360 / 60) + 90;
  }

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p = polar_to_point_offset(origin, angle, bounds.size.h);
  bool is_vertical = false;

  if (p.x == origin.x) {
    is_vertical = true;
  }

  float m = (is_vertical) ? 0 : -slope_from_two_points(origin, p);
  int b = (origin.y + offset_y - m * origin.x) + 0.5;

  for(int y = max(0, offset_y); y < bounds.size.h + offset_y; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);

    for(int x = info.min_x; x <= info.max_x; x++) {
      int line_y = m * x + b;
      int half_width = bounds.size.w / 2;

      GColor color = settings.BackgroundColor1;
      bool is_in_bg1 = true;
      if (!is_vertical && minutes < 30 && y <= line_y) {
        is_in_bg1 = false;
      } else if (!is_vertical && minutes >= 30 && y >= line_y) {
        is_in_bg1 = false;
      } else if (is_vertical && (minutes >= 59 || minutes <= 1) && x <= half_width) {
        is_in_bg1 = false;
      } else if (is_vertical && (minutes >= 29 && minutes <= 31) && x >= half_width) {
        is_in_bg1 = false;
      }

      bool is_in_dial = binary_image_mask_data_get_pixel(dial, x, y - offset_y);

      if (is_in_bg1 && is_in_dial) {
#ifdef PBL_COLOR
        color = settings.TextColor1;
#else
        color = settings.BWTextColor1;
#endif
      }
      else if (!is_in_bg1 && is_in_dial) {
#ifdef PBL_COLOR
        color = settings.TextColor2;
#else
        color = settings.BWTextColor2;
#endif
      }
      else if (is_in_bg1) {
#ifdef PBL_COLOR
        color = settings.BackgroundColor1;
#else
        color = settings.BWBackgroundColor1;
#endif
      }
      else if (!is_in_bg1) {
        //if ((y * bounds.size.w + x) % 5 == 0) {
          //color = bg_color1;
        //} else {
          //color = bg_color2;
        //}
#ifdef PBL_COLOR
        color = settings.BackgroundColor2;
#else
        color = settings.BWBackgroundColor2;
#endif
      }

      set_pixel_color(info, GPoint(x, y), color);
    }
  }
  graphics_release_frame_buffer(ctx, fb);

#ifdef LOG
  time_ms(&end_sec, &end_ms);
  int elapsed_ms = (int)((end_sec - start_sec) * 1000 + (end_ms - start_ms));
  APP_LOG(APP_LOG_LEVEL_INFO, "Time to update BG: %d ms", elapsed_ms);
#endif
}

static enum DialType get_dial_type() {
  switch (settings.Font) {
    case 1:
      if (!settings.DigitalWatch) {
#ifdef PBL_ROUND
        return FONT1_ROUND;
#else
        return FONT1;
#endif
      } else {
#ifdef PBL_ROUND
        return FONT1_ROUND_DIGITAL;
#else
        return FONT1_DIGITAL;
#endif
      }
    case 2:
      if (!settings.DigitalWatch) {
#ifdef PBL_ROUND
      return FONT2_ROUND;
#else
      return FONT2;
#endif
      } else {
#ifdef PBL_ROUND
      return FONT2_ROUND_DIGITAL;
#else
      return FONT2_DIGITAL;
#endif
      }
    case 3:
      if (!settings.DigitalWatch) {
#ifdef PBL_ROUND
      return FONT3_ROUND;
#else
      return FONT3;
#endif
      } else {
#ifdef PBL_ROUND
      return FONT3_ROUND_DIGITAL;
#else
      return FONT3_DIGITAL;
#endif
      }
    default:
      if (!settings.DigitalWatch) {
#ifdef PBL_ROUND
      return FONT1_ROUND;
#else
      return FONT1;
#endif
      } else {
#ifdef PBL_ROUND
      return FONT1_ROUND_DIGITAL;
#else
      return FONT1_DIGITAL;
#endif
      }
  }
}

static void draw_dial() {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing dial");
#endif
  binary_image_mask_data_destroy(dial);
  binary_image_mask_data_destroy(digits);
  binary_image_mask_data_destroy(digits_big);

  free(ds);
  ds = get_dial_spec(get_dial_type());

  dial = binary_image_mask_data_create(bounds.size);
  digits = binary_image_mask_data_create_from_resource(GSize(ds->digit_size.w * 10, ds->digit_size.h), ds->digit_res);

  if (!settings.DigitalWatch) {
    BinaryImageMaskData *markers = binary_image_mask_data_create_from_resource(GSize(ds->marker_size.w, ds->marker_size.h * 12), ds->marker_res);
    for (int i = 0; i < 12; i++) {
      binary_image_mask_data_draw(dial, markers, ds->markers[i], GRect(0, ds->marker_size.h * i, ds->marker_size.w, ds->marker_size.h));
    }
    binary_image_mask_data_destroy(markers);
  }
  else {
    digits_big = binary_image_mask_data_create_from_resource(GSize(ds->digit_big_size.w * 10, ds->digit_big_size.h), ds->digit_big_res);
  }


  if (settings.EnablePebbleLogo) {
    BinaryImageMaskData *logo = binary_image_mask_data_create_from_resource(ds->logo_size, ds->logo_res);
    binary_image_mask_data_draw(dial, logo, ds->logo, GRect(0, 0, ds->logo_size.w, ds->logo_size.h));
    binary_image_mask_data_destroy(logo);
  }

  // TODO: Setting to force model
  if (settings.EnableWatchModel) {
    BinaryImageMaskData *models = binary_image_mask_data_create_from_resource(GSize(ds->model_size.w, ds->model_size.h * MODEL_COUNT), ds->models_res);
    int index = 5;
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
      default:
        index = 8;
        break;
    }
    binary_image_mask_data_draw(dial, models, ds->model, GRect(0, ds->model_size.h * index, ds->model_size.w, ds->model_size.h));
    binary_image_mask_data_destroy(models);
  }

  if (settings.EnableDate && !settings.DigitalWatch) {
    BinaryImageMaskData *date_box = binary_image_mask_data_create_from_resource(ds->date_box_size, ds->date_box_res);
    binary_image_mask_data_draw(dial, date_box, ds->date_box, GRect(0, 0, ds->date_box_size.w, ds->date_box_size.h));
    binary_image_mask_data_destroy(date_box);
  }

  if (settings.EnableDate) {
    update_date();
  }

  if (settings.DigitalWatch) {
    BinaryImageMaskData *digital_box = binary_image_mask_data_create_from_resource(ds->digital_box_size, ds->digital_box_res);
    binary_image_mask_data_draw(dial, digital_box, ds->digital_box, GRect(0, 0, ds->digital_box_size.w, ds->digital_box_size.h));
    binary_image_mask_data_destroy(digital_box);

    BinaryImageMaskData *digital_colon = binary_image_mask_data_create_from_resource(ds->digital_colon_size, ds->digital_colon_res);
    binary_image_mask_data_draw(dial, digital_colon, ds->digital_colon, GRect(0, 0, ds->digital_colon_size.w, ds->digital_colon_size.h));
    binary_image_mask_data_destroy(digital_colon);

    update_digital_time(prv_tick_time);
  }
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

  if (settings.EnableSecondsHand && !settings.DigitalWatch) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }

  s_bg_layer = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  layer_add_child(window_layer, s_bg_layer);
  layer_add_child(window_layer, s_canvas_layer);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_update_proc(s_bg_layer, bg_update_proc);

  unobstructed_change_handler(100, NULL);

  UnobstructedAreaHandlers unobstructed_handlers = {
    .change = unobstructed_change_handler
  };

  unobstructed_area_service_subscribe(unobstructed_handlers, NULL);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  binary_image_mask_data_destroy(dial);
  binary_image_mask_data_destroy(digits);
  binary_image_mask_data_destroy(digits_big);
  free(ds);
}

static void prv_init(void) {
  prv_load_settings();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 0);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);
#endif

  app_event_loop();
  prv_deinit();
}
