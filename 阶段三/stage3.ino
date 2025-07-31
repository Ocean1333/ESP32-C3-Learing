#ifndef Wire1
  #define Wire1 Wire
#endif
#include<WiFi.h>
#include<WebServer.h>

#include<Adafruit_BME280.h>
#include<Adafruit_Sensor.h>
#include<Wire.h>
#include<SSD1306.h>
// Led & Wifi
const int led_pin = 8;
const char* ssid = "CMCC-YcDN";
const char* password = "yfyx2424";
WebServer server(80);
// Screen & BME280
const int SCL_pin = 5;
const int SDA_pin = 4;
SSD1306Wire display(0x3c, SDA_pin, SCL_pin, GEOMETRY_128_64, I2C_ONE); //led
Adafruit_BME280 bme;
// millis()
unsigned long lastMillis = 0;
const unsigned long interval = 1000;

void startLED()
{
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
}

void connectWiFi()
{
  //连接wifi
  WiFi.begin(ssid,password);
  // 打印到串口
  Serial.print("   ");
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500); //未连接WIFI时等待
    Serial.print(".");
  }
  // 打印IP
  Serial.println("Successfully connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void handleRoot()
{
  String HTML = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Esp32WebServer</title>
    <style>
        div {text-align: center; align-items: center; background-color: #ddd; width: 100%;}
        button {margin: 5px;}
    </style>
</head>
<body>
    <h1 style="text-align: center;">Welcome To Esp32WebServer!</h1>
    <div>
        <span>LED state: </span>
        <span id="led_state" style="color: red">On</span>
        <button onclick="switchLed('on')">TurnOn</button>
        <button onclick="switchLed('off')">TurnOff</button>
        <br>
        <span>Temperature: </span>
        <span id="Temperature" style="color: blue;">detecting..</span>
        <br>
        <span>Humidity: </span>
        <span id="Humidity" style="color: blue;">detecting..</span>
        <br>
    </div>
    <script>
        var http = new XMLHttpRequest();
        //每秒获取LED状态
        function getLedState(){
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4 && xhr.status == 200){
                    var data = xhr.responseText;
                    document.getElementById("led_state").innerHTML = data;
                }
            }
            xhr.open("GET", "/ledstate", true);
            xhr.send();
        }
        setInterval(getLedState, 1000);
        //开关灯
        function switchLed(state){
            http.open("GET","/led?switch=" + state, true);
            http.send();
        }
        //温湿度
        function getTemperature_and_Humidity(){
            var xhr1 = new XMLHttpRequest();
            var xhr2 = new XMLHttpRequest();
            xhr1.onreadystatechange = function() {
                if (xhr1.readyState == 4 && xhr1.status == 200){
                    var data1 = xhr1.responseText;
                    var arr = data1.split('!');
                    document.getElementById("Temperature").innerText = arr[0] + "*C";
                    document.getElementById("Humidity").innerText = arr[1] + "%";
                }
            }
            xhr1.open("GET", "/Temperature_and_Humidity", true);
            xhr1.send();
        }
        setInterval(getTemperature_and_Humidity, 1000);
    </script>
</body>
</html>)rawliteral";
  server.send(200, "text/html", HTML);
}

void handleNotfound()
{
  server.send(200, "text/plain", "Web Not Found!");
}

void ledSwitch()
{
  String ledswitch = server.arg("switch");
  if (ledswitch == "on")
    digitalWrite(led_pin, HIGH);
  else if (ledswitch == "off")
    digitalWrite(led_pin, LOW);
  server.send(200, "text/plain", "led is now " + ledswitch);
}

void handleLedState()
{
  if (digitalRead(led_pin))
    server.send(200, "text/html", "<span id='led_state' style='color: red'>On</span>");
    else
    server.send(200, "text/html", "<span id='led_state' style='color: blue'>Off</span>");
  // String state = digitalRead(led_pin)? "On" : "Off";
  // server.send(200, "text/html", "<span id='led_state' style='color: red'>" + state + "</span>"); 废案
}

void handleTemperature_and_Humidity()
{
    float Temperature = bme.readTemperature();
    float Humidity = bme.readHumidity();
    // SERVER
    server.send(200, "text/plain", String(Temperature, 1) + "!" + String(Humidity, 1));
}

void createServer()
{
  server.on("/", handleRoot); //主页面
  server.onNotFound(handleNotfound);
  server.on("/led", ledSwitch); //开关
  server.on("/ledstate", handleLedState); //LED状态
  server.on("/Temperature_and_Humidity", handleTemperature_and_Humidity); //查询温湿度
//   server.on("/Humidity", handleHumidity); //湿度
  
  server.begin();
  Serial.println("HTTP server started");
}

void startOLED()
{
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16); //Plain后面是像素大小
}

void OLEDupdate() //用millis而非delay防止服务器因每秒刷新OLED等待而卡顿
{
    float Temperature, Humidity;
    if (millis() - lastMillis >= interval)
    {
        lastMillis = millis();
        //BME
        Temperature = bme.readTemperature();
        Humidity = bme.readHumidity();
        // OLED
        display.clear();
        display.drawString(0, 0, "T: " + String(Temperature, 1) + "*C");
        display.drawString(0, 16, "H: " + String(Humidity, 1) + "%");
        display.display();
    }
}

void setup(){
  //初始化波特率
  Serial.begin(9600);
  delay(10);

  bme.begin(0x76); //启动BME
  startLED(); //LED灯
  startOLED(); //屏幕
  connectWiFi(); //WIFI
  createServer(); //服务器
}

void loop(){
  server.handleClient();
  OLEDupdate();
}