#pragma once
#include <pebble.h>

#define MODEL_COUNT 11
#define SETTINGS_KEY 1

enum BackgroundPattern {
  SOLID = 0,
  DITHER_STRONG = 1,
  DITHER_LIGHT = 2,
  VERTICAL_STRONG = 3,
  VERTICAL_LIGHT = 4,
  HORIZONTAL_STRONG = 5,
  HORIZONTAL_LIGHT = 6,
  DIAGONAL = 7,
  DIAGONAL_MIRROR = 8,
  CHECKERBOARD=9
};

typedef struct DrawnArea {
  struct DrawnArea *next;
  GPoint p1;
  GPoint p2;
} DrawnArea;

typedef struct DialSpec {
  GPoint markers[12];
  GPoint logo;
  GPoint model;
  GPoint date_box;
  GPoint date1;
  GPoint date2;
  GPoint date_single;
  GPoint digital_box;
  GPoint digital_time1;
  GPoint digital_time2;
  GPoint digital_time3;
  GPoint digital_time4;
  GPoint digital_colon;
  GPoint day;
  GPoint moonphase;

  GSize marker_size;
  GSize digit_size;
  GSize logo_size;
  GSize model_size;
  GSize date_box_size;
  GSize digit_big_size;
  GSize digital_box_size;
  GSize digital_colon_size;
  GSize day_size;
  GSize moonphase_size;

  uint32_t marker_res;
  uint32_t digit_res;
  uint32_t logo_res;
  uint32_t models_res;
  uint32_t date_box_res;
  uint32_t digital_box_res;
  uint32_t digital_colon_res;
  uint32_t digit_big_res;
  uint32_t day_res;
  uint32_t moonphase_res;
} __attribute__((__packed__)) DialSpec;

typedef struct ClaySettings {
  bool EnableSecondsHand;
  bool EnableDate;
  bool EnablePebbleLogo;
  bool EnableWatchModel;
  int ForcedWatchModel;
  bool EnableMoonphase;
  bool DigitalWatch;
  int Font;
  bool FixedAngle;
  int Angle;
  GColor BackgroundColor1;
  GColor SecondaryBackgroundColor1;
  GColor BackgroundColor2;
  GColor SecondaryBackgroundColor2;
  bool NoPatternUnderText;
  enum BackgroundPattern BackgroundPattern1;
  enum BackgroundPattern BackgroundPattern2;
  GColor TextColor1;
  GColor TextColor2;
  GColor HoursHandColor;
  GColor HoursHandBorderColor;
  GColor MinutesHandColor;
  GColor MinutesHandBorderColor;
  GColor SecondsHandColor;
  GColor BWBackgroundColor1;
  GColor BWBackgroundColor2;
  GColor BWTextColor1;
  GColor BWTextColor2;
  GColor BWHoursHandColor;
  GColor BWHoursHandBorderColor;
  GColor BWMinutesHandColor;
  GColor BWMinutesHandBorderColor;
  GColor BWSecondsHandColor;
  GColor BWSecondsHandBorderColor;
} __attribute__((__packed__)) ClaySettings;
