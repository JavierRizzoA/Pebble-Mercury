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
static int current_date = -1;
static ClaySettings settings;
static DialSpec *ds;

//#define LOG
//#define LOGTIME
//#define QUICKTEST

//#define HOUR 10
//#define MINUTE 10
//#define SECOND 30


static void prv_save_settings(void);
static void prv_default_settings(void);
static void prv_load_settings(void);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void update_date();
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

static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_default_settings(void) {
  settings.EnableSecondsHand = true;
  settings.EnableDate = true;
  settings.EnablePebbleLogo = true;
  settings.EnableWatchModel = true;
  settings.DigitalWatch = false;
  settings.BackgroundColor1 = GColorCobaltBlue;
  settings.BackgroundColor2 = GColorPastelYellow;
  settings.TextColor1 = GColorWhite;
  settings.TextColor2 = GColorOrange;
  settings.HoursHandColor = GColorWhite;
  settings.HoursHandBorderColor = GColorBlack;
  settings.MinutesHandColor = GColorWhite;
  settings.MinutesHandBorderColor = GColorBlack;
  settings.SecondsHandColor = GColorOrange;
  settings.BWBackgroundColor1 = GColorWhite;
  settings.BWBackgroundColor2 = GColorBlack;
  settings.BWTextColor1 = GColorBlack;
  settings.BWTextColor2 = GColorWhite;
  settings.Font = 1;
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

  if(enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;
    tick_timer_service_unsubscribe();
    if (settings.EnableSecondsHand) {
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

  prv_save_settings();
  draw_dial();
  layer_mark_dirty(s_bg_layer);
}



DialSpec* get_dial_spec(enum DialType dial_type) {
  DialSpec *ds = (DialSpec*) malloc(sizeof(DialSpec));

  ds->logo_size = GSize(38, 12);
  ds->model_size = GSize(71, 5);
  ds->logo_res = RESOURCE_ID_LOGO;
  ds->models_res = RESOURCE_ID_MODELS;

  if (dial_type == FONT1 || dial_type == FONT2 || dial_type == FONT3) {
    ds->logo = GPoint(53, 27);
    ds->model = GPoint(36, 40);
  }
  else if (dial_type == FONT1_ROUND || dial_type == FONT2_ROUND || dial_type == FONT3_ROUND) {
    ds->logo = GPoint(71, 39);
    ds->model = GPoint(54, 52);
  } else {
    free(ds);
    return ds;
  }

  if (dial_type == FONT1 || dial_type == FONT1_ROUND) {
    ds->date_box_size = GSize(34, 22);
    ds->digit_size = GSize(10, 14);
    ds->marker_size = GSize(22, 19);

    ds->date_box_res = RESOURCE_ID_DATE_BOX1;
    ds->digit_res = RESOURCE_ID_DIGITS1;
    ds->marker_res = RESOURCE_ID_MARKERS1;
  } else if (dial_type == FONT2 || dial_type == FONT2_ROUND) {
    ds->date_box_size = GSize(30, 18);
    ds->digit_size = GSize(10, 10);
    ds->marker_size = GSize(22, 15);

    ds->date_box_res = RESOURCE_ID_DATE_BOX2;
    ds->digit_res = RESOURCE_ID_DIGITS2;
    ds->marker_res = RESOURCE_ID_MARKERS2;
  } else if (dial_type == FONT3 || dial_type == FONT3_ROUND) {
    ds->date_box_size = GSize(30, 17);
    ds->digit_size = GSize(10, 11);
    ds->marker_size = GSize(26, 16);

    ds->date_box_res = RESOURCE_ID_DATE_BOX3;
    ds->digit_res = RESOURCE_ID_DIGITS3;
    ds->marker_res = RESOURCE_ID_MARKERS3;
  } else {
    free(ds);
    return ds;
  }

  switch (dial_type) {
    case FONT1:
      ds->markers[0] = GPoint(61, 1);
      ds->markers[1] = GPoint(105, 1);
      ds->markers[2] = GPoint(121, 39);
      ds->markers[3] = GPoint(121, 74);
      ds->markers[4] = GPoint(121, 110);
      ds->markers[5] = GPoint(105, 147);
      ds->markers[6] = GPoint(61, 147);
      ds->markers[7] = GPoint(17, 147);
      ds->markers[8] = GPoint(1, 110);
      ds->markers[9] = GPoint(1, 74);
      ds->markers[10] = GPoint(1, 39);
      ds->markers[11] = GPoint(17, 1);

      ds->date_box = GPoint(55, 114);
      ds->date1 = GPoint(61, 118);
      ds->date2 = GPoint(73, 118);
      ds->date_single = GPoint(67, 118);

      break;
    case FONT1_ROUND:
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
      ds->date1 = GPoint(79, 119);
      ds->date2 = GPoint(91, 119);
      ds->date_single = GPoint(85, 119);

      break;
    case FONT2:
      ds->markers[0] = GPoint(61, 1);
      ds->markers[1] = GPoint(105, 1);
      ds->markers[2] = GPoint(121, 41);
      ds->markers[3] = GPoint(121, 76);
      ds->markers[4] = GPoint(121, 112);
      ds->markers[5] = GPoint(105, 151);
      ds->markers[6] = GPoint(61, 151);
      ds->markers[7] = GPoint(17, 151);
      ds->markers[8] = GPoint(1, 112);
      ds->markers[9] = GPoint(1, 76);
      ds->markers[10] = GPoint(1, 41);
      ds->markers[11] = GPoint(17, 1);

      ds->date_box = GPoint(57, 116);
      ds->date1 = GPoint(61, 120);
      ds->date2 = GPoint(73, 120);
      ds->date_single = GPoint(67, 118);

      break;
    case FONT2_ROUND:
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
      ds->date1 = GPoint(79, 121);
      ds->date2 = GPoint(91, 121);
      ds->date_single = GPoint(85, 121);

      break;
    case FONT3:
      ds->markers[0] = GPoint(61, 1);
      ds->markers[1] = GPoint(105, 1);
      ds->markers[2] = GPoint(121, 41);
      ds->markers[3] = GPoint(121, 76);
      ds->markers[4] = GPoint(121, 112);
      ds->markers[5] = GPoint(105, 151);
      ds->markers[6] = GPoint(61, 151);
      ds->markers[7] = GPoint(17, 151);
      ds->markers[8] = GPoint(1, 112);
      ds->markers[9] = GPoint(1, 76);
      ds->markers[10] = GPoint(1, 41);
      ds->markers[11] = GPoint(17, 1);

      ds->date_box = GPoint(57, 117);
      ds->date1 = GPoint(61, 120);
      ds->date2 = GPoint(73, 120);
      ds->date_single = GPoint(67, 120);

      break;
    case FONT3_ROUND:
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
      ds->date1 = GPoint(79, 121);
      ds->date2 = GPoint(91, 121);
      ds->date_single = GPoint(85, 121);

      break;
    default:
      free(ds);
      break;
  }
  return ds;
}

static void update_date() {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Updating Date");
#endif

  binary_image_mask_data_clear_region(dial, GRect(ds->date1.x, ds->date1.y, ds->date2.x + ds->digit_size.w - ds->date1.x, ds->date2.y + ds->digit_size.h - ds->date1.y));
  int d1 = current_date / 10;
  int d2 = current_date % 10;
  binary_image_mask_data_draw(dial, digits, ds->date1, GRect(d1*ds->digit_size.w, 0, ds->digit_size.w, ds->digit_size.h));
  binary_image_mask_data_draw(dial, digits, ds->date2, GRect(d2*ds->digit_size.w, 0, ds->digit_size.w, ds->digit_size.h));
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
      current_date = date;
      update_date();
    }
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
#endif

  int minutes_angle = ((double)minutes / 60 * 360) + ((double)seconds / 60 * 360 / 60) - 90;
#ifdef PBL_COLOR
  draw_fancy_hand(ctx, minutes_angle, bounds.size.w / 2 - 10, settings.MinutesHandColor, settings.MinutesHandBorderColor);
#else
  draw_fancy_hand(ctx, minutes_angle, bounds.size.w / 2 - 10, GColorWhite, GColorBlack);
#endif

  int hours_angle = ((double)hours / 12 * 360) + ((double)minutes / 60 * 360 / 12) + ((double)seconds / 60 * 360 / 60 / 12)  - 90;

#ifdef PBL_COLOR
  draw_fancy_hand(ctx, hours_angle, bounds.size.w / 2 - 30, settings.HoursHandColor, settings.HoursHandBorderColor);
#else
  draw_fancy_hand(ctx, hours_angle, bounds.size.w / 2 - 30, GColorWhite, GColorBlack);
#endif

  if (settings.EnableSecondsHand) {
    int seconds_angle = ((double)seconds / 60 * 360) - 90;
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


static void bg_update_proc(Layer *layer, GContext *ctx) {
  int minutes = prv_tick_time->tm_min;
  int seconds = prv_tick_time->tm_sec;

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
#endif

  int angle = 360 - ((double)minutes / 60 * 360) - ((double)seconds / 60 * 360 / 60) + 90;
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p = polar_to_point_offset(origin, angle, bounds.size.h);
  bool is_vertical = false;

  if (p.x == origin.x) {
    is_vertical = true;
  }

  double m = (is_vertical) ? 0 : -slope_from_two_points(origin, p);
  int b = (origin.y - m * origin.x) + 0.5;

  for(int x = 0; x < bounds.size.w; x++) {
    for(int y = 0; y < bounds.size.h; y++) {
      GColor color = settings.BackgroundColor1;
      bool is_in_bg1 = true;
      if (is_vertical && (minutes >= 59 || minutes <= 1) && x <= bounds.size.w / 2) {
        is_in_bg1 = false;
      }
      if (is_vertical && (minutes >= 29 && minutes <= 31) && x >= bounds.size.w / 2) {
        is_in_bg1 = false;
      }
      if (!is_vertical && minutes < 30 && y <= m*x + b) {
        is_in_bg1 = false;
      }
      if (!is_vertical && minutes >= 30 && y >= m*x + b) {
        is_in_bg1 = false;
      }

      bool is_in_dial = binary_image_mask_data_get_pixel(dial, x, y);

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

      graphics_context_set_stroke_color(ctx, color);
      graphics_draw_pixel(ctx, GPoint(x, y));
    }
  }
}

static enum DialType get_dial_type() {
  switch (settings.Font) {
    case 1:
#ifdef PBL_ROUND
      return FONT1_ROUND;
#else
      return FONT1;
#endif
    case 2:
#ifdef PBL_ROUND
      return FONT2_ROUND;
#else
      return FONT2;
#endif
    case 3:
#ifdef PBL_ROUND
      return FONT3_ROUND;
#else
      return FONT3;
#endif
    default:
#ifdef PBL_ROUND
      return FONT1_ROUND;
#else
      return FONT1;
#endif
  }
}

static void draw_dial() {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing dial");
#endif
  binary_image_mask_data_destroy(dial);
  binary_image_mask_data_destroy(digits);

  free(ds);
  ds = get_dial_spec(get_dial_type());

  dial = binary_image_mask_data_create(bounds.size);
  digits = binary_image_mask_data_create_from_resource(GSize(ds->digit_size.w * 10, ds->digit_size.h), ds->digit_res);


  BinaryImageMaskData *markers = binary_image_mask_data_create_from_resource(GSize(ds->marker_size.w, ds->marker_size.h * 12), ds->marker_res);
  for (int i = 0; i < 12; i++) {
    binary_image_mask_data_draw(dial, markers, ds->markers[i], GRect(0, ds->marker_size.h * i, ds->marker_size.w, ds->marker_size.h));
  }
  binary_image_mask_data_destroy(markers);


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
      default:
        index = 8;
        break;
    }
    binary_image_mask_data_draw(dial, models, ds->model, GRect(0, ds->model_size.h * index, ds->model_size.w, ds->model_size.h));
    binary_image_mask_data_destroy(models);
  }

  if (settings.EnableDate) {
    BinaryImageMaskData *date_box = binary_image_mask_data_create_from_resource(ds->date_box_size, ds->date_box_res);
    binary_image_mask_data_draw(dial, date_box, ds->date_box, GRect(0, 0, ds->date_box_size.w, ds->date_box_size.h));
    binary_image_mask_data_destroy(date_box);
  }

  if (settings.EnableDate) {
    update_date();
  }
}

static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);
  current_date = prv_tick_time->tm_mday;
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  draw_dial();

  if (settings.EnableSecondsHand) {
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
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  binary_image_mask_data_destroy(dial);
  binary_image_mask_data_destroy(digits);
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
