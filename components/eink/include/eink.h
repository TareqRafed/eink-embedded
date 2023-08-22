#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp8266/spi_struct.h"
#include "esp8266/gpio_struct.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/spi.h"

static const char *TAG_EINK = "E-INK";

#define SPI_WRITE_BUFFER_MAX_SIZE 2048

#define CS_PIN 15
#define RST_PIN 2
#define DC_PIN 4 // High for data, low for commands
#define BUSY_PIN 5

#define POWER_PIN 10 // when low disables power to E-INK

#define s_width 800
#define s_height 480

typedef enum
{
    SPI_SEND = 0,
    SPI_RECV
} spi_master_mode_t;

/**
 * @brief transmit data using SPI as a Master
 *
 * @param data
 * @param len_bytes
 */
static void IRAM_ATTR spi_master_transmit(uint8_t *data, uint32_t len_bytes);

/**
 * @brief Init SPI for transmitting
 *
 */
void Init_SPI(void);

/**
 * @brief Init GPIOs for transmitting
 *
 */
void GPIO_Init();

/**
 * @brief transmits  in command mode
 *
 * @param data
 */
void EPD_SendCommand(uint8_t data);

/**
 * @brief transmits data
 *
 * @param data
 */
void EpdSpiTransferCallback(uint8_t *data);

/**
 * @brief Resets screen
 *
 */
void EPD_Reset();

/**
 * @brief Initializes display registers
 *
 */
void EPD_Init();

/**
 * @brief transmits image
 *
 * @param blackimage
 */
void EPD_Display(const uint8_t *blackimage);

/**
 * @brief Displays Image (Refresh)
 *
 */
void EPD_TurnOnDisplay(void);

/**
 * @brief Blocks until screen isn't busy
 *
 */
static void EPD_WaitUntilIdle(void);

/**
 * @brief Go to deep sleep (display)
 *
 */
void EPD_Sleep(void);

/**
 * @brief Clears screen (goes to white)
 *
 */
void EPD_Clear(void);