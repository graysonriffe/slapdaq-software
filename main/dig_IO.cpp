
//Necessary imports
#include "dig_IO.h"

esp_err_t set_all_pins_as_input(void)
{
    //Pins 21, 7, 6, 5, 4, 2, 1, and 14 will be 8 digital I/O

    //Configure all these pins to be inputs
    gpio_config_t io_config = {};

    io_config.pin_bit_mask = GPIO_SEL_BITMASK;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE; //Configure to a pulldown resistor

    return gpio_config(&io_config);

}



bool parseDioConfigPacket(const uint8_t *buf, size_t len, uint8_t *type, uint8_t *mask)
{
    if (buf == nullptr || type == nullptr || mask == nullptr || len < DIO_PACKET_LEN)
    {
        return false;
    }
    
    if (buf[0] != MSG_SET_DIO_DIR && buf[0] != MSG_SET_DIO_LEVEL)
    {
        return false;
    }
    *type = buf[0];
    *mask = buf[1];
    return true;
}

esp_err_t applyDioDirections(uint8_t dir_mask)
{
    int index;

    for (index = 0; index < 8; index++)
    {
        bool is_output = (dir_mask>>index) & 0x01;
        gpio_mode_t mode = is_output ? GPIO_MODE_INPUT_OUTPUT : GPIO_MODE_INPUT;

        esp_err_t err = gpio_set_direction(DIO_PINS[index], mode);
        if (err != ESP_OK)
        {
            return err;
        }
    }
    return ESP_OK;
}

// a function to set the level of the input. Used for when a pin becomes an output
esp_err_t applyDioLevels(uint8_t level_mask)
{
    int index;

    for (index = 0; index < 8; index++)
    {
        uint32_t level = (level_mask >> index) & 0x01;

        esp_err_t err = gpio_set_level(DIO_PINS[index], level);
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
        uint8_t level = (uint8_t) (gpio_get_level(DIO_PINS[index]) & 0x01);
        bits |= (uint8_t) (level << index);
    }
    return bits;
}