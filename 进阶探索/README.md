# 进阶探索-蜂鸣器
## 简介
本阶段为ESP32添加了蜂鸣器模块，在控制Led灯打开时会播放一段音乐

## 硬件准备
* 无源蜂鸣器*1
* 工作原理：当给无源蜂鸣器施加直流电压时，内部的压电陶瓷会产生震动，震动的频率和给电流的频率有关，于是就可以通过控制GPIO口上的电流频率来控制声音的音调
* 硬件连接：将VCC口接到ESP的5V口，GND口接GND，再连上GPIO口

## 实现过程 
* `tone`函数语法：`tone(GPIO_pin, unsigned int frequency, unsigned long duration)`，（持续时间的单位是毫秒）也就是说`tone`函数可以向指定的GPIO口产生一定时间的指定的频率
* 同时，Arduino官网提供了包含钢琴88键对应的频率的库`pitched.h`，导入后，就可以将简谱的音调对应到琴键上，再对应到频率上，并且可以根据谱，将每个音的间隔换算成以毫秒为单位，在写进对应的两个数组，最后使用一个for函数逐个音播放，便实现了音乐的播放
* 为了实现开灯时播放音乐，新增了一个大小为1个`int`的队列，修改了LED控制模块的函数，使其在开灯时向这个队列发送一个数字，与此同时，蜂鸣器模块的任务不断向任务获取消息（等待时间1s），一旦成功获取到信息，就执行播放音乐的函数，到这就完成了开灯时播放音乐的目标
> 为此还去特意学了下怎么读简谱XD

## 参考资料
[Arduino: 使用tone函数播放一段音乐](https://docs.arduino.cc/built-in-examples/digital/toneMelody/)
[CSDN：使用Arduino tone()演奏旋律](https://blog.csdn.net/acktomas/article/details/116121814)

