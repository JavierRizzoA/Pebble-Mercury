#include <pebble.h>
#include "BinaryImageMaskData.h"

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size);
void binary_image_mask_data_destroy(BinaryImageMaskData* bimd);
struct BinaryImageMaskData* binary_image_mask_data_create_from_resource(GSize size, uint32_t resource_id);
void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val);
void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region);
bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y);
void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg);

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size) {
  BinaryImageMaskData *bimd = (BinaryImageMaskData*) malloc(sizeof(BinaryImageMaskData));
  bimd->size = size;
  bimd->data = (uint8_t*)malloc(size.w * size.h / 8.0 + 0.5);
  binary_image_mask_data_clear_region(bimd, GRect(0, 0, size.w, size.h));
  return bimd;
}

void binary_image_mask_data_destroy(BinaryImageMaskData* bimd) {
  if (bimd != NULL) {
    free(bimd->data);
    free(bimd);
  }
}

struct BinaryImageMaskData* binary_image_mask_data_create_from_resource(GSize size, uint32_t resource_id) {
  BinaryImageMaskData *bimd = binary_image_mask_data_create(size);
  ResHandle handle = resource_get_handle(resource_id);
  resource_load(handle, bimd->data, bimd->size.w * bimd->size.h / 8.0 + 0.5);
  return bimd;
}

void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val) {
  uint8_t byte = bimd->data[(bimd->size.w * y + x) / 8];
  int reminder = (bimd->size.w * y + x) % 8;
  uint8_t mask = 1 << reminder;
  bimd->data[(bimd->size.w * y + x) / 8] = (byte & ~mask) | (((val) ? 0b11111111 : 0) & mask);
}

void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region) {
  for(int x = region.origin.x; x < region.origin.x + region.size.w; x++) {
    for(int y = region.origin.y; y < region.origin.y + region.size.h; y++) {
      binary_image_mask_data_set_bit(bimd, x, y, false);
    }
  }
}

bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y) {
  uint8_t byte = bimd->data[(bimd->size.w * y + x) / 8];
  int reminder = (bimd->size.w * y + x) % 8;
  return ((byte >> reminder) & 1) == 1;
}

void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg) {
  for (int x = 0; x < src_reg.size.w; x++) {
    for(int y = 0; y < src_reg.size.h; y++) {
      bool val = binary_image_mask_data_get_pixel(src, src_reg.origin.x + x, src_reg.origin.y + y);
      if (val) {
        binary_image_mask_data_set_bit(dest, dest_pos.x + x, dest_pos.y + y, val);
      }
    }
  }
}
