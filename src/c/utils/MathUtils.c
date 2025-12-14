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
  int x = distance * ((float)cos_lookup(DEG_TO_TRIGANGLE(angle)) / ((float)TRIG_MAX_ANGLE));
  int y = distance * ((float)sin_lookup(DEG_TO_TRIGANGLE(angle)) / ((float)TRIG_MAX_ANGLE));
  return GPoint(x, y);
}

GPoint polar_to_point_offset(GPoint offset, int angle, int distance) {
  return add_points(offset, polar_to_point(angle, distance));
}

float slope_from_two_points(GPoint a, GPoint b) {
  return (float)(b.y - a.y) / (float)(b.x - a.x);
}

int min(int a, int b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

int max(int a, int b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}

double fmod(double a, double b) {
    int quotient = (int)(a / b);
    double result = a - (quotient * b);
    if (result < 0) result += b;
    return result;
}

float sine(int32_t angle) {
  return ((float)sin_lookup(DEG_TO_TRIGANGLE(angle)) / ((float)TRIG_MAX_ANGLE));
}

float cosine(int32_t angle) {
  return ((float)cos_lookup(DEG_TO_TRIGANGLE(angle)) / ((float)TRIG_MAX_ANGLE));
}

float tangent(int32_t angle) {
  return sine(angle) / cosine(angle);
}
