/*********************************************************
 * 本例中讲完成 SPI1 和SPI2 的通讯 ，SPI1为主机，SPI2为从机
 * 启动两个任务，分别对应SPI主机和从机
 *********************************************************/
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "freertos/event_groups.h"
#include <string.h>

#define GPIO_MASTER_MOSI 12
#define GPIO_MASTER_MISO 13
#define GPIO_MASTER_SCLK 15
#define GPIO_MASTER_CS 14
#define SPI_MASTER SPI2_HOST
#define CLOCK_SPEED 10 * 1000 * 1000

#define GPIO_SLAVE_MOSI 21
#define GPIO_SLAVE_MISO 20
#define GPIO_SLAVE_SCLK 40
#define GPIO_SLAVE_CS 19
#define SPI_SLAVE SPI3_HOST

#define MASTER "MASTER"
#define SLAVE "SLAVE"

EventGroupHandle_t ready = NULL; // 等待双方都准备好的时间句柄
spi_device_handle_t master_dev;  // 设备句柄

#define EXAMPLE 1
#if EXAMPLE == 1

/*
 * 实验1： 主机发送：I'm master，从机发送 I'm slave，同时收发，间隔1秒收发一次
 * 因为主从发送数据是不对等的，看接收和发送的时候都会出现什么问题
 */
static uint16_t m_index = 0;
static uint16_t s_index = 0;

void master_transaction()
{
    const size_t BUFFER_SIZE = 20;
    char send_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE + 1];
    sprintf(send_buffer, "I'm master(%d)", m_index++);
    spi_transaction_t t = {
        .length = sizeof(send_buffer) * 8, // 长度大小是按位计算的，所以要 *8
        .tx_buffer = send_buffer,          // 发送缓冲区
        .rx_buffer = read_buffer,          // 读取缓冲区
    };
    // 发送数据
    esp_err_t res = spi_device_transmit(master_dev, &t);
    if (res == ESP_OK)
    {
        read_buffer[BUFFER_SIZE] = 0; // 强制结束符
        ESP_LOGE(MASTER, "发送成功 : %s", send_buffer);
        ESP_LOGE(MASTER, "接收成功 : %s", read_buffer);
    }
    else
    {
        ESP_LOGE(MASTER, "数据发送失败 : %d", res);
    }
}

void slave_transaction()
{
    const size_t BUFFER_SIZE = 20;
    char send_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE + 1];
    sprintf(send_buffer, "I'm slave(%d)", s_index++);
    spi_slave_transaction_t t = {
        .length = BUFFER_SIZE * 8, // 长度大小是按位计算的，所以要 *8
        .tx_buffer = send_buffer,  // 发送缓冲区
        .rx_buffer = read_buffer,  // 读取缓冲区
    };
    // 等待收发数据
    esp_err_t res = spi_slave_transmit(SPI_SLAVE, &t, portMAX_DELAY);
    if (res == ESP_OK)
    {
        ESP_LOGI(SLAVE, "发出 : %s", send_buffer);
        ESP_LOGI(SLAVE, "收到 : %s", read_buffer);
    }
    else
    {
        ESP_LOGI(SLAVE, "数据发送失败 : %d", res);
    }
}
#elif EXAMPLE == 2
/*
 * 实验2：带地址的交互，主机读取从机寄存器地址的数据，从机根据主机发送的地址读取内容
 * 传输的数据格式为： 8位地址，8位寄存器数据，[n*8 位消息内容]，每一帧中必带地址和寄存器数据
 * 如果只是读寄存器，则数据位填充1或0，如果不为0，则返回对应长度的消息。
 * 命令：
 *      0x01 新消息到达寄存器，查看是否有新信息到达，返回1字节数据，0表示没有，其他表示新消息的长度（字节数）
 *      0x02 新消息寄存器，读取最新的消息，长度为上次的数据长度
 *      0x81 消息发送长度寄存器，要发送消息的长度
 *      0x82 要发送的消息，发送消息寄存器准备发送一条信息，随后一个字节为要发送消息的长度（字节数），再随后是正式消息
 */

QueueHandle_t message_queue = NULL;
// 寄存器结构体
typedef struct
{
    uint8_t addr;  // 寄存器地址
    uint8_t value; // 寄存器值
} Reg_t;

// 消息结构体
typedef struct
{
    char *msg;
    size_t length;
} Message_t;
void master_transaction()
{

    // 首先发送一个查询指令，看有没有最新消息，如果有，返回的则应该是 寄存器地址+长度
    spi_slave_transaction_t t;
    t.length = 8;       // 地址另存，数据中只有一个无意义空字节
    t.tx_buffer = NULL; // 不发送任何数据
    t.rx_buffer = NULL; // 不接收任何数据
    esp_err_t res = spi_slave_transmit(SPI_SLAVE, &t, portMAX_DELAY);
    //if(res =ESP_OK)
     
}

void slave_transaction()
{

    const size_t BUFFER_SIZE = 2;
    char send_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE + 1];
    send_buffer[0] = 'C';
    send_buffer[1] = 'D';
    spi_slave_transaction_t t = {
        .length = BUFFER_SIZE * 8, // 长度大小是按位计算的，所以要 *8
        .tx_buffer = send_buffer,  // 发送缓冲区
        .rx_buffer = read_buffer,  // 读取缓冲区
    };
    // 等待收发数据
    esp_err_t res = spi_slave_transmit(SPI_SLAVE, &t, portMAX_DELAY);
    if (res == ESP_OK)
    {
        ESP_LOGI(SLAVE, "发出 : %s", send_buffer);
        ESP_LOGI(SLAVE, "收到 : %s", read_buffer);
    }
    else
    {
        ESP_LOGI(SLAVE, "数据发送失败 : %d", res);
    }
}
#elif EXAMPLE == 3

void master_transaction()
{
    const size_t BUFFER_SIZE = 1;
    char send_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE + 1];
    send_buffer[0] = 'A';
    // send_buffer[1]='B';
    spi_transaction_t t = {
        .length = sizeof(send_buffer) * 8, // 长度大小是按位计算的，所以要 *8
        .addr = 'E',
        .tx_buffer = send_buffer, // 发送缓冲区
        .rx_buffer = read_buffer, // 读取缓冲区
    };
    // 发送数据
    esp_err_t res = spi_device_transmit(master_dev, &t);
    if (res == ESP_OK) 
    {
        read_buffer[BUFFER_SIZE] = 0; // 强制结束符
        ESP_LOGE(MASTER, "发送成功 : %s", send_buffer);
        ESP_LOGE(MASTER, "接收成功 : %s", read_buffer);
    }
    else
    {
        ESP_LOGE(MASTER, "数据发送失败 : %d", res);
    }
}

void slave_transaction()
{
    const size_t BUFFER_SIZE = 2;
    char send_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE + 1];
    send_buffer[0] = 'C';
    send_buffer[1] = 'D';
    spi_slave_transaction_t t = {
        .length = BUFFER_SIZE * 8, // 长度大小是按位计算的，所以要 *8
        .tx_buffer = send_buffer,  // 发送缓冲区
        .rx_buffer = read_buffer,  // 读取缓冲区
    };
    // 等待收发数据
    esp_err_t res = spi_slave_transmit(SPI_SLAVE, &t, portMAX_DELAY);
    if (res == ESP_OK)
    {
        ESP_LOGI(SLAVE, "发出 : %s", send_buffer);
        ESP_LOGI(SLAVE, "收到 : %s", read_buffer);
    }
    else
    {
        ESP_LOGI(SLAVE, "数据发送失败 : %d", res);
    }
}
#endif
 

/**
 * @brief SPI 主机初始化
 */
static void spi_master_init()
{
    // 总线参数配置
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_MASTER_MOSI,
        .miso_io_num = GPIO_MASTER_MISO,
        .sclk_io_num = GPIO_MASTER_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};

    // 配置设备参数
    spi_device_interface_config_t dev_cfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = CLOCK_SPEED, 
        .duty_cycle_pos = 128, 
        .mode = 0, 
        .spics_io_num = GPIO_MASTER_CS, 
        .cs_ena_posttrans = 3, 
        .queue_size = 7}; 

    // 总线初始化，DMA 还没学，先不用
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_MASTER, &bus_cfg, SPI_DMA_DISABLED));
    ESP_LOGI(MASTER, "SPI 总线初始化成功！");

    // 挂载 SPI 设备
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_MASTER, &dev_cfg, &master_dev));
    ESP_LOGI(MASTER, "SPI 设备挂载完毕！");
}

/**
 * @brief SPI 主机任务入口
 */
static void task_spi_master_entry(void *params)
{
    spi_master_init();

    xEventGroupSync(ready, 1, 3, portMAX_DELAY);
    // 开始收发消息
    // vTaskDelay(1);  // 等等从机
    ESP_LOGI(MASTER, "Start transmitting data...");
    while (1)
    {
        master_transaction();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief SPI 从机初始化
 */
static void spi_slave_init()
{
    // 设置 SPI 从机设备参数
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_SLAVE_MOSI,
        .miso_io_num = GPIO_SLAVE_MISO,
        .sclk_io_num = GPIO_SLAVE_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};

    // 设置从机设备参数
    spi_slave_interface_config_t slv_cfg = {
        .mode = 0,
        .spics_io_num = GPIO_SLAVE_CS,
        .queue_size = 3,
        .flags = 0};

    // 安装从机设备
    ESP_ERROR_CHECK(spi_slave_initialize(SPI_SLAVE, &bus_cfg, &slv_cfg, SPI_DMA_DISABLED));
    ESP_LOGI(SLAVE, "SPI 从机初始成功！");
}
/**
 * @brief SPI 从机任务入口
 */
static void task_spi_slave_entry(void *params)
{
    spi_slave_init();

    xEventGroupSync(ready, 2, 3, portMAX_DELAY);         //数值2对应于二进制10，表示要设置第二位。数值3对应于二进制11，表示等待第一位和第二位都被设置。
    // 开始收发消息
    ESP_LOGI(SLAVE, "Start transmitting data..."); 
    while (1) 
    { 
        slave_transaction(); 
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    ready = xEventGroupCreate();
    // xTaskCreatePinnedToCore(task_spi_slave_entry, "Slave", 10240, NULL, 1, NULL, 1);
    // xTaskCreatePinnedToCore(task_spi_master_entry, "Master", 10240, NULL, 1, NULL, 0);
    xTaskCreate(task_spi_slave_entry, "Slave", 10240, NULL, 1, NULL);
    xTaskCreate(task_spi_master_entry, "Master", 10240, NULL, 1, NULL);
}
