#include <pebble.h>

// Quick integer square root.
uint32_t isqrt(uint32_t n) {
  if (n == 0) {
    return 0;
  }
  uint32_t root = 0;
  uint32_t bit = 1 << 30;

  while (bit > n) {
    bit >>= 2;
  }

  while (bit != 0) {
    if (n >= root + bit) {
      n -= root + bit;
      root += 2 * bit;
    }
    root >>= 1;
    bit >>= 2;
  }
  return root;
}

// Distance between two points.
uint32_t two_point_distance(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
  return isqrt(((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2-y1)));
}

GPoint add_points(GPoint a, GPoint b) {
  return GPoint(a.x + b.x, a.y + b.y);
}

// Converts an angle and a distance to a cartesian point.
GPoint polar_to_point(int angle, int distance) {
  int x = distance * ((double)cos_lookup(DEG_TO_TRIGANGLE(angle)) / ((double)TRIG_MAX_ANGLE));
  int y = distance * ((double)sin_lookup(DEG_TO_TRIGANGLE(angle)) / ((double)TRIG_MAX_ANGLE));
  return GPoint(x, y);
}

GPoint polar_to_point_offset(GPoint offset, int angle, int distance) {
  return add_points(offset, polar_to_point(angle, distance));
}

double slope_from_two_points(GPoint a, GPoint b) {
  return (double)(b.y - a.y) / (double)(b.x - a.x);
}
