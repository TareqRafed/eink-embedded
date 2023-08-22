

#include "FreeRTOS.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/task.h"

#include "eink.h"
#include "wifi.h"
#include "mqtt_local.h"

static const char *MAIN_TAG = "main-thread";

void app_main()

{

    ESP_LOGI(MAIN_TAG, "E-INK with serial number of NaN");

    ESP_LOGI(MAIN_TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    img = heap_caps_malloc(48100, MALLOC_CAP_8BIT);

    if (img == NULL)
    {
        ESP_LOGE(MAIN_TAG, "Image Allocating Fail");
    }

    wifi_init();

    GPIO_Init();
    Init_SPI();
    EPD_Init();

    mqtt_connect_to_server();
}
