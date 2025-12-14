#pragma once
#include <pebble.h>

typedef struct BinaryImageMaskData {
  GSize size;
  uint8_t *data;
} __attribute__((__packed__)) BinaryImageMaskData;

struct BinaryImageMaskData* binary_image_mask_data_create(GSize size);

void binary_image_mask_data_destroy(BinaryImageMaskData* bimd);

struct BinaryImageMaskData* binary_image_mask_data_create_from_resource(GSize size, uint32_t resource_id);

void binary_image_mask_data_set_bit(BinaryImageMaskData* bimd, int x, int y, bool val);

void binary_image_mask_data_clear_region(BinaryImageMaskData* bimd, GRect region, bool centered);

bool binary_image_mask_data_get_pixel(BinaryImageMaskData* bimd, int x, int y);

void binary_image_mask_data_draw(BinaryImageMaskData* dest, BinaryImageMaskData* src, GPoint dest_pos, GRect src_reg, bool centered);
