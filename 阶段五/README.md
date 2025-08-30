# 阶段五
## 简介
* 本阶段使用Arduino下的FreeRTOS将 传感器读取、数据显示、MQTT通信、LED控制 功能拆分为了四个独立的FreeRTOS任务

## 学习过程
* 上网查找FreeRTOS的介绍和教程
* 学习了基本的创建任务方法，并尝试了多任务点灯
* 点灯成功后学习任务间的通信方法
* 结合任务间通信方法构思各个模块的分块和运行机制，并编写各任务的运行函数

## FreeRTOS学习
1. 任务--FreeRTOS最基本的执行单元，任务的执行是独立的
   1. 使用`xTaskCreate(vTaskFunction, "TaskName", STACK_SIZE, Parameters, TASK_PRIORITY, NULL);`函数创建任务`vTaskFunction(void *pvParameters)`任务函数必须有一个`void*`的参数
   2. 任务有多种状态
      1. 准备就绪（Ready）：任务已经准备好运行，只要调度器分配CPU时间就可执行
      2. 运行中（Running）：任务正在运行
      3. 阻塞（Blocked）：任务因等待某事件暂时停止运行
      4. 挂起（Suspended）：无限期停止运行知道被恢复
2. 任务调度
   1. FreeRTOS 采用的是抢占式调度算法。高优先级的任务可以打断低优先级的任务运行。调度器会根据任务的优先级和状态，选择当前最高优先级的就绪任务来运行。（当一个高优先级的任务从阻塞态变为就绪态时，调度器会立即切换到这个高优先级任务运行，即使当前有低优先级任务正在运行。）
3. 任务间通信
   1. 队列
      1. `QueueHandle_t xQueue = xQueueCreate(QUEUE_SIZE, sizeof(DataType));`创建队列
      2. `xQueueSend(xQueue, &data, 0);`向队列发送数据
      3. `xQueueReceive(xQueue, &data, 0)`接收数据，最后一项参数是阻塞时间，可以设置在没收到消息时等待多少Tick
   2. 信号量
      * 一种同步机制，用于控制多个任务对共享资源的访问，或者用于任务之间的同步。
      1. 二进制信号量：只有两种状态，用于任务间的同步，一个任务获取信号量一个任务释放信号量
        ```
        SemaphoreHandle_t xSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(xSemaphore);
        if (xSemaphoreTake(xSemaphore, 0)) {}
        ```
      2. 计数信号量：可以有多个计数，用于控制对共享资源的访问。例如，如果有多个任务可以访问一个共享资源，可以使用计数信号量来限制同时访问的任务数量。
        ```
        SemaphoreHandle_t xSemaphore = xSemaphoreCreateCounting(MAX_COUNT, INITIAL_COUNT);
        xSemaphoreGive(xSemaphore);
        if (xSemaphoreTake(xSemaphore, 0)) {}
        ```
      3. 互斥信号量（Mutex）：用于保护共享资源，防止多个任务同时访问。互斥信号量与普通信号量的区别在于，它支持优先级继承，防止优先级反转问题。
        ```
        SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
        xSemaphoreTake(xMutex, 0);
        xSemaphoreGive(xMutex);
        ```

## 任务划分
1. 传感器读取：将传感器读取放在一个任务中，通过向队列发送信息，或是修改全局变量（用Mutex信号量防止同时访问）实现数据的读取和发送
2. OLED模块：将OLED显示单独放在一个任务，向队列获取信息并显示内容
3. MQTT模块：创建一个任务专门负责MQTT的连接，订阅，数据上报和指令接收
4. LED控制模块：接收指令，并执行开关灯

## 任务间通信
* 主要通过任务队列实现，队列难以实现的部分使用了Mutex+全局变量
  * 传感器模块每隔1s读取数据，将数据转为json格式后，向队列1发送数据（用于OLED），并修改全局变量（用于MQTT，在遇到的问题部分会细讲）
  * OLED模块接收json数据并转换为字符串形式显示（接收时不必间隔一秒，因为传感器模块已经设置了时间间隔，不用重复设置）
  * MQTT模块分为两部分，一部分借助millis函数每5s读取一次全局变量并上传，另一部分是回调函数，接收到下发的LED控制消息后通过另一个任务队列发送到LED控制模块
  * LED模块接收数据并将字符串转为json格式读取数据并控制LED

## 遇到的问题及解决方案
* 想要MQTT每5s上传一次，但如果使用任务队列通信，传感器每1s发送数据而接收方每5s接收就会导致队列大部分情况下都是满的，并且MQTT上传的消息不是实时读取的数据
  * 解决方案：修改传感器模块，不通过消息队列与MQTT通信，而是在Mutex保护下修改全局变量，这样MQTT模块每5s读取的数据就是实时的了

## 参考资料
[RTOS队列](https://blog.csdn.net/SlientICE/article/details/149064131?ops_request_misc=&request_id=&biz_id=102&utm_term=esp32%E4%BB%BB%E5%8A%A1%E9%97%B4%E9%80%9A%E4%BF%A1&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-4-149064131.142^v102^pc_search_result_base2&spm=1018.2226.3001.4187)
[ESP32上的FreeRTOS](https://www.bilibili.com/video/BV1q54y1Z7ca/?spm_id_from=333.1007.top_right_bar_window_custom_collection.content.click&vd_source=9c4a7a24a570d4b1fd3035947cdaa6c0)
[FreeRTOS官网](https://www.freertos.org/zh-cn-cmn-s/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/01-RTOS-fundamentals)
[FreeRTOS教程](https://blog.csdn.net/qq_61672347/article/details/125748646)
