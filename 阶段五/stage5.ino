#ifndef Wire1
  #define Wire1 Wire
#endif
#include<Arduino.h>
// rtos
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<freertos/queue.h>
// MQTT&WIFI
#include<PubSubClient.h>
#include<WiFi.h>
#include<WiFiClientSecure.h>
// BME280
#include<Adafruit_BME280.h>
#include<Adafruit_Sensor.h>
#include<Wire.h>
// OLED
#include<SSD1306.h>
// JSON
#include<ArduinoJson.h>

const char* Wifi_ssid = "CMCC-YcDN";
const char* Wifi_password = "yfyx2424";
// MQTT相关
const char* Mqtt_broker = "raf13fa2.ala.cn-hangzhou.emqxsl.cn";
const int Mqtt_port = 8883;
const char* Mqtt_username = "Sensor&Led";
const char* Mqtt_password = "MQTTS&L";
const char* Mqtt_topic = "testtopic";
const char* Mqtt_topic2 = "ControlLed";; // 控制LED的主题

// IIC
const int SCL_pin = 5;
const int SDA_pin = 4;

// CA证书
const char* CA_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

// 初始化WIFI和MQTT
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);
// BME
Adafruit_BME280 bme;
const int ledPin = 8;
unsigned long lastMillis = 0; 
// OLED
SSD1306Wire display(0x3c, SDA_pin, SCL_pin, GEOMETRY_128_64, I2C_ONE);
// 任务队列
xQueueHandle queueMsg1 = xQueueCreate(8, sizeof(char[200])); // OLED
// xQueueHandle queueMsg2 = xQueueCreate(8, sizeof(char[200])); // MQTT //最初想用任务队列，但是实现不了每1s传数据到队列，而每5s上传MQTT同时保持实时
xQueueHandle queueMsg3 = xQueueCreate(8, sizeof(char[32])); // Led控制
// 信号量
SemaphoreHandle_t mutex = NULL; // 创建信号量
char mqttBuf[256];

// void serialPrint1(void* arg){
//     while (1)
//     {
//         Serial.println("1");
//         vTaskDelay(1000);
//     }
// }

// void serialPrint2(void* arg){
//     while (1)
//     {
//         Serial.println("2");
//         vTaskDelay(5000);
//     }
// }

void connectToWifi(){
    WiFi.begin(Wifi_ssid, Wifi_password);
    Serial.print("Connecting to ");
    Serial.print(Wifi_ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nSuccessfully Connected!");
}

void connectToMqtt(){
    while (!mqtt_client.connected())
    {
        String client_id = "esp32-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(),  Mqtt_username, Mqtt_password))
        {
            Serial.println("MQTT Connected");
            mqtt_client.subscribe(Mqtt_topic);
            mqtt_client.subscribe(Mqtt_topic2); //添加LED控制主题
            mqtt_client.publish(Mqtt_topic, "Hello");
        }
        else 
        {
            Serial.print("Fail to connect to MQTT broker, rc=");
            Serial.println(mqtt_client.state());
            delay(5000);
        }
    }
}

void readTenpandHum(void* arg){
    bme.begin(0x76);
    while (1)
    {
        JsonDocument jsonDoc;
        jsonDoc["Tem"] = bme.readTemperature();
        jsonDoc["Hum"] = bme.readHumidity();
        char jsonBuf[256];
        serializeJson(jsonDoc, jsonBuf);
        if (xQueueSend(queueMsg1, jsonBuf, 1000) != pdPASS)
            Serial.println("Queue1 is full.");
        if (xSemaphoreTake(mutex, 1000) == pdPASS) // mutex 互斥，修改数据时无法访问数据
        {
            strcpy(mqttBuf, jsonBuf);
            xSemaphoreGive(mutex);
        }
        vTaskDelay(1000);
    }
}

void handleOLED(void* arg){
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    char jsonBuf[256];
    while (1)
    {
        if (xQueueReceive(queueMsg1, jsonBuf, 1000) == pdPASS)
        {
            JsonDocument jsonDoc;
            deserializeJson(jsonDoc, jsonBuf);
            float Temperature = jsonDoc["Tem"];
            float Humidity = jsonDoc["Hum"];
            display.clear();
            display.drawString(0, 0, "T: " + String(Temperature, 1) + "*C");
            display.drawString(0, 16, "H: " + String(Humidity, 1) + "%");
            display.display();
            // vTaskDelay(1000);
        }
    }
}

void handleLED(void* arg){
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    char buf[32];
    while (1)
    {
        if (xQueueReceive(queueMsg3, buf, 1000) == pdPASS)
        {
            JsonDocument jsonDoc;
            deserializeJson(jsonDoc, buf);
            if (strcmp(jsonDoc["led"], "ON") == 0)
                digitalWrite(ledPin, HIGH);
            else if (strcmp(jsonDoc["led"], "OFF") == 0)
                digitalWrite(ledPin, LOW);
        }
    }
}

void mqttCallBack(char* topic, byte* payload, unsigned int length){
    if (strcmp(topic, Mqtt_topic2) != 0)
        return;
    char data[32];
    JsonDocument jsonDoc;
    for (unsigned int i = 0; i < length; i++)
        data[i] = payload[i];
    xQueueSend(queueMsg3, data, 1000);
}

void handleMQTT(void* arg){
    esp_client.setCACert(CA_cert);
    mqtt_client.setServer(Mqtt_broker, Mqtt_port);
    mqtt_client.setKeepAlive(60);
    mqtt_client.setCallback(mqttCallBack);
    connectToMqtt();
    // char jsonBuf[256];
    while (1)
    {
        if (millis() - lastMillis >= 5000) // 5s publish一次
        {
            lastMillis = millis();
            if (xSemaphoreTake(mutex, 500) == pdPASS)
            {
                mqtt_client.publish(Mqtt_topic, mqttBuf);
                xSemaphoreGive(mutex);
            }
        }
        mqtt_client.loop();
    }
}

void setup(){
    Serial.begin(115200);
    connectToWifi(); //Wifi
    mutex = xSemaphoreCreateMutex(); // 创建mutex，有一次没创建导致报错
    xTaskCreate(readTenpandHum, "BME280", 1024*4, NULL, 1, NULL);
    xTaskCreate(handleOLED, "OLED", 1024*4, NULL, 1 , NULL);
    xTaskCreate(handleLED, "LED", 1024, NULL, 1, NULL);
    xTaskCreate(handleMQTT, "MQTT", 1024*12, NULL, 1, NULL);
}

void loop(){
    
}