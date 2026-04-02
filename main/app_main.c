#include "wifi_connect.h"
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
    photoresistorInit();

    connectWiFi();

    xTaskCreate(sensorTask, "Sensor Task", 1000, NULL, 1, NULL);
    //xTaskCreate(emergencyTask, "Emergency Task", 1000, NULL, 1, NULL);

}