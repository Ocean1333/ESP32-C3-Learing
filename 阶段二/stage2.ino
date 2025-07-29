#include<WiFi.h>
#include<WebServer.h>

int led_pin = 8;
const char* ssid = "CMCC-YcDN";
const char* password = "yfyx2424";
WebServer server(80);

void connectWiFi()
{
  //初始化波特率
  Serial.begin(9600);
  delay(10);

  //连接wifi
  WiFi.begin(ssid,password);
  Serial.print("   ");
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

void handleRoot()
{
  String HTML = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
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
    </div>
    <script>
        var http = new XMLHttpRequest();
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
        function switchLed(state){
            http.open("GET","/led?switch=" + state, true);
            http.send();
            }
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
  // server.send(200, "text/html", "<span id='led_state' style='color: red'>" + state + "</span>");
}

void createServer()
{
  server.on("/", handleRoot);
  server.onNotFound(handleNotfound);
  server.on("/led", ledSwitch);
  server.on("/ledstate", handleLedState);

  server.begin();
  Serial.println("HTTP server started");

}

void setup() {
  // put your setup code here, to run once:
  connectWiFi();
  createServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  delay(2);
}