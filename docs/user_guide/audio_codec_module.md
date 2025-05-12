# 小智 AI 聊天机器人 - 音频编解码器模块解析

## 音频编解码器概述

音频编解码器模块负责管理小智 AI 聊天机器人的音频输入和输出功能，包括麦克风采集和扬声器播放。项目支持多种音频编解码器芯片，如 ES8388、ES8311 等。

## 核心组件

### 1. AudioCodec 类

`AudioCodec` 类是音频编解码器的抽象基类，定义了音频功能的通用接口。 

```cpp
class AudioCodec {
public:
AudioCodec();
virtual ~AudioCodec();
virtual void SetOutputVolume(int volume);
virtual void EnableInput(bool enable);
virtual void EnableOutput(bool enable);
void Start();
void OutputData(std::vector<int16_t>& data);
bool InputData(std::vector<int16_t>& data);
void OnOutputReady(std::function<bool()> callback);
void OnInputReady(std::function<bool()> callback);
inline bool duplex() const { return duplex_; }
inline bool input_reference() const { return input_reference_; }
inline int input_sample_rate() const { return input_sample_rate_; }
inline int output_sample_rate() const { return output_sample_rate_; }
inline int input_channels() const { return input_channels_; }
inline int output_channels() const { return output_channels_; }
inline int output_volume() const { return output_volume_; }
protected:
// ... 保护成员 ...
virtual int Read(int16_t dest, int samples) = 0;
virtual int Write(const int16_t data, int samples) = 0;
};
```

### 核心方法

- `SetOutputVolume()`: 设置输出音量
- `EnableInput()`: 启用或禁用音频输入
- `EnableOutput()`: 启用或禁用音频输出
- `Start()`: 启动音频编解码器
- `OutputData()`: 输出音频数据
- `InputData()`: 获取输入音频数据
- `OnOutputReady()`: 设置输出就绪回调
- `OnInputReady()`: 设置输入就绪回调
- `Read()`: 读取音频数据（纯虚函数，需要子类实现）
- `Write()`: 写入音频数据（纯虚函数，需要子类实现）

### 2. 具体编解码器实现

项目实现了多种具体的音频编解码器类，以支持不同的硬件：

- `ES8388AudioCodec`: 基于 ES8388 芯片的音频编解码器
- `ES8311AudioCodec`: 基于 ES8311 芯片的音频编解码器
- `CoreS3AudioCodec`: 适用于 M5Stack CoreS3 的音频编解码器
- `BoxAudioCodec`: 适用于 ESP-BOX 的音频编解码器
- `TCircleS3AudioCodec`: 适用于 LILYGO T-Circle-S3 的音频编解码器
- `NoAudioCodec`: 空实现，用于不支持音频功能的设备

这些类继承自 `AudioCodec` 基类，实现了特定硬件的音频输入和输出功能。

## 工作流程

### 初始化

1. 在应用启动时，创建并初始化音频编解码器：

```cpp
auto codec = board.GetAudioCodec();
codec->Start();
```

### 设置回调

1. 设置音频输入和输出就绪的回调函数：

```cpp
codec->OnInputReady([this, codec]() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    xEventGroupSetBitsFromISR(event_group_, AUDIO_INPUT_READY_EVENT, &higher_priority_task_woken);
    return higher_priority_task_woken == pdTRUE;
});

codec->OnOutputReady([this]() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    xEventGroupSetBitsFromISR(event_group_, AUDIO_OUTPUT_READY_EVENT, &higher_priority_task_woken);
    return higher_priority_task_woken == pdTRUE;
});
```

### 音频输入

1. 当音频输入就绪时，获取音频数据：

```cpp
void Application::InputAudio() {
    auto codec = Board::GetInstance().GetAudioCodec();
    std::vector<int16_t> data;
    if (!codec->InputData(data)) {
        return;
    }

    // 处理音频数据...
}
```

### 音频输出

1. 当需要输出音频时，发送音频数据：

```cpp
void Application::OutputAudio() {
    // ...
    codec->OutputData(pcm);
}
```

## 音频数据流

### 输入流程

1. **硬件采集**：
   音频编解码器芯片通过 I2S 接口采集麦克风数据。

2. **数据处理**：
   `AudioCodec` 类的 `Read()` 方法读取 I2S 数据，并通过 `InputData()` 方法提供给应用。

3. **回调通知**：
   当有新的音频数据可用时，通过 `on_input_ready_` 回调通知应用。

### 输出流程

1. **数据发送**：
   应用通过 `OutputData()` 方法发送 PCM 音频数据。

2. **数据处理**：
   `AudioCodec` 类的 `Write()` 方法将 PCM 数据写入 I2S 接口。

3. **硬件播放**：
   音频编解码器芯片通过 I2S 接口将数据发送到扬声器。

4. **回调通知**：
   当需要更多音频数据时，通过 `on_output_ready_` 回调通知应用。

## 音频参数

### 采样率

项目支持不同的采样率，常用的有：
- 16000 Hz：用于语音识别和唤醒词检测
- 44100 Hz：用于音乐播放
- 48000 Hz：用于高质量音频

### 通道数

项目支持单通道和双通道音频：
- 单通道（mono）：用于语音识别和唤醒词检测
- 双通道（stereo）：用于音乐播放或回声消除（一个通道为麦克风，一个通道为参考信号）

### 位深度

项目使用 16 位深度的音频数据（int16_t），这是语音处理的常用格式。

## 特殊功能

### 回声消除

某些编解码器实现支持回声消除功能，通过参考信号（通常是扬声器输出）消除麦克风采集到的回声。

```cpp
bool input_reference() const { return input_reference_; }
```

### 双工模式

某些编解码器实现支持全双工模式，可以同时进行音频输入和输出。

```cpp
bool duplex() const { return duplex_; }
```

## 总结

音频编解码器模块是小智 AI 聊天机器人的核心组件，它负责音频输入和输出功能，是语音交互的基础。通过抽象基类 `AudioCodec` 和多种具体实现，项目实现了灵活的音频处理机制，支持不同类型的音频硬件。
