#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

/* Component includes */
#include "esp32-spi-ssd1327.h"


/* Going off  https://learn.adafruit.com/assets/111179 */
#define SPI_MOSI_PIN_NUM 18
/* #define SPI_MISO_PIN_NUM 19 */ // Not used for the OLED
#define SPI_SCK_PIN_NUM  14
#define SPI_CS_PIN_NUM   15
#define DC_PIN_NUM   26
#define RST_PIN_NUM  25

/* SPI0 and SPI1 are reserved so we can choose between SPI2 and SPI3 which
 * are also referred to elsewhere as HSPI and VSPI respectively. Here we
 * commit to using HSPI */
#define SPI_HOST_TAG SPI2_HOST


void app_main(void) {
    /* {{{ */
	/* 1. Configure the spi master bus */
    spi_bus_config_t spi_bus_cfg = {
        .miso_io_num = -1,
        .mosi_io_num = SPI_MOSI_PIN_NUM,
        .sclk_io_num = SPI_SCK_PIN_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));

    /* 2. Configure the spi device */
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 10 * 1000 * 1000,      // Clock out at 10 MHz
        .mode = 0,                               // SPI mode 0
        .spics_io_num = SPI_CS_PIN_NUM,          // CS pin
        .queue_size = 7,                         // We want to be able to queue 7 transactions at a time
    };

    spi_device_handle_t oled_dev_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_TAG, &dev_cfg, &oled_dev_handle));

    /* 3. Initialize the remaining GPIO pins */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DC_PIN_NUM),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&io_conf);

    gpio_config_t io_conf2 = {
        .pin_bit_mask = (1ULL << RST_PIN_NUM),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf2);

    /* 4. Create SSD1327 struct for use of the spi_oled functions */
    struct spi_ssd1327 spi_ssd1327 = {
        .dc_pin_num = DC_PIN_NUM,
        .rst_pin_num = RST_PIN_NUM,
        .spi_handle = &oled_dev_handle,
    };
    /* }}} */

    spi_oled_init(&spi_ssd1327);

    printf("screen is on\n");

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    spi_oled_draw_square(&spi_ssd1327, 0, 0, 128, 128, SSD1327_GS_0);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1) {
        spi_oled_draw_circle(&spi_ssd1327, 0, 0);
        printf("(1/4) White 16 pixel radius circle is painted\n");
        vTaskDelay(1500 / portTICK_PERIOD_MS);

        spi_oled_draw_square(&spi_ssd1327, 0, 0, 128, 128, SSD1327_GS_0);
        printf("(2/4) Black 128x128 square painted\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        spi_oled_draw_square(&spi_ssd1327, 0, 0, 128, 128, SSD1327_GS_15);
        printf("(3/4) White 128x128 square painted\n");
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        spi_oled_draw_square(&spi_ssd1327, 0, 0, 128, 128, SSD1327_GS_0);
        printf("(4/4) Black 128x128 square painted\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
