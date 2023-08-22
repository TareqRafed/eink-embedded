
#pragma once

#include <string.h>

#include "esp_err.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

/* Event group to signal when succeed connection */
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0 // relevant to `s_wifi_event_group`
#define WIFI_FAIL_BIT BIT1      // relevant to `s_wifi_event_group`

/**
 * @brief Connection attempts
 *
 */
static int s_retry_num = 0;

/**
 * @brief Prefixed for wifi related Logs
 *
 */
static const char *WIFI_TAG = "wifi-module";

/**
 * @brief Starts connecting to wifi
 *
 */
void wifi_init();

// Helpers
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

/**
 * @brief General Configs
 *
 */
#define SERIAL_NUMBER "NaN"
#define WIFI_SSID "Airbox-B045"
#define WIFI_PASS "NAB34G43"
#define WIFI_MAXIMUM_RETRY 5