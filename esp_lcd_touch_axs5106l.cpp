#include "esp_lcd_touch_axs5106l.h"
TwoWire *g_touch_i2c;       

uint16_t g_width; 
uint16_t g_height;
uint16_t g_rotation;
touch_data_t g_touch_data;

bool g_touch_int_flag = false;

static bool touch_i2c_write(uint8_t driver_addr, uint8_t reg_addr, const uint8_t *data, uint32_t length)
{
    g_touch_i2c->beginTransmission(driver_addr);
    g_touch_i2c->write(reg_addr);
    g_touch_i2c->write(data, length);


    if (g_touch_i2c->endTransmission() != 0) {
        Serial.println("The I2C transmission fails. - I2C Read\r\n");
        return false;
    }
    return true;
}

static bool touch_i2c_read(uint8_t driver_addr, uint8_t reg_addr, uint8_t *data, uint32_t length)
{
    g_touch_i2c->beginTransmission(driver_addr);
    g_touch_i2c->write(reg_addr);
    if (g_touch_i2c->endTransmission() != 0) {
        Serial.println("The I2C write fails. - I2C Read\r\n");
        return false;
    }

    g_touch_i2c->requestFrom(driver_addr, length);
    if (g_touch_i2c->available() != length) {
        Serial.println("The I2C read fails. - I2C Read\r\n");
        return false;
    }
    g_touch_i2c->readBytes(data, length);
    return true;
}

static void touch_int_cb(void)
{
    g_touch_int_flag = true;
}


void bsp_touch_init(TwoWire *touch_i2c,int tp_rst, int tp_int, uint16_t rotation, uint16_t width, uint16_t height)
{
    g_touch_i2c = touch_i2c;
    g_width = width;
    g_height = height;
    g_rotation = rotation;

    pinMode(tp_rst, OUTPUT);

    digitalWrite(tp_rst, LOW);
    delay(200);
    digitalWrite(tp_rst, HIGH);
    delay(300);

    attachInterrupt(tp_int, touch_int_cb, FALLING);

    uint8_t data[5] = {0};
    touch_i2c_read(AXS5106L_ADDR, AXS5106L_ID_REG, data, 3);
    if (data[0] != 0){
        Serial.print("read: ");
        Serial.print(data[0]);Serial.print(data[1]);Serial.print(data[2]);
        Serial.println();
    }
}


void bsp_touch_read(void)
{
    uint8_t data[14] = {0};
    g_touch_data.touch_num = 0;

    if (g_touch_int_flag == false)
        return;
    g_touch_int_flag = false;
    
    touch_i2c_read(AXS5106L_ADDR, AXS5106L_TOUCH_DATA_REG, data, 14);

    g_touch_data.touch_num = data[1];
    if (g_touch_data.touch_num == 0)
        return;
    for (uint8_t i = 0; i < g_touch_data.touch_num; i++){
        g_touch_data.coords[i].x = ((uint16_t)(data[2+i*6] & 0x0f)) << 8;
        g_touch_data.coords[i].x |= data[3+i*6];
        g_touch_data.coords[i].y = (((uint16_t)(data[4+i*6] & 0x0f)) << 8);
        g_touch_data.coords[i].y |= data[5+i*6];
    }
}

bool bsp_touch_get_coordinates(touch_data_t *touch_data)
{
    if ((touch_data == NULL) || (g_touch_data.touch_num == 0))
        return false;

    for (int i = 0; i < g_touch_data.touch_num; i++)
    {
        switch (g_rotation)
        {
        case 1:
            touch_data->coords[i].y = g_touch_data.coords[i].x;
            touch_data->coords[i].x = g_touch_data.coords[i].y;
            break;
        case 2:
            touch_data->coords[i].x = g_touch_data.coords[i].x;
            touch_data->coords[i].y = g_height - 1 - g_touch_data.coords[i].y;
            break;
        case 3:
            touch_data->coords[i].y = g_height - 1 - g_touch_data.coords[i].x;
            touch_data->coords[i].x = g_width - 1 - g_touch_data.coords[i].y;
            break;
        default:
            touch_data->coords[i].x = g_width - 1 - g_touch_data.coords[i].x;
            touch_data->coords[i].y = g_touch_data.coords[i].y;
            break;
        }
    }
    return true;
}