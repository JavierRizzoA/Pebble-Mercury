#include <pebble.h>
#include "BinaryImageMaskData.h"
#include "MathUtils.h"

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size);
void binary_image_mask_data_destroy(BinaryImageMaskData* bimd);
struct BinaryImageMaskData* binary_image_mask_data_create_from_resource(GSize size, uint32_t resource_id);
void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val);
void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region, bool centered);
bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y);
void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg, bool centered);
bool binary_image_mask_data_is_filled_or_adjacent(BinaryImageMaskData* bimd, int x, int y, int margin);

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size) {
  BinaryImageMaskData *bimd = (BinaryImageMaskData*) malloc(sizeof(BinaryImageMaskData));
  bimd->size = size;
  bimd->data = (uint8_t*)calloc(((size.w * size.h) + 7) >> 3, 1);
  return bimd;
}

void binary_image_mask_data_destroy(BinaryImageMaskData* bimd) {
  if (bimd != NULL) {
    if (bimd->data != NULL) {
      free(bimd->data);
      bimd->data = NULL;
    }
    free(bimd);
    bimd = NULL;
  }
}

struct BinaryImageMaskData* binary_image_mask_data_create_from_resource(GSize size, uint32_t resource_id) {
  BinaryImageMaskData *bimd = binary_image_mask_data_create(size);
  ResHandle handle = resource_get_handle(resource_id);
  resource_load(handle, bimd->data, ((bimd->size.w * bimd->size.h) + 7) >> 3);
  return bimd;
}

void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val) {
  size_t index = (size_t)bimd->size.w * y + x;
  uint8_t *byte_ptr = &bimd->data[index >> 3];
  uint8_t mask = 1 << (index & 7);

  if (val) {
    *byte_ptr |= mask;   // set bit
  } else {
    *byte_ptr &= ~mask;  // clear bit
  }
}

void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region, bool centered) {
  GPoint offset = centered ? GPoint(region.size.w / 2, region.size.h / 2) : GPoint(0, 0);

  int start_x = region.origin.x - offset.x;
  int start_y = region.origin.y - offset.y;
  int end_x = start_x + region.size.w;
  int end_y = start_y + region.size.h;

  // Clamp to image bounds
  if (start_x < 0) start_x = 0;
  if (start_y < 0) start_y = 0;
  if (end_x > bimd->size.w) end_x = bimd->size.w;
  if (end_y > bimd->size.h) end_y = bimd->size.h;

  for (int y = start_y; y < end_y; y++) {
    size_t row_start = (size_t)bimd->size.w * y + start_x;
    size_t row_end = (size_t)bimd->size.w * y + end_x;

    // If clearing full bytes, do it in chunks
    size_t byte_start = row_start >> 3;
    size_t byte_end = row_end >> 3;

    if ((row_start & 7) == 0 && (row_end & 7) == 0) {
      // Perfect byte alignment: clear entire bytes
      memset(&bimd->data[byte_start], 0, byte_end - byte_start);
    } else {
      // Fallback: clear bit-by-bit for partial bytes
      for (int x = start_x; x < end_x; x++) {
        binary_image_mask_data_set_bit(bimd, x, y, false);
      }
    }
  }
}

bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y) {
  size_t index = bimd->size.w * y + x;
  uint8_t byte = bimd->data[index >> 3];
  return ((byte >> (index & 7)) & 1);
}

bool binary_image_mask_data_is_filled_or_adjacent(BinaryImageMaskData* bimd, int x, int y, int margin) {
  for (int yy = MAX(y - margin, 0); yy <= MIN(y + margin, bimd->size.h - 1); yy++) {
    for (int xx = MAX(x - margin, 0); xx <= MIN(x + margin, bimd->size.w - 1); xx++) {
      if (binary_image_mask_data_get_pixel(bimd, xx, yy)) {
        return true;
      }
    }
  }
  return false;
}

void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg, bool centered) {
  GPoint offset = centered ? GPoint(src_reg.size.w / 2, src_reg.size.h / 2) : GPoint(0, 0);

  int start_x = src_reg.origin.x;
  int start_y = src_reg.origin.y;
  int end_x = start_x + src_reg.size.w;
  int end_y = start_y + src_reg.size.h;

  for (int y = start_y; y < end_y; y++) {
    int dest_y = dest_pos.y + (y - start_y) - offset.y;
    if (dest_y < 0 || dest_y >= dest->size.h) continue;

    for (int x = start_x; x < end_x; x++) {
      int dest_x = dest_pos.x + (x - start_x) - offset.x;
      if (dest_x < 0 || dest_x >= dest->size.w) continue;

      if (binary_image_mask_data_get_pixel(src, x, y)) {
        binary_image_mask_data_set_bit(dest, dest_x, dest_y, true);
      }
    }
  }
}
