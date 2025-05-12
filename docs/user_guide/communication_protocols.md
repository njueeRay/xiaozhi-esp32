# 小智 AI 聊天机器人 - 通信协议解析

## 通信协议概述

小智 AI 聊天机器人支持两种通信协议：WebSocket 和 MQTT。这些协议用于与服务器进行数据交换，包括发送音频数据、接收服务器回复、发送和接收控制命令等。

## 协议基类 (Protocol)

`Protocol` 是一个抽象基类，定义了通信协议的通用接口。

```cpp
class Protocol {
public:
virtual ~Protocol() = default;
// 获取服务器采样率
inline int server_sample_rate() const;
// 获取会话ID
inline const std::string& session_id() const;
// 设置回调
void OnIncomingAudio(std::function<void(std::vector<uint8_t>&& data)> callback);
void OnIncomingJson(std::function<void(const cJSON root)> callback);
void OnAudioChannelOpened(std::function<void()> callback);
void OnAudioChannelClosed(std::function<void()> callback);
void OnNetworkError(std::function<void(const std::string& message)> callback);
// 纯虚函数，需要子类实现
virtual void Start() = 0;
virtual bool OpenAudioChannel() = 0;
virtual void CloseAudioChannel() = 0;
virtual bool IsAudioChannelOpened() const = 0;
virtual void SendAudio(const std::vector<uint8_t>& data) = 0;
// 虚函数，有默认实现
virtual void SendWakeWordDetected(const std::string& wake_word);
virtual void SendStartListening(ListeningMode mode);
virtual void SendStopListening();
virtual void SendAbortSpeaking(AbortReason reason);
virtual void SendIotDescriptors(const std::string& descriptors);
virtual void SendIotStates(const std::string& states);
protected:
// 回调函数
std::function<void(const cJSON root)> on_incoming_json;
std::function<void(std::vector<uint8_t>&& data)> on_incoming_audio_;
std::function<void()> on_audio_channel_opened_;
std::function<void()> on_audio_channel_closed_;
std::function<void(const std::string& message)> on_network_error_;
// 服务器采样率和会话ID
int server_sample_rate_ = 16000;
std::string session_id_;
// 纯虚函数，发送文本
virtual void SendText(const std::string& text) = 0;
};
```

### 核心方法

- `Start()`: 启动协议
- `OpenAudioChannel()`: 打开音频通道
- `CloseAudioChannel()`: 关闭音频通道
- `IsAudioChannelOpened()`: 检查音频通道是否已打开
- `SendAudio()`: 发送音频数据
- `SendStartListening()`: 发送开始监听命令
- `SendStopListening()`: 发送停止监听命令
- `SendWakeWordDetected()`: 发送唤醒词检测事件
- `SendAbortSpeaking()`: 发送中断说话命令
- `SendIotDescriptors()`: 发送 IoT 设备描述符
- `SendIotStates()`: 发送 IoT 设备状态

### 回调设置

- `OnNetworkError()`: 设置网络错误回调
- `OnIncomingAudio()`: 设置接收音频数据回调
- `OnIncomingJson()`: 设置接收 JSON 数据回调
- `OnAudioChannelOpened()`: 设置音频通道打开回调
- `OnAudioChannelClosed()`: 设置音频通道关闭回调

## WebSocket 协议 (WebsocketProtocol)

`WebsocketProtocol` 类实现了基于 WebSocket 的通信协议。

### 核心功能

1. **连接管理**：
   - 建立和维护与服务器的 WebSocket 连接
   - 处理连接错误和重连

2. **数据传输**：
   - 发送和接收二进制数据（音频）
   - 发送和接收 JSON 数据（控制命令）

3. **通道管理**：
   - 打开和关闭音频通道
   - 管理通道状态

### 数据格式

WebSocket 协议使用两种数据格式：

1. **二进制格式**：用于传输音频数据，使用自定义的二进制协议。
2. **JSON 格式**：用于传输控制命令和状态信息。

## MQTT 协议 (MqttProtocol)

`MqttProtocol` 类实现了基于 MQTT 的通信协议。

### 核心功能

1. **连接管理**：
   - 建立和维护与 MQTT 代理的连接
   - 处理连接错误和重连

2. **主题订阅和发布**：
   - 订阅接收数据的主题
   - 发布数据到相应的主题

3. **通道管理**：
   - 打开和关闭音频通道
   - 管理通道状态

### 数据格式

MQTT 协议也使用两种数据格式：

1. **二进制格式**：用于传输音频数据，使用与 WebSocket 相同的二进制协议。
2. **JSON 格式**：用于传输控制命令和状态信息。

## 二进制协议

项目使用自定义的二进制协议 `BinaryProtocol3` 传输音频数据：

```cpp
struct BinaryProtocol3 {
uint8_t type;
uint8_t reserved;
uint16_t payload_size;
uint8_t payload[];
} attribute((packed));
```

- `type`: 数据类型
- `reserved`: 保留字段
- `payload_size`: 负载大小
- `payload`: 负载数据（变长）

## 通信流程

### 音频通道建立

1. 应用调用 `OpenAudioChannel()`
2. 协议实现建立连接
3. 连接成功后调用 `on_audio_channel_opened_` 回调

### 音频数据发送

1. 应用调用 `SendAudio()` 发送 Opus 编码的音频数据
2. 协议实现将数据打包并发送

### 控制命令发送

1. 应用调用相应的方法（如 `SendStartListening()`）
2. 协议实现将命令转换为 JSON 并发送

### 数据接收

1. 协议实现接收数据
2. 根据数据类型调用相应的回调（`on_incoming_audio_` 或 `on_incoming_json_`）

### 音频通道关闭

1. 应用调用 `CloseAudioChannel()`
2. 协议实现关闭连接
3. 连接关闭后调用 `on_audio_channel_closed_` 回调

## 错误处理

协议实现会监控连接状态，当发生错误时调用 `on_network_error_` 回调，传递错误信息。应用可以根据错误信息采取相应的措施，如重新连接或显示错误消息。

## 总结

通信协议模块是小智 AI 聊天机器人的关键组件，它负责与服务器进行数据交换，实现语音交互功能。通过抽象基类 `Protocol` 和具体实现 `WebsocketProtocol` 和 `MqttProtocol`，项目实现了灵活的通信机制，支持不同的通信协议。

