
//Necessary imports
#include "dig_IO.h"

void dig_io_config_to_input(void)
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
