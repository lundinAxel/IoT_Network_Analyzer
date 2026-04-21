#include "photoresistor.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "esp_random.h"

#include "esp_crt_bundle.h"
#include "mqtt_client.h"
#include "esp_sntp.h"
#include "esp_timer.h"

static void mqtt_publish_task(void *pvParameters);
void timestamp_realtime(char *buffer, size_t buffer_size);
void timestamp_init(void);

esp_mqtt_client_handle_t mqttClient;
esp_timer_handle_t espTimer;
static bool isConnected = false;

static const char *TAG = "mqtts_example";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        isConnected = true;
        //msg_id = esp_mqtt_client_subscribe(client, "esp32testing/sensors", 0);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        xTaskCreate(mqtt_publish_task, "mqtt_publish_task", 4096, NULL, 5, NULL);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        isConnected = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ", event->msg_id, (uint8_t)*event->data);
        msg_id = esp_mqtt_client_publish(client, "topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
        .address.uri = "mqtt://broker.hivemq.com",
        .address.port = 1883
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    mqttClient = client;

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_publish_task(void *pvParameters)
{
    timestamp_init();

    // Wait for NTP sync before starting to publish
    ESP_LOGI(TAG, "Waiting for NTP sync...");
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "NTP synced");

    char payload[128];
    char timestamp[64];
    
    while(1){
        if(isConnected)
        {
            timestamp_realtime(timestamp, sizeof(timestamp));
            int val = photoresistorRead_raw();
            snprintf(payload, sizeof(payload), "{\"DeviceId\":\"esp32-1\",\"Sensor\":\"photo\",\"Raw\":%d,\"Timestamp\":\"%s\"}", val, timestamp);
            int msg_id = esp_mqtt_client_publish(mqttClient, "esp32testing/sensors", payload, 0, 0, 0);
            if (msg_id == 0) ESP_LOGI(TAG, "Sent Data: %d", val);
            else ESP_LOGI(TAG, "Error msg_id:%d while sending data", msg_id);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

    }
}

void timestamp_init(void){
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    //Swedish CEST timezone
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
}

void timestamp_realtime(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm timeinfo;
    time(&now);

    localtime_r(&now, &timeinfo);
    strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%S", &timeinfo);
}
