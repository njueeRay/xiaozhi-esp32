# 小智 AI 聊天机器人 - 音频处理模块解析

## 音频处理概述

音频处理模块是小智 AI 聊天机器人的核心组件，负责处理音频输入和输出，包括唤醒词检测、语音活动检测、音频编解码和重采样等功能。

## 核心组件

### 1. AudioProcessor 类

`AudioProcessor` 类负责音频数据的处理，包括重采样、音量调整等功能。

```cpp
class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();

    void Initialize(int input_sample_rate, int output_sample_rate);
    void Resample(const std::vector<int16_t>& input, std::vector<int16_t>& output, int input_sample_rate, int output_sample_rate);
    void AdjustVolume(std::vector<int16_t>& data, float gain);
    void ApplyHighPassFilter(std::vector<int16_t>& data);
    void ApplyLowPassFilter(std::vector<int16_t>& data);
    void ApplyNoiseReduction(std::vector<int16_t>& data);

private:
    // ... 私有成员 ...
};
```

### 2. WakeWordDetect 类

`WakeWordDetect` 类负责唤醒词检测和语音活动检测（VAD）功能。

```cpp
class WakeWordDetect {
public:
    WakeWordDetect();
    ~WakeWordDetect();

    void Initialize(int sample_rate, int channels);
    bool ProcessAudio(const std::vector<int16_t>& audio);
    bool IsWakeWordDetected() const;
    bool IsVoiceDetected() const;
    void Reset();

    void OnWakeWordDetected(std::function<void(const std::string& wake_word)> callback);
    void OnVadStateChange(std::function<void(bool speaking)> callback);

private:
    // ... 私有成员 ...
};
```

### 3. OpusEncoder 类

`OpusEncoder` 类负责将 PCM 音频数据编码为 Opus 格式，以减小数据大小，适合网络传输。

```cpp
class OpusEncoder {
public:
    OpusEncoder();
    ~OpusEncoder();

    void Initialize(int sample_rate, int channels, int bitrate);
    void Encode(const std::vector<int16_t>& pcm, std::vector<uint8_t>& opus);
    void Reset();

private:
    // ... 私有成员 ...
};
```

### 4. OpusDecoder 类

`OpusDecoder` 类负责将 Opus 格式的音频数据解码为 PCM 格式，用于播放。

```cpp
class OpusDecoder {
public:
    OpusDecoder();
    ~OpusDecoder();

    void Initialize(int sample_rate, int channels);
    void Decode(const std::vector<uint8_t>& opus, std::vector<int16_t>& pcm);
    void Reset();

private:
    // ... 私有成员 ...
};
```

## 工作流程

### 音频输入流程

1. **采集音频**：
   从音频编解码器获取 PCM 音频数据。

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

2. **唤醒词检测**：
   将音频数据传递给 `WakeWordDetect` 类进行唤醒词检测和语音活动检测。

```cpp
bool wake_word_detected = wake_word_detect_.ProcessAudio(data);
if (wake_word_detected) {
    // 处理唤醒事件...
}
```

3. **音频处理**：
   对音频数据进行处理，如重采样、音量调整等。

```cpp
std::vector<int16_t> processed_data;
audio_processor_.Resample(data, processed_data, codec->input_sample_rate(), protocol_->server_sample_rate());
```

4. **音频编码**：
   将处理后的音频数据编码为 Opus 格式。

```cpp
std::vector<uint8_t> encoded_data;
opus_encoder_.Encode(processed_data, encoded_data);
```

5. **发送音频**：
   将编码后的音频数据发送到服务器。

```cpp
protocol_->SendAudio(encoded_data);
```

### 音频输出流程

1. **接收音频**：
   从服务器接收 Opus 格式的音频数据。

```cpp
protocol_->OnIncomingAudio([this](std::vector<uint8_t>&& data) {
    // 处理接收到的音频数据...
});
```

2. **音频解码**：
   将 Opus 格式的音频数据解码为 PCM 格式。

```cpp
std::vector<int16_t> pcm;
opus_decoder_.Decode(data, pcm);
```

3. **音频处理**：
   对解码后的音频数据进行处理，如重采样、音量调整等。

```cpp
std::vector<int16_t> processed_pcm;
audio_processor_.Resample(pcm, processed_pcm, protocol_->server_sample_rate(), codec->output_sample_rate());
```

4. **播放音频**：
   将处理后的音频数据发送到音频编解码器进行播放。

```cpp
codec->OutputData(processed_pcm);
```

## 唤醒词检测

唤醒词检测是音频处理模块的重要功能，它使设备能够在待机状态下监听特定的唤醒词，如"小智小智"。

### 初始化

```cpp
wake_word_detect_.Initialize(16000, 1);
```

### 回调设置

```cpp
wake_word_detect_.OnWakeWordDetected([this](const std::string& wake_word) {
    Schedule([this, wake_word]() {
        if (device_state_ == kDeviceStateIdle) {
            ESP_LOGI(TAG, "Wake word detected: %s", wake_word.c_str());
            StartListening();
            protocol_->SendWakeWordDetected(wake_word);
        }
    });
});

wake_word_detect_.OnVadStateChange([this](bool speaking) {
    Schedule([this, speaking]() {
        if (device_state_ == kDeviceStateListening) {
            if (speaking) {
                voice_detected_ = true;
            } else {
                voice_detected_ = false;
            }
            auto led = Board::GetInstance().GetLed();
            led->OnStateChanged();
        }
    });
});
```

### 音频处理

```cpp
bool WakeWordDetect::ProcessAudio(const std::vector<int16_t>& audio) {
    // 处理音频数据，检测唤醒词和语音活动
    // ...
    
    // 如果检测到唤醒词，调用回调函数
    if (wake_word_detected) {
        if (on_wake_word_detected_) {
            on_wake_word_detected_(wake_word);
        }
        return true;
    }
    
    // 如果语音活动状态变化，调用回调函数
    if (vad_state_changed) {
        if (on_vad_state_change_) {
            on_vad_state_change_(speaking);
        }
    }
    
    return false;
}
```

## 音频编解码

音频编解码是音频处理模块的另一个重要功能，它使设备能够高效地传输和存储音频数据。

### Opus 编码

```cpp
void OpusEncoder::Initialize(int sample_rate, int channels, int bitrate) {
    // 初始化 Opus 编码器
    // ...
}

void OpusEncoder::Encode(const std::vector<int16_t>& pcm, std::vector<uint8_t>& opus) {
    // 编码 PCM 数据为 Opus 格式
    // ...
}
```

### Opus 解码

```cpp
void OpusDecoder::Initialize(int sample_rate, int channels) {
    // 初始化 Opus 解码器
    // ...
}

void OpusDecoder::Decode(const std::vector<uint8_t>& opus, std::vector<int16_t>& pcm) {
    // 解码 Opus 数据为 PCM 格式
    // ...
}
```

## 音频重采样

音频重采样是音频处理模块的基础功能，它使设备能够处理不同采样率的音频数据。

```cpp
void AudioProcessor::Resample(const std::vector<int16_t>& input, std::vector<int16_t>& output, int input_sample_rate, int output_sample_rate) {
    // 重采样音频数据
    // ...
}
```

## 总结

音频处理模块是小智 AI 聊天机器人的核心组件，它负责处理音频输入和输出，实现唤醒词检测、语音活动检测、音频编解码和重采样等功能。通过 `AudioProcessor`、`WakeWordDetect`、`OpusEncoder` 和 `OpusDecoder` 等类，项目实现了完整的音频处理流程，支持高质量的语音交互。

