#pragma once

#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

extern uint8_t *img;
extern uint16_t imgSize;

static const char *MQTT_TAG = "MQTT";

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_connect_to_server(void);
