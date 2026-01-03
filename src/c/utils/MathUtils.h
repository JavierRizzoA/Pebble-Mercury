#pragma once
#include <pebble.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define TRIG_SCALE (1.0f / (float)TRIG_MAX_ANGLE)

uint32_t two_point_distance(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);

uint32_t isqrt(uint32_t n);

GPoint polar_to_point(int angle, int distance);

GPoint add_points(GPoint a, GPoint b);

GPoint polar_to_point_offset(GPoint offset, int angle, int distance);

float slope_from_two_points(GPoint a, GPoint b);

double fmod(double a, double b);

float sine(int32_t angle);

float cosine(int32_t angle);

float tangent(int32_t angle);
