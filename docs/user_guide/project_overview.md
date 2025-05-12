# 小智 AI 聊天机器人 - 项目总览

## 项目简介

小智 AI 聊天机器人是一个基于 ESP32 的开源项目，旨在创建一个能够与用户进行语音交互的 AI 助手。该项目结合了语音识别、自然语言处理和物联网技术，为用户提供智能语音交互体验。

## 系统架构

小智 AI 聊天机器人采用模块化设计，主要由以下几个核心部分组成：

### 1. 应用核心层

应用核心层是整个系统的中枢，负责协调各个模块的工作，管理设备状态和处理用户交互。主要组件包括：

- **Application 类**：采用单例模式设计，管理整个应用的生命周期和状态
- **状态管理**：定义和管理设备的不同状态（空闲、监听、说话等）
- **事件处理**：处理按钮事件、网络事件和音频事件

### 2. 硬件抽象层

硬件抽象层封装了对底层硬件的操作，提供统一的接口，使上层应用能够与不同的硬件平台兼容。主要组件包括：

- **Board 类**：硬件抽象层的核心，管理硬件资源和提供硬件访问接口
- **Display 类**：管理显示功能，支持 OLED 和 LCD 等不同显示设备
- **AudioCodec 类**：管理音频输入和输出功能，支持不同的音频编解码器芯片
- **Led 类**：管理 LED 指示灯，提供视觉反馈

### 3. 音频处理层

音频处理层负责处理音频输入和输出，包括唤醒词检测、语音活动检测、音频编解码和重采样等功能。主要组件包括：

- **AudioProcessor 类**：处理音频数据，包括重采样、音量调整等
- **WakeWordDetect 类**：检测唤醒词和语音活动
- **OpusEncoder/OpusDecoder 类**：编码和解码音频数据

### 4. 通信层

通信层负责与服务器进行数据交换，支持不同的通信协议。主要组件包括：

- **Protocol 类**：通信协议的抽象基类，定义通用接口
- **WebsocketProtocol 类**：基于 WebSocket 的通信协议实现
- **MqttProtocol 类**：基于 MQTT 的通信协议实现

### 5. 系统服务层

系统服务层提供各种系统级服务，如固件更新、设置管理、IoT 集成等。主要组件包括：

- **Ota 类**：处理固件的在线更新
- **Settings 类**：管理用户设置
- **ThingManager 类**：管理 IoT 设备
- **BackgroundTask 类**：处理后台任务

## 数据流

### 音频输入流

1. **采集音频**：从音频编解码器获取 PCM 音频数据
2. **唤醒词检测**：检测唤醒词和语音活动
3. **音频处理**：重采样、音量调整等
4. **音频编码**：将 PCM 数据编码为 Opus 格式
5. **发送音频**：将编码后的音频数据发送到服务器

### 音频输出流

1. **接收音频**：从服务器接收 Opus 格式的音频数据
2. **音频解码**：将 Opus 数据解码为 PCM 格式
3. **音频处理**：重采样、音量调整等
4. **播放音频**：将处理后的音频数据发送到音频编解码器进行播放

### 控制流

1. **用户输入**：按钮按下或唤醒词检测
2. **状态变更**：应用核心层更新设备状态
3. **模块响应**：各模块根据状态变更执行相应操作
4. **服务器交互**：与服务器交换控制命令和状态信息

## 状态管理

设备有多种状态，由 `DeviceState` 枚举定义：

- `kDeviceStateUnknown`：未知状态
- `kDeviceStateStarting`：启动中
- `kDeviceStateWifiConfiguring`：Wi-Fi 配置中
- `kDeviceStateIdle`：空闲状态
- `kDeviceStateConnecting`：连接中
- `kDeviceStateListening`：监听中
- `kDeviceStateSpeaking`：说话中
- `kDeviceStateUpgrading`：升级中
- `kDeviceStateActivating`：激活中
- `kDeviceStateFatalError`：致命错误

状态转换由 `Application::SetDeviceState()` 方法控制，不同状态下系统会有不同的行为。

## 模块交互

### 应用核心与硬件抽象层

应用核心通过硬件抽象层访问硬件资源：

```cpp
auto& board = Board::GetInstance();
auto display = board.GetDisplay();
auto codec = board.GetAudioCodec();
auto led = board.GetLed();
```

### 应用核心与音频处理层

应用核心使用音频处理层处理音频数据：

```cpp
// 唤醒词检测
wake_word_detect_.OnWakeWordDetected([this](const std::string& wake_word) {
    // 处理唤醒事件
});

// 音频编码
std::vector<uint8_t> encoded;
opus_encoder_.Encode(pcm_data_, encoded);
```

### 应用核心与通信层

应用核心使用通信层与服务器交换数据：

```cpp
// 发送音频
protocol_->SendAudio(encoded_data);

// 接收音频
protocol_->OnIncomingAudio([this](std::vector<uint8_t>&& data) {
    // 处理接收到的音频数据
});
```

### 应用核心与系统服务层

应用核心使用系统服务层提供的各种服务：

```cpp
// 检查新版本
if (ota_.CheckVersion()) {
    if (ota_.HasNewVersion()) {
        // 处理新版本
    }
}

// 调度后台任务
background_task_.Schedule([this]() {
    // 执行耗时操作
});
```

## 扩展性设计

小智 AI 聊天机器人采用了多种设计模式和技术，使项目具有良好的扩展性：

### 1. 抽象基类和接口

项目使用抽象基类和接口定义通用接口，使不同的实现可以互换：

- `Display` 类：显示设备的抽象基类
- `AudioCodec` 类：音频编解码器的抽象基类
- `Led` 类：LED 控制的抽象基类
- `Protocol` 类：通信协议的抽象基类
- `Thing` 类：IoT 设备的抽象基类

### 2. 单例模式

项目使用单例模式管理全局资源和状态：

- `Application` 类：应用核心
- `Board` 类：硬件抽象层
- `ThingManager` 类：IoT 设备管理
- `Settings` 类：用户设置管理

### 3. 回调机制

项目使用回调机制实现模块间的松耦合通信：

- 唤醒词检测回调
- 语音活动检测回调
- 音频输入/输出就绪回调
- 按钮事件回调
- 网络事件回调

### 4. 事件驱动

项目使用事件驱动模型处理异步事件：

- FreeRTOS 事件组
- 条件变量
- 任务通知

## 多平台支持

小智 AI 聊天机器人支持多种硬件平台，包括：

- ESP32-S3-BOX
- M5Stack CoreS3
- LILYGO T-Circle-S3
- 立创实战派
- 自定义硬件

通过硬件抽象层，项目能够在不同的硬件平台上运行，而无需修改上层应用代码。

## 多语言支持

项目支持多语言界面，语言资源存储在 JSON 文件中，通过 `Lang::Strings` 命名空间访问当前语言的字符串。

## 总结

小智 AI 聊天机器人是一个功能丰富、架构清晰的开源项目，它结合了语音识别、自然语言处理和物联网技术，为用户提供智能语音交互体验。通过模块化设计和良好的扩展性，项目能够适应不同的硬件平台和应用场景，为开发者提供了一个灵活的 AI 助手开发框架。 