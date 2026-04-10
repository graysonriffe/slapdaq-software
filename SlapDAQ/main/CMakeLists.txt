idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES freertos esp_common
                    REQUIRES esp_driver_spi
                    REQUIRES esp_driver_gpio
                    REQUIRES esp_hal_gpspi
                    REQUIRES esp_wifi
                    )