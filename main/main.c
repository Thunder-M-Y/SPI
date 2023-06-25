#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "esp_log.h"

void app_main(void)
{
    // SPI总线初始化
    spi_bus_config_t spi_bus_cfg = {
        .miso_io_num = GPIO_NUM_1,
        .mosi_io_num = GPIO_NUM_2,
        .sclk_io_num = GPIO_NUM_42,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_bus_cfg, SPI_DMA_DISABLED));

    spi_device_interface_config_t s_cfg = {
        .command_bits = 0,                  // 命令位长度（位数）
        .address_bits = 0,                  // 地址位长度
        .dummy_bits = 0,                    // 空闲位长度
        .clock_speed_hz = 10 * 1000 * 1000, // 频率
        .spics_io_num -= GPIO_NUM_41,       // 设备 cs控制引脚
        .mode = 0,
        .duty_cycle_pos = 128, // 脉冲占比
        .queue_size = 7,
    };
    spi_device_handle_t slave;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &s_cfg, &slave));

    spi_bus_remove_device(slave);
    spi_bus_free(SPI2_HOST);

    spi_device_polling_transmit(SPI2_HOST,)
    vTaskDelete(NULL);
}
