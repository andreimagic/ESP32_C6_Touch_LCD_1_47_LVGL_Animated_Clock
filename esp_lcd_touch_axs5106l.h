#pragma once

#include <Arduino.h>
#include <Wire.h>

#define MAX_TOUCH_MAX_POINTS    5

#define AXS5106L_ADDR 0x63
#define AXS5106L_ID_REG 0x08

#define AXS5106L_TOUCH_DATA_REG 0X01


typedef struct{
    uint16_t x;
    uint16_t y;
}coords_t;

typedef struct{
    coords_t coords[MAX_TOUCH_MAX_POINTS];
    uint8_t touch_num;
}touch_data_t;


// bool get_touch_data(touch_data_t *touch_data);
void bsp_touch_read(void);
bool bsp_touch_get_coordinates(touch_data_t *touch_data);
// bool touch_init(TwoWire *touch_i2c, int tp_rst, int tp_int);
void bsp_touch_init(TwoWire *touch_i2c,int tp_rst, int tp_int, uint16_t rotation, uint16_t width, uint16_t height);
