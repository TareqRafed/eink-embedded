#include "eink.h"

static void IRAM_ATTR spi_master_send_length(uint32_t len)
{
    spi_trans_t trans;
    uint16_t cmd = SPI_MASTER_WRITE_STATUS_TO_SLAVE_CMD;
    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0;
    trans.bits.cmd = 8 * 1;
    trans.bits.addr = 0;     // transmit status do not use address bit
    trans.bits.mosi = 8 * 4; // status length is 32bit
    trans.cmd = &cmd;
    trans.addr = NULL;
    trans.mosi = &len;
    spi_trans(HSPI_HOST, &trans);
}

/* In order to improve the transmission efficiency, it is recommended that the external
    incoming data is (uint32_t *) type data, do not use other type data. */
static uint32_t write_data[SPI_WRITE_BUFFER_MAX_SIZE / 4];

static void spi_master_write_slave(void *arg)
{

    uint32_t total_len = 0;
    uint32_t send_len = 0;

    for (uint32_t loop = 0; loop < sizeof(write_data) / sizeof(write_data[0]); loop++)
    {
        write_data[loop] = 0x34343434;
    }

    vTaskDelay(5000 / portTICK_RATE_MS);
    ESP_LOGI(TAG_EINK, "Start test send data");
    while (1)
    {

        send_len = sizeof(write_data);
        spi_master_send_length(send_len);

        // wait ESP8266 received the length

        for (uint32_t loop = 0; loop < (send_len + 63) / 64; loop++)
        {
            // transmit data, ESP8266 only transmit 64bytes one time
            spi_master_transmit(write_data + (loop * 16), 64 / sizeof(uint32_t));
        }
        // send 0 to clear send length, and tell Slave send done
        spi_master_send_length(0);

        total_len += send_len;

        if (total_len >= (10 * 1024 * 1024))
        {
            ESP_LOGI(TAG_EINK, "total_len=%d\r\n", total_len);
            for (;;)
            {
                vTaskDelay(1000);
            }
        }
    }
}

/* SPI transmit data, format: 8bit command (read value: 3, write value: 4) + 8bit address(value: 0x0) + 64byte data */
static void IRAM_ATTR spi_master_transmit(uint8_t *data, uint32_t len_bytes)
{
    spi_trans_t trans;
    uint16_t cmd;
    uint32_t addr = 0x0;

    if (len_bytes > 64)
    {
        ESP_LOGE(TAG_EINK, "ESP8266 only support transmit 64bytes(16 * sizeof(uint32_t)) one time");
        return;
    }

    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0; // clear all bit

    cmd = SPI_MASTER_WRITE_DATA_TO_SLAVE_CMD;
    trans.bits.mosi = len_bytes * 8; // One time transmit only support 64bytes
    trans.mosi = data;

    trans.bits.cmd = 0;
    trans.bits.addr = 0; // transmit data will use 8bit address
    // trans.cmd = &cmd;
    // trans.addr = &addr;

    spi_trans(HSPI_HOST, &trans);
}

void Init_SPI(void)
{

    ESP_LOGI(TAG_EINK, "init spi");
    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;

    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Set SPI to master mode
    // ESP8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_2MHz_DIV;
    spi_config.event_cb = NULL;
    spi_init(HSPI_HOST, &spi_config);

    // xTaskCreate(spi_master_write_slave_task, "spi_master_write_slave_task", 2048, NULL, 3, NULL);
}

void GPIO_Init()
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL << RST_PIN) | (1ULL << DC_PIN) | (1ULL << CS_PIN) | (1ULL << POWER_PIN));
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << BUSY_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;

    gpio_config(&io_conf);

    gpio_set_level(POWER_PIN, 1);
    gpio_set_level(CS_PIN, 1);
}

void EpdSpiTransferCallback(uint8_t *data)
{
    gpio_set_level(CS_PIN, 0);
    spi_master_transmit(data, 1);
    gpio_set_level(CS_PIN, 1);
}

void EPD_SendCommand(uint8_t data)
{
    gpio_set_level(DC_PIN, 0);
    EpdSpiTransferCallback(&data);
}

void EPD_SendData(uint8_t data)
{
    gpio_set_level(DC_PIN, 1);
    EpdSpiTransferCallback(&data);
}

void EPD_Reset()
{
    ESP_LOGI(TAG_EINK, "Reseting");
    vTaskDelay(50 / portTICK_RATE_MS);

    gpio_set_level(RST_PIN, 1);
    vTaskDelay(50 / portTICK_RATE_MS);

    gpio_set_level(RST_PIN, 0);
    vTaskDelay(5 / portTICK_RATE_MS);

    gpio_set_level(RST_PIN, 1);
    vTaskDelay(50 / portTICK_RATE_MS);
}

void EPD_Init()
{
    ESP_LOGI(TAG_EINK, "Init");

    EPD_Reset();

    ESP_LOGI(TAG_EINK, "Sending Commands");

    EPD_SendCommand(0x01); // POWER SETTING
    EPD_SendData(0x07);
    EPD_SendData(0x07); // VGH=20V,VGL=-20V
    EPD_SendData(0x3f); // VDH=15V
    EPD_SendData(0x3f); // VDL=-15V

    EPD_SendCommand(0x04);              // POWER ON
    vTaskDelay(100 / portTICK_RATE_MS); //!!!The delay here is necessary, 200uS at least!!!
    EPD_WaitUntilIdle();

    EPD_SendCommand(0X00); // PANNEL SETTING
    EPD_SendData(0x1F);    // KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

    EPD_SendCommand(0x61); // tres
    EPD_SendData(0x03);    // source 800
    EPD_SendData(0x20);
    EPD_SendData(0x01); // gate 480
    EPD_SendData(0xE0);

    EPD_SendCommand(0X15);
    EPD_SendData(0x00);

    EPD_SendCommand(0X50); // VCOM AND DATA INTERVAL SETTING
    EPD_SendData(0x10);
    EPD_SendData(0x07);

    EPD_SendCommand(0X60); // TCON SETTING
    EPD_SendData(0x22);

    EPD_SendCommand(0x13);
}

static void EPD_WaitUntilIdle(void)
{
    do
    {
        vTaskDelay(20 / portTICK_RATE_MS);
        ESP_LOGI(TAG_EINK, "READ SCREEN BUSY: %i", gpio_get_level(BUSY_PIN));
    } while (!(gpio_get_level(BUSY_PIN)));

    vTaskDelay(20 / portTICK_RATE_MS);
    ESP_LOGI(TAG_EINK, "READ SCREEN BUSY RELEASE");
}

static void EPD_Show(void)
{
    EPD_SendCommand(0x12);             // DISPLAY REFRESH
    vTaskDelay(10 / portTICK_RATE_MS); //!!!The delay here is necessary, 200uS at least!!!

    // Enter sleep mode
    EPD_SendCommand(0X02); // power off
    EPD_WaitUntilIdle();
    EPD_SendCommand(0X07); // deep sleep
    EPD_SendData(0xA5);
}

void EPD_Sleep(void)
{
    EPD_SendCommand(0X02); // power off
    EPD_WaitUntilIdle();
    EPD_SendCommand(0X07); // deep sleep
    EPD_SendData(0xA5);
}

void EPD_Clear(void)
{
    ESP_LOGI(TAG_EINK, "Clearing...");

    uint16_t Width, Height;
    Width = (s_width % 8 == 0) ? (s_width / 8) : (s_width / 8 + 1);
    Height = s_height;

    EPD_SendCommand(0x10);
    for (uint32_t i = 0; i < Height * Width; i++)
    {
        EPD_SendData(0x00);
    }
    EPD_SendCommand(0x13);
    for (uint32_t i = 0; i < Height * Width; i++)
    {
        EPD_SendData(0x00);
    }
    EPD_TurnOnDisplay();
}

void EPD_Display(const uint8_t *blackimage)
{
    int32_t Width, Height;
    Width = (s_width % 8 == 0) ? (s_width / 8) : (s_width / 8 + 1);
    Height = s_height;

    // send black data
    EPD_SendCommand(0x13);
    for (int j = 0; j < Height; j++)
    {
        for (int i = 0; i < Width; i++)
        {
            EPD_SendData(~blackimage[i + j * Width]);
        }
    }
    EPD_TurnOnDisplay();
}

void EPD_TurnOnDisplay(void)
{
    EPD_SendCommand(0x12); // DISPLAY REFRESH
    EPD_WaitUntilIdle();
}