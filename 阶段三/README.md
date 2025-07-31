# 阶段三

## 简介

本项目在阶段二的基础上，使用`I2C/SPI通信`读取`BME280`传感器数据，并将数据实时显示在`SSD1306 OLED`屏幕和`Web服务器`上。

---

## 学习过程
1. **硬件介绍**
   1. `BME280` 有六个引脚,用到了四个: GND(接地),  VCC(供电), SCL(时钟线), SDA(数据线)
   2. `SSD1306` 四个引脚都用上了: GND(接地),  VCC(供电), SCL(时钟线), SDA(数据线)
      * 通过时钟线和数据线两线的电平高低来控制数据传输
      * 主机通过发送从机地址（7位常见）选择通信对象
      * SDA仅在SCL低电平时变化,SDA在SCL高电平时保持稳定
      * 均采用开漏输出结构，需外接上拉电阻
      * 流程: 
        * SCL高电平时，SDA由高→低跳变，标志传输开始。
        * 主机发送7位从机地址 + 1位方向位（0:写, 1:读）。
        * 每帧（8位）后，接收方拉低SDA表示ACK，否则为NACK（通信终止）。
        * 主机或从机在SCL同步下发送数据（每字节后跟随ACK/NACK）
        * SCL高电平时，SDA由低→高跳变，结束传输。
2. **点亮OLED**
   * 准备工作：下载必要的库文件: `ESP8266 and ESP32 OLED driver for SSD1306 displays` 和 `U8g2`
   * 确定`SCL`和`SDA`对应引脚并创建对象: `SSD1306Wire display(0x3c, SDA_pin, SCL_pin, GEOMETRY_128_64, I2C_ONE);`对应含义: (I2C地址,SDA引脚,SCL引脚,屏幕参数,总线接口)
   * 初始化，设置字体等
   * 然后使用`display.clear`-清屏 `display.drawString`-写字符串 `display.display`-显示 三种函数控制屏幕显示
      >成功打印Hello World (doge)
3. **传感器读取数据**
   * 准备工作: 下库: `Adafruit BME280 Library` `Adafruit Unified Sensor`
   * 创建I2C协议对应的对象 `Adafruit_BME280 bme;`
   * 在`setup`里初始化: `bme.begin(0x76);` (begin内的是BME280的I2C地址)
   * 用 `bme.readTemperature` `bme.readHumidity` 读取温湿度数据 (温度为摄氏度，湿度为相对湿度百分比),使用两个浮点数来保存
4. **控制网页和屏幕实时显示**
   * 网页部分：大体同阶段二的实时显示LED状态，使用Ajax轮询
   * OLED部分：由于不能在`loop`里加`delay(1000)`，否则网页会卡，故使用`millis`函数做非阻塞定时任务，详见**遇到的问题与解决方案**部分

---

## 遇到的问题与解决方案
* **网页与屏幕同步显示数据问题**
  * 原因：最初的设想是：既然已经添加了`handleTemperature`这样的响应函数，而且发现在`loop`里加`delay`会让cpu强制等待导致服务器变卡，那就把OLED的控制也放在这个响应函数里，这样在实现Ajax轮询的同时也实现了OLED的显示
    * 但是清屏只能有一次，所以不能把温湿度分开用两个响应函数，所以修改了响应函数，将温湿度字符串拼接，用 `!` 连接然后切割，初步实现了网页与OLED同步显示
    * **但是**这样的处理方式**仍有问题** ---> 不访问网页时，OLED的数据也会跟着不动了
    * **最终解决方案：** 使用`millis`函数做非阻塞定时任务 ---> `millis` 会记录esp32的当前运行时间，可以设置一个 `时间间隔` 和 `上次记录时间` ，`当前时间` - `上次时间` < `时间间隔` 时不执行，实现了定时的效果，并且这个判断语句的执行时间非常短，不会影响loop函数中的其他函数
      > 同时millis的数据是`unsign int`，数据溢出是取模运算，就算溢出运行仍正常
* **在PlatformIO上 编译通过但在Arduino编译器上不通过**
  * 起因: 由于`Arduino`原生编译器实在是不好用,所以一直都是用`PlatformIO`写和运行,然后再到`Arduino`编译器里看看正常与否
  * 问题原因: 
    1. `PlatformIO` 和 `Arduino IDE` 可能使用了 不同的开发板配置。PlatformIO中默认启用了双I2C接口，而Arduino IDE的板型配置可能仅启用了单I2C。
    2. 库存在硬件适配问题
   * 解决方案: 
     * 强制单总线: `SSD1306Wire display(0x3c, SDA_pin, SCL_pin, GEOMETRY_128_64, I2C_ONE);`
     * 在代码最开头添加: 
         ```
         #ifndef Wire1
         #define Wire1 Wire
         #endif
         ```

---

## 参考资料
* [带有BME280的ESP32 Web服务器–迷你气象站](https://www.bilibili.com/video/BV1rE41167no/?spm_id_from=333.337.search-card.all.click&vd_source=9c4a7a24a570d4b1fd3035947cdaa6c0)
* [ESP32+Arduino入门教程（二）：连接OLED屏](https://www.cnblogs.com/mingupupu/p/18818250)
* Arduino示例: `[Adafruit BME280 Library/bme280test]` `[ESP8266 and ESP32 OLED driver for SSD1306 displays/SSD1306SimpleDemo]`