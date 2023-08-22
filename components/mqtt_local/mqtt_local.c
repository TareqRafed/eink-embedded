

#include "mqtt_local.h"
#include "eink.h"

uint8_t *img = NULL;
uint16_t imgSize = 0;

int should_start = 0;

void remove_non_numbers(char *str)
{
    int i, j;

    // Iterate through each character in the string.
    for (i = 0, j = 0; str[i] != '\0'; i++)
    {
        // If the current character is a numeric digit, keep it.
        if (str[i] >= '0' && str[i] <= '9')
        {
            str[j] = str[i];
            j++;
        }
    }

    // Null-terminate the new string.
    str[j] = '\0';
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client, "device/yakj132gv", 2);
        ESP_LOGI(MQTT_TAG, "subscribed to NaN successfully, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_publish(client, "device/yakj132gv", "get\0", 0, 2, 0); // change to fetch
        ESP_LOGI(MQTT_TAG, "sent cmd fetch publish successfully, msg_id=%d", msg_id);

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
        if (strcmp(event->data, "fetch") == 0 || strcmp(event->data, "get") == 0)
        {
            ESP_LOGI(MQTT_TAG, "Caught Fetch, breaking");
            break;
        }

        if (strcmp(event->data, "start") == 0)
        {
            should_start = 1;
            ESP_LOGI(MQTT_TAG, "Start");
            break;
        }

        if (should_start)
        {
            const char delimiter[] = ",";
            char *token = strtok((event)->data, delimiter);

            while (token != NULL && imgSize < 48000)
            {
                remove_non_numbers(token);
                if (token[0] < '0' || token[0] > '9')
                {
                    token = strtok(NULL, delimiter);
                    continue;
                }

                img[imgSize] = atoi(token);
                imgSize += 1;

                token = strtok(NULL, delimiter);
            }
        }

        ESP_LOGI(MQTT_TAG, "Image Size %d", imgSize);

        if (imgSize == 48000)
        {
            should_start = 0;
            ESP_LOGI(MQTT_TAG, "ending");
            EPD_Display(img);
            EPD_Sleep();
            imgSize = 0;
            break;
        }

        ESP_LOGI(MQTT_TAG, "Free memory: %d bytes, current packet length %d, current offset %d", esp_get_free_heap_size(), event->data_len, event->current_data_offset);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
        imgSize = 0;
        free(img);
        break;
    default:
        ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);

    mqtt_event_handler_cb(event_data);
}

void mqtt_connect_to_server(void)
{
    ESP_LOGI(MQTT_TAG, "[APP] Free memory: %d bytes before MQTT", esp_get_free_heap_size());

    ESP_LOGI(MQTT_TAG, "Starting MQTT Service");

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://3.69.75.108:1883",
        .username = "admin",
        .password = "123@123",
        .buffer_size = 1024 * 7,
        .out_buffer_size = 512,
        .task_stack = 1024 * 2,

        .protocol_ver = MQTT_PROTOCOL_V_3_1_1

    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    ESP_LOGI(MQTT_TAG, "[APP] Free memory: %d bytes after MQTT", esp_get_free_heap_size());
}
