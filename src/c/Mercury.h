#pragma once
#include <pebble.h>

#define MODEL_COUNT 9
#define SETTINGS_KEY 1

typedef struct DialSpec {
  GPoint markers[12];
  GPoint logo;
  GPoint model;
  GPoint date_box;
  GPoint date1;
  GPoint date2;
  GPoint date_single;

  GSize marker_size;
  GSize digit_size;
  GSize logo_size;
  GSize model_size;
  GSize date_box_size;

  uint32_t marker_res;
  uint32_t digit_res;
  uint32_t logo_res;
  uint32_t models_res;
  uint32_t date_box_res;
} __attribute__((__packed__)) DialSpec;

enum DialType {
  FONT1,
  FONT2,
  FONT3,
  FONT1_ROUND,
  FONT2_ROUND,
  FONT3_ROUND
};

typedef struct ClaySettings {
  bool EnableSecondsHand;
  bool EnableDate;
  bool EnablePebbleLogo;
  bool EnableWatchModel;
  bool DigitalWatch;
  GColor BackgroundColor1;
  GColor BackgroundColor2;
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
} __attribute__((__packed__)) ClaySettings;
