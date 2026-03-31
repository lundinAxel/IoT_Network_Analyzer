#include <PubSubClient.h>
#include <WiFi.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PHOTORES_GPIO GPIO_NUM_1
#define PHOTORES_LED_GPIO GPIO_NUM_2

//WiFi
const char *ssid = "";
const char *password = "";

//MQTT Broker
const char *mqtt_broker = "192.168.1.215";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = "1883";

WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi(){
    WiFi.begin(ssid, password);
    While(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
}

void connectMQTT() {
    while(!client.connected()){
        client.connect("esp..", mqtt_username, mqtt_password);
        if (!client.connected()){
            delay(500)
            Serial.println("Connecting to MQTT broker...");
        }
    }
}

void setup() {
    Serial.begin(115200);

    connectWiFi();
    client.setServer(mqtt_broker, mqtt_port);
    connectMQTT();

}

void app_main(void)
{




    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}