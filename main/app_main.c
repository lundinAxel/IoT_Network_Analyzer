#include "wifi_connect.h"
#include "mqtt.h"
#include "photoresistor.h"

#include <stdio.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

void app_main()
{
    WiFiInit();
    WiFiConnect();
    
    mqtt_app_start();
    
    adc_init();
    while(1)
    {
        int raw = photoresistorRead_raw();
        int mv = raw * 3300 / 4095;

        printf("Photoresistor RAW = %d, mV = %d\n", raw, mv);

        vTaskDelay(pdMS_TO_TICKS(500));
    }


    //xTaskCreate(sensorTask, "Sensor Task", 1000, NULL, 1, NULL);
    //xTaskCreate(emergencyTask, "Emergency Task", 1000, NULL, 1, NULL);

}