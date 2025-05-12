# 小智 AI 聊天机器人 - 开发指南

## 开发环境搭建

### 1. 安装 ESP-IDF

小智 AI 聊天机器人基于 ESP-IDF 开发，首先需要安装 ESP-IDF 开发环境：

1. 下载并安装 [ESP-IDF](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html)
2. 设置环境变量
3. 验证安装：运行 `idf.py --version`

### 2. 克隆项目

```bash
git clone https://github.com/yourusername/xiaozhi-esp32.git
cd xiaozhi-esp32
git submodule update --init --recursive
```

### 3. 编译项目

```bash
idf.py set-target esp32s3
idf.py build
```

### 4. 烧录固件

```bash
idf.py -p COM3 flash
```

### 5. 监视输出

```bash
idf.py -p COM3 monitor
```

## 项目结构

```
xiaozhi-esp32/
├── components/         # 第三方组件
├── docs/               # 文档
├── main/               # 主要源代码
│   ├── application.cc  # 应用核心
│   ├── application.h
│   ├── assets/         # 资源文件
│   ├── audio_codecs/   # 音频编解码器
│   ├── audio_processing/ # 音频处理
│   ├── background_task.cc # 后台任务
│   ├── background_task.h
│   ├── boards/         # 硬件抽象层
│   ├── display/        # 显示模块
│   ├── iot/            # IoT 集成
│   ├── led/            # LED 控制
│   ├── protocols/      # 通信协议
│   └── utils/          # 工具类
├── partitions.csv      # 分区表
└── sdkconfig           # SDK 配置
```

## 扩展指南

### 1. 添加新的硬件平台

要添加新的硬件平台，需要在 `main/boards/` 目录下创建新的板卡类：

1. 创建新的头文件和源文件，如 `my_board.h` 和 `my_board.cc`
2. 实现 `Board` 类的接口
3. 在 `main/boards/board.cc` 中添加新板卡的支持

示例：

```cpp
// my_board.h
#ifndef MY_BOARD_H
#define MY_BOARD_H

#include "board.h"

class MyBoard : public Board {
public:
    MyBoard();
    ~MyBoard();

    void Initialize() override;
    Display* GetDisplay() const override;
    AudioCodec* GetAudioCodec() const override;
    Led* GetLed() const override;
    void StartNetwork() override;
    bool IsNetworkConnected() const override;
    void OnButtonPressed(std::function<void()> callback) override;
    void OnButtonLongPressed(std::function<void()> callback) override;

private:
    // 私有成员
};

#endif
```

```cpp
// my_board.cc
#include "my_board.h"
#include "../display/my_display.h"
#include "../audio_codecs/my_audio_codec.h"
#include "../led/my_led.h"

MyBoard::MyBoard() {
    // 初始化
}

MyBoard::~MyBoard() {
    // 清理
}

void MyBoard::Initialize() {
    // 初始化硬件
}

Display* MyBoard::GetDisplay() const {
    // 返回显示器实例
    return new MyDisplay();
}

AudioCodec* MyBoard::GetAudioCodec() const {
    // 返回音频编解码器实例
    return new MyAudioCodec();
}

Led* MyBoard::GetLed() const {
    // 返回 LED 实例
    return new MyLed();
}

void MyBoard::StartNetwork() {
    // 启动网络
}

bool MyBoard::IsNetworkConnected() const {
    // 检查网络连接状态
    return true;
}

void MyBoard::OnButtonPressed(std::function<void()> callback) {
    // 设置按钮按下回调
}

void MyBoard::OnButtonLongPressed(std::function<void()> callback) {
    // 设置按钮长按回调
}
```

### 2. 添加新的显示设备

要添加新的显示设备，需要在 `main/display/` 目录下创建新的显示类：

1. 创建新的头文件和源文件，如 `my_display.h` 和 `my_display.cc`
2. 实现 `Display` 类的接口

示例：

```cpp
// my_display.h
#ifndef MY_DISPLAY_H
#define MY_DISPLAY_H

#include "display.h"

class MyDisplay : public Display {
public:
    MyDisplay();
    ~MyDisplay();

    void Initialize() override;
    void SetStatus(const char* status) override;
    void SetIcon(uint16_t icon_code) override;
    void SetEmotion(const char* emotion) override;
    void SetChatMessage(const char* role, const char* message) override;
    void ShowNotification(const char* message) override;
    void Clear() override;
    void Update() override;

private:
    // 私有成员
};

#endif
```

### 3. 添加新的音频编解码器

要添加新的音频编解码器，需要在 `main/audio_codecs/` 目录下创建新的编解码器类：

1. 创建新的头文件和源文件，如 `my_audio_codec.h` 和 `my_audio_codec.cc`
2. 实现 `AudioCodec` 类的接口

示例：

```cpp
// my_audio_codec.h
#ifndef MY_AUDIO_CODEC_H
#define MY_AUDIO_CODEC_H

#include "audio_codec.h"

class MyAudioCodec : public AudioCodec {
public:
    MyAudioCodec();
    ~MyAudioCodec();

    void SetOutputVolume(int volume) override;
    void EnableInput(bool enable) override;
    void EnableOutput(bool enable) override;

protected:
    int Read(int16_t* dest, int samples) override;
    int Write(const int16_t* data, int samples) override;

private:
    // 私有成员
};

#endif
```

### 4. 添加新的 LED 控制

要添加新的 LED 控制，需要在 `main/led/` 目录下创建新的 LED 类：

1. 创建新的头文件和源文件，如 `my_led.h` 和 `my_led.cc`
2. 实现 `Led` 类的接口

示例：

```cpp
// my_led.h
#ifndef MY_LED_H
#define MY_LED_H

#include "led.h"

class MyLed : public Led {
public:
    MyLed();
    ~MyLed();

    void Initialize() override;
    void OnStateChanged() override;

private:
    // 私有成员
};

#endif
```

### 5. 添加新的通信协议

要添加新的通信协议，需要在 `main/protocols/` 目录下创建新的协议类：

1. 创建新的头文件和源文件，如 `my_protocol.h` 和 `my_protocol.cc`
2. 实现 `Protocol` 类的接口

示例：

```cpp
// my_protocol.h
#ifndef MY_PROTOCOL_H
#define MY_PROTOCOL_H

#include "protocol.h"

class MyProtocol : public Protocol {
public:
    MyProtocol();
    ~MyProtocol();

    void Start() override;
    bool OpenAudioChannel() override;
    void CloseAudioChannel() override;
    bool IsAudioChannelOpened() const override;
    void SendAudio(const std::vector<uint8_t>& data) override;

protected:
    void SendText(const std::string& text) override;

private:
    // 私有成员
};

#endif
```

### 6. 添加新的 IoT 设备

要添加新的 IoT 设备，需要在 `main/iot/things/` 目录下创建新的设备类：

1. 创建新的头文件和源文件，如 `my_thing.h` 和 `my_thing.cc`
2. 实现 `Thing` 类的接口
3. 在应用中注册新设备

示例：

```cpp
// my_thing.h
#ifndef MY_THING_H
#define MY_THING_H

#include "../thing.h"

class MyThing : public Thing {
public:
    MyThing(const std::string& id, const std::string& name);
    ~MyThing();

    const std::string& id() const override;
    const std::string& type() const override;
    const std::string& name() const override;
    cJSON* GetDescriptor() const override;
    cJSON* GetState() const override;
    bool Invoke(const cJSON* command) override;

private:
    std::string id_;
    std::string name_;
    // 其他私有成员
};

#endif
```

## 配置指南

### 1. 修改 Wi-Fi 配置

在 `sdkconfig` 文件中修改 Wi-Fi 配置：

```
CONFIG_ESP_WIFI_SSID="your_ssid"
CONFIG_ESP_WIFI_PASSWORD="your_password"
```

或者使用 `idf.py menuconfig` 命令进行配置。

### 2. 修改服务器地址

在 `main/application.cc` 文件中修改服务器地址：

```cpp
// 设置服务器地址
protocol_->SetServerUrl("wss://your-server.com/ws");
```

### 3. 修改唤醒词

在 `main/audio_processing/wake_word_detect.cc` 文件中修改唤醒词：

```cpp
// 设置唤醒词
const char* wake_words[] = {
    "小智小智",
    "你好小智",
    // 添加更多唤醒词
};
```

### 4. 修改音频参数

在 `main/application.cc` 文件中修改音频参数：

```cpp
// 设置音频参数
opus_encoder_.Initialize(16000, 1, 16000);
opus_decoder_.Initialize(16000, 1);
```

## 调试指南

### 1. 日志级别

可以通过 `menuconfig` 或修改 `sdkconfig` 文件来调整日志级别：

```
CONFIG_LOG_DEFAULT_LEVEL_ERROR=n
CONFIG_LOG_DEFAULT_LEVEL_WARN=n
CONFIG_LOG_DEFAULT_LEVEL_INFO=y
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=n
CONFIG_LOG_DEFAULT_LEVEL_VERBOSE=n
```

### 2. 使用 GDB 调试

```bash
idf.py -p COM3 flash gdb
```

### 3. 内存调试

使用 ESP-IDF 提供的内存调试工具：

```cpp
// 打印堆内存信息
ESP_LOGI(TAG, "Free heap: %u", esp_get_free_heap_size());
ESP_LOGI(TAG, "Minimum free heap: %u", esp_get_minimum_free_heap_size());
```

### 4. 性能分析

使用 ESP-IDF 提供的性能分析工具：

```cpp
// 开始计时
int64_t start = esp_timer_get_time();

// 执行代码
// ...

// 结束计时
int64_t end = esp_timer_get_time();
ESP_LOGI(TAG, "Execution time: %lld us", end - start);
```

## 常见问题

### 1. 编译错误

**问题**：编译时出现 "undefined reference to ..." 错误。

**解决方案**：
- 检查是否包含了正确的头文件
- 检查是否链接了正确的库
- 检查是否初始化了子模块：`git submodule update --init --recursive`

### 2. 连接失败

**问题**：设备无法连接到 Wi-Fi 或服务器。

**解决方案**：
- 检查 Wi-Fi 配置是否正确
- 检查服务器地址是否正确
- 检查网络环境是否稳定
- 查看日志以获取更多信息

### 3. 音频问题

**问题**：没有声音或麦克风不工作。

**解决方案**：
- 检查音频编解码器配置是否正确
- 检查音量设置
- 检查硬件连接
- 使用示波器或逻辑分析仪检查 I2S 信号

### 4. 内存不足

**问题**：运行时出现内存不足错误。

**解决方案**：
- 优化内存使用，减少动态分配
- 增加堆大小：修改 `sdkconfig` 中的 `CONFIG_ESP_MAIN_TASK_STACK_SIZE`
- 检查是否有内存泄漏
- 使用静态分配代替动态分配

## 贡献指南

### 1. 代码风格

项目遵循 Google C++ 风格指南，请确保代码符合以下规范：

- 使用 4 空格缩进
- 类名使用 CamelCase
- 方法名和变量名使用 snake_case
- 常量使用 kConstantName
- 私有成员变量以下划线结尾：`variable_`

### 2. 提交流程

1. Fork 项目
2. 创建功能分支：`git checkout -b feature/my-feature`
3. 提交更改：`git commit -am 'Add my feature'`
4. 推送到分支：`git push origin feature/my-feature`
5. 提交 Pull Request

### 3. 测试

在提交代码前，请确保：

- 代码能够编译通过
- 功能正常工作
- 没有引入新的 bug
- 添加了必要的测试

## 资源

- [ESP-IDF 文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/index.html)
- [ESP32-S3 技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_cn.pdf)
- [FreeRTOS 文档](https://www.freertos.org/Documentation/RTOS_book.html)
- [Opus 编解码器文档](https://opus-codec.org/docs/)

## 总结

本开发指南提供了扩展和定制小智 AI 聊天机器人的详细说明，包括环境搭建、项目结构、扩展方法、配置方法、调试技巧和常见问题解决方案。通过遵循本指南，开发者可以轻松地为项目添加新功能或适配新的硬件平台。 