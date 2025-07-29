#include<WiFi.h>

int led_pin = 5;
const char* ssid = "CMCC-YcDN";
const char* password = "yfyx2424";

void setup() {
  // put your setup code here, to run once:
  //pinMode(led_pin, OUTPUT);  //设置为输出模式
  // 初始化串口通信
  Serial.begin(9600);
  delay(10);

  //连接wifi
  WiFi.begin(ssid,password);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Successfully connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());                      //连接成功后LED会长亮5秒，并打印IP
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);
  delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
  //间隔1s闪烁
  digitalWrite(led_pin, HIGH);
  delay(1000);
  digitalWrite(led_pin, LOW);
  delay(1000);
}
