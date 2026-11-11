idf_component_register(SRCS "struct_send.cpp" "adc.cpp" "thermo.cpp" "main.cpp" "dig_IO.cpp" "spi_DAQ.cpp" "wifi.cpp"
                    REQUIRES esp_driver_spi esp_driver_gpio esp_wifi nvs_flash esp_timer
                    INCLUDE_DIRS ".")
