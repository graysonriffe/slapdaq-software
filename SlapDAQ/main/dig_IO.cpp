
//Necessary imports
#include "dig_IO.h"

void set_all_pins_as_input(void)
{
    //Pins 21, 7, 6, 5, 4, 2, 1, and 14 will be 8 digital I/O

    //Configure all these pins to be inputs
    gpio_config_t io_config = {};

    io_config.pin_bit_mask = GPIO_SEL_BITMASK;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE; //Configure to a pulldown resistor

    esp_err_t ret = gpio_config(&io_config);

    ESP_ERROR_CHECK(ret);
}

/*

    TCP byte streams are like this:

    0x00 means everything is an input
    0x01 means that digital IO port associated with place 8 is an output now

*/

bool parseDioConfigPacket(const uint8_t *buf, size_t len, uint8_t *type, uint8_t *dir_mask)
{
    if (len < 2 || buf[0] != MSG_SET_DIO_DIR)
    {
        return false;
    }
    *dir_mask = buf[1];
    return true;
}

esp_err_t applyDioDirections(uint8_t dir_mask)
{
    int index;

    for (index = 0; index < 8; index++)
    {
        bool is_output = (dir_mask>>index) & 0x01;
        gpio_mode_t mode = is_output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;

        esp_err_t err = gpio_set_direction(DIO_PINS[index], mode);
        if (err != ESP_OK)
        {
            return err;
        }
    }
    return ESP_OK;
}

uint8_t read_all_pins(void)
{
    uint8_t bits = 0;
    int index;
    for (index = 0; index < 8; index++)
    {
        uint8_t level = gpio_get_level(DIO_PINS[index]);
        bits |= (level << index);
    }
    return bits;
}