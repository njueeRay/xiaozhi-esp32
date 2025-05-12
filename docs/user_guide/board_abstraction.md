# 小智 AI 聊天机器人 - 硬件抽象层解析

## 硬件抽象层概述

硬件抽象层是小智 AI 聊天机器人的基础组件，它封装了对底层硬件的操作，提供了统一的接口，使上层应用能够与不同的硬件平台兼容。项目支持多种硬件平台，如 ESP32-S3-BOX、M5Stack CoreS3、立创实战派等。

## 核心组件

### 1. Board 类

`Board` 类是硬件抽象层的核心，采用单例模式设计，负责管理硬件资源和提供硬件访问接口。 

```cpp
class Board {
public:
static Board& GetInstance();
// 获取硬件信息
const char GetBoardType() const;
const char GetUuid() const;
std::string GetJson() const;
// 获取硬件组件
Display GetDisplay() const;
AudioCodec GetAudioCodec() const;
Led GetLed() const;
// 网络相关
void StartNetwork();
bool IsNetworkConnected() const;
void SetPowerSaveMode(bool enable);
// 按钮相关
void OnButtonPressed(std::function<void()> callback);
void OnButtonLongPressed(std::function<void()> callback);
private:
// ... 私有成员 ...
};
```

### 2. 具体板卡实现

项目在 `main/boards/` 目录下实现了多种具体的板卡类，每种板卡类负责初始化和管理特定硬件平台的资源。

## 工作流程

### 初始化

1. 在应用启动时，获取 Board 实例并初始化硬件：

```cpp
auto& board = Board::GetInstance();
```

### 获取硬件组件

1. 通过 Board 实例获取硬件组件：

```cpp
auto display = board.GetDisplay();
auto codec = board.GetAudioCodec();
auto led = board.GetLed();
```

### 网络连接

1. 启动网络连接：

```cpp
board.StartNetwork();
```

### 按钮事件处理

1. 设置按钮事件回调：

```cpp
board.OnButtonPressed([this]() {
    ToggleChatState();
});

board.OnButtonLongPressed([this]() {
    StartListening();
});
```

## 硬件组件

### 显示器

Board 类负责创建和管理适合当前硬件平台的显示器实例：

```cpp
Display* Board::GetDisplay() const {
    // 根据板卡类型返回相应的显示器实例
    // ...
}
```

### 音频编解码器

Board 类负责创建和管理适合当前硬件平台的音频编解码器实例：

```cpp
AudioCodec* Board::GetAudioCodec() const {
    // 根据板卡类型返回相应的音频编解码器实例
    // ...
}
```

### LED

Board 类负责创建和管理适合当前硬件平台的 LED 实例：

```cpp
Led* Board::GetLed() const {
    // 根据板卡类型返回相应的 LED 实例
    // ...
}
```

### 网络

Board 类负责管理网络连接，支持 Wi-Fi 和 4G（ML307）两种连接方式：

```cpp
void Board::StartNetwork() {
    // 根据板卡类型启动相应的网络连接
    // ...
}
```

### 按钮

Board 类负责管理按钮事件，支持按下和长按两种触发方式：

```cpp
void Board::OnButtonPressed(std::function<void()> callback) {
    // 设置按钮按下回调
    // ...
}

void Board::OnButtonLongPressed(std::function<void()> callback) {
    // 设置按钮长按回调
    // ...
}
```

## 板卡配置

项目支持多种硬件平台，每种平台有自己的配置和初始化方式。板卡配置通常包括：

- 引脚定义：定义各个硬件组件使用的 GPIO 引脚
- 硬件参数：如显示器分辨率、音频采样率等
- 初始化顺序：定义硬件组件的初始化顺序

## 总结

硬件抽象层是小智 AI 聊天机器人的基础组件，它使项目能够在多种硬件平台上运行，而无需修改上层应用代码。通过 `Board` 类和具体板卡实现，项目实现了灵活的硬件抽象机制，支持多种硬件平台和配置。