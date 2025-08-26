// MQTT&WIFI
#include<PubSubClient.h>
#include<WiFi.h>
#include<WiFiClientSecure.h>
// BME280
#include<Adafruit_BME280.h>
#include<Adafruit_Sensor.h>
#include<Wire.h>
// JSON
#include<ArduinoJson.h>

const char* Wifi_ssid = "TP-LINK_333";
const char* Wifi_password = "hl@3838438";
// MQTT相关
const char* Mqtt_broker = "raf13fa2.ala.cn-hangzhou.emqxsl.cn";
const int Mqtt_port = 8883;
const char* Mqtt_username = "Sensor&Led";
const char* Mqtt_password = "MQTTS&L";
const char* Mqtt_topic = "testtopic";
const char* Mqtt_topic2 = "ControlLed";; // 控制LED的主题

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
Adafruit_BME280 bme;
const int ledPin = 8;
unsigned long lastMillis = 0; 
// const unsigned long interval = 5000; //间隔5s


void startLed(){
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

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

void mqttCallBack(char* topic, byte* payload, unsigned int length){
  Serial.println(topic);
  if (strcmp(Mqtt_topic2, topic) != 0) //检查是否为控制LED的主题
    return;
  String data; // 存储接收的字符串
  JsonDocument jsonDoc;
  for (unsigned int i = 0; i < length; i++)
  {
    data += (char)payload[i];
  }
  deserializeJson(jsonDoc, data); // 把接收的字符串转成json
  if (!jsonDoc["led"].isNull())
  {
    String operation = jsonDoc["led"]; //检查操作并执行
    if (operation == "ON")
      digitalWrite(ledPin, HIGH);
    else if (operation == "OFF")
      digitalWrite(ledPin, LOW);
  }
  // mqtt_client.publish(Mqtt_topic2, "OK"); // 不断触发callback狂发消息的罪魁祸首
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

void publishTempandHum(){
  JsonDocument jsonDoc;
  jsonDoc["temp"] = bme.readTemperature();
  jsonDoc["hum"] = bme.readHumidity();

  char jsonBuf[512];
  serializeJson(jsonDoc, jsonBuf);
  mqtt_client.publish(Mqtt_topic, jsonBuf);
}

void pubWithInterval(unsigned long interval){
  if (millis() - lastMillis >= interval)
  {
    lastMillis = millis();
    publishTempandHum();
  }
}

void setup(){
  startLed();
  // 连接WiFi
  Serial.begin(115200);
  connectToWifi();
  // BME初始化
  bme.begin(0x76);
  
  esp_client.setCACert(CA_cert);

  mqtt_client.setServer(Mqtt_broker, Mqtt_port);
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallBack);
  
  connectToMqtt();
}

void loop(){
  if (!mqtt_client.connected())
  {
    connectToMqtt();
  }
    mqtt_client.loop();
  pubWithInterval(5000);
  delay(50);
}