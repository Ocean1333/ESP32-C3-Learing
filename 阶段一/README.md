# 阶段一
## 环境搭建过程
1. 从官网下载并安装Arduino [官网](https://www.arduino.cc/)
2. 下载并安装ESP32库文件
    <!-- [^1^]这段过程比较顺利 -->
## 控制LED
点亮LED的过程遇到了一些问题
1. 连接USB后编译器找不到开发板
    询问AI后发现是有个USB驱动没安装，安装后成功识别到USB接口，并连接编译器
2. 接好电路后烧录完代码，灯泡不亮，或是不按代码写的运行
    依照图片标注的接口搞错了，pinMode()函数传的int是指GPIO接口，当成图上标注接口了
## 连接WiFi
通过查询Arduino编译器的WIFI示例，以及观看B站上视频后初步了解了WIFI的使用方式
1. 一开始不知道为什么要初始化波特率，查询AI后得知
2. 编译器找不到输出界面，查询资料后得知要从串口监视器中查看
## 最终代码
```Arduino
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
```