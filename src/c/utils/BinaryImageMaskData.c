#include <pebble.h>
#include "BinaryImageMaskData.h"

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size);
void binary_image_mask_data_destroy(BinaryImageMaskData* bimd);
struct BinaryImageMaskData* binary_image_mask_data_create_from_resource(GSize size, uint32_t resource_id);
struct BinaryImageMaskData* binary_image_mask_data_create_from_resource_offset(GSize size, uint32_t resource_id, uint32_t offset);
void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val);
void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region, bool centered);
bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y);
void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg, bool centered);

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size) {
  BinaryImageMaskData *bimd = (BinaryImageMaskData*) malloc(sizeof(BinaryImageMaskData));
  bimd->size = size;
  bimd->data = (uint8_t*)malloc(size.w * size.h / 8.0 + 0.9);
  binary_image_mask_data_clear_region(bimd, GRect(0, 0, size.w, size.h), false);
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
  resource_load(handle, bimd->data, bimd->size.w * bimd->size.h / 8.0 + 0.9);
  return bimd;
}

void shift_left_bitstream(uint8_t *data, size_t len, unsigned int n) {
  if (n == 0 || len == 0) return;

  // Handle full-byte shifts first
  unsigned int byte_shift = n / 8;
  unsigned int bit_shift = n % 8;

  if (byte_shift > 0) {
    for (size_t i = 0; i < len; i++) {
      if (i + byte_shift < len)
        data[i] = data[i + byte_shift];
      else
        data[i] = 0;
    }
  }

  if (bit_shift > 0) {
    uint8_t carry = 0;
    for (size_t i = 0; i < len; i++) {
      uint8_t new_carry = data[i] >> (8 - bit_shift);
      data[i] = (data[i] << bit_shift) | carry;
      carry = new_carry;
    }
  }
}

struct BinaryImageMaskData* binary_image_mask_data_create_from_resource_offset(GSize size, uint32_t resource_id, uint32_t offset_bits) {
  BinaryImageMaskData *bimd = binary_image_mask_data_create(size);

  uint32_t resource_size_bits = (bimd->size.w * bimd->size.h);
  uint32_t resource_size_bytes = (resource_size_bits / 8.0) + 0.9;

  uint32_t offset_bytes = offset_bits / 8;
  uint32_t offset_reminder_bits = offset_bits % 8;
  uint32_t actual_size = resource_size_bytes + ((offset_reminder_bits > 0) ? 1 : 0);

  if (offset_reminder_bits > 0) {
    free(bimd->data);
    bimd->data = (uint8_t*)malloc(actual_size);
  }

  ResHandle handle = resource_get_handle(resource_id);
  resource_load_byte_range(handle, offset_bytes, bimd->data, actual_size);
  shift_left_bitstream(bimd->data, offset_reminder_bits, sizeof(bimd->data));
  return bimd;
}

void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val) {
  uint8_t byte = bimd->data[(bimd->size.w * y + x) / 8];
  int reminder = (bimd->size.w * y + x) % 8;
  uint8_t mask = 1 << reminder;
  bimd->data[(bimd->size.w * y + x) / 8] = (byte & ~mask) | (((val) ? 0b11111111 : 0) & mask);
}

void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region, bool centered) {
  GPoint offset = GPoint(0, 0);
  if (centered) {
    offset = GPoint(region.size.w / 2, region.size.h / 2);
  }


  for(int x = region.origin.x; x < region.origin.x + region.size.w; x++) {
    GPoint actual_pos = GPoint(x - offset.x, 0);
    if(actual_pos.x > bimd->size.w) {
      break;
    }
    if (actual_pos.x < 0) {
      continue;
    }
    for(int y = region.origin.y; y < region.origin.y + region.size.h; y++) {
      actual_pos.y = y - offset.y;
      if(actual_pos.y > bimd->size.h) {
        break;
      }
      if (actual_pos.y < 0) {
        continue;
      }
      binary_image_mask_data_set_bit(bimd, actual_pos.x, actual_pos.y, false);
    }
  }
}

bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y) {
  uint8_t byte = bimd->data[(bimd->size.w * y + x) / 8];
  int reminder = (bimd->size.w * y + x) % 8;
  return ((byte >> reminder) & 1) == 1;
}

void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg, bool centered) {
  GPoint offset = GPoint(0, 0);
  if (centered) {
    offset = GPoint(src_reg.size.w / 2, src_reg.size.h / 2);
  }

  for (int x = 0; x < src_reg.size.w; x++) {
    GPoint actual_dest_pos = GPoint(dest_pos.x + x - offset.x, 0);
    if (actual_dest_pos.x >= dest->size.w) {
      break;
    }
    if (actual_dest_pos.x < 0) {
      continue;
    }
    for(int y = 0; y < src_reg.size.h; y++) {
      actual_dest_pos.y = dest_pos.y + y - offset.y;
      if (actual_dest_pos.y >= dest->size.h) {
        break;
      }
      if (actual_dest_pos.y < 0) {
        continue;
      }
      bool val = binary_image_mask_data_get_pixel(src, src_reg.origin.x + x, src_reg.origin.y + y);
      if (val) {
        binary_image_mask_data_set_bit(dest, actual_dest_pos.x, actual_dest_pos.y, val);
      }
    }
  }
}
