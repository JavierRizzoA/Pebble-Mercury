#include "MathUtils.h"
#include <pebble.h>
#include <math.h>

// Quick integer square root.
uint32_t isqrt(uint32_t n) {
  uint32_t root = 0;
  uint32_t bit = 1u << 30; // highest power of 4 <= 2^32

  while (bit > n) bit >>= 2;

  while (bit) {
    if (n >= root + bit) {
      n -= root + bit;
      root += bit << 1;
    }
    root >>= 1;
    bit >>= 2;
  }
  return root;
}

// Distance between two points.
uint32_t two_point_distance(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
  int dx = (int)x2 - (int)x1;
  int dy = (int)y2 - (int)y1;
  return isqrt((uint32_t)dx * dx + (uint32_t)dy * dy);
}

GPoint add_points(GPoint a, GPoint b) {
  return GPoint(a.x + b.x, a.y + b.y);
}

// Converts an angle and a distance to a cartesian point.
GPoint polar_to_point(int angle, int distance) {
  int x = (int)(distance * cos_lookup(DEG_TO_TRIGANGLE(angle)) * TRIG_SCALE);
  int y = (int)(distance * sin_lookup(DEG_TO_TRIGANGLE(angle)) * TRIG_SCALE);
  return GPoint(x, y);
}

GPoint polar_to_point_offset(GPoint offset, int angle, int distance) {
  return add_points(offset, polar_to_point(angle, distance));
}

float slope_from_two_points(GPoint a, GPoint b) {
  int dx = b.x - a.x;
  if (dx == 0) return INFINITY;
  return (float)(b.y - a.y) / (float)dx;
}

double fmod(double a, double b) {
  int quotient = (int)(a / b);
  double result = a - (quotient * b);
  if (result < 0) result += b;
  return result;
}

float sine(int32_t angle)   {
  return sin_lookup(DEG_TO_TRIGANGLE(angle)) * TRIG_SCALE;
}

float cosine(int32_t angle) {
  return cos_lookup(DEG_TO_TRIGANGLE(angle)) * TRIG_SCALE;
}

float tangent(int32_t angle){
  return sine(angle) / cosine(angle);
}
