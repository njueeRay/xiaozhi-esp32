# 小智 AI 聊天机器人 - 应用核心层解析

## 应用核心概述

应用核心层是小智 AI 聊天机器人的中枢神经系统，负责协调各个模块的工作，管理设备状态和处理用户交互。应用核心层主要由 `Application` 类实现，采用单例模式设计，确保整个系统中只有一个应用实例。

## Application 类

`Application` 类是应用核心层的核心，它管理整个应用的生命周期和状态，协调各个模块的工作。

```cpp
class Application {
public:
    static Application& GetInstance();
    
    void Initialize();
    void Run();
    void Schedule(std::function<void()> callback);
    void WaitForCompletion();
    
    void SetDeviceState(DeviceState state);
    DeviceState GetDeviceState() const;
    
    void StartListening(ListeningMode mode = kListeningModeNormal);
    void StopListening();
    void AbortSpeaking(AbortReason reason = kAbortReasonUnknown);
    
    // ... 其他方法 ...

private:
    Application();
    ~Application();
    
    // 禁止拷贝和赋值
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    // 私有成员变量
    DeviceState device_state_ = kDeviceStateUnknown;
    BackgroundTask background_task_;
    Protocol* protocol_ = nullptr;
    Ota ota_;
    OpusEncoder opus_encoder_;
    OpusDecoder opus_decoder_;
    WakeWordDetect wake_word_detect_;
    EventGroupHandle_t event_group_ = nullptr;
    
    // 私有方法
    void InputAudio();
    void OutputAudio();
    void ProcessAudio();
    void CheckNewVersion();
    void ShowActivationCode();
    void UpdateIotStates();
    void Alert(const char* title, const char* message, const char* emotion, const char* sound);
    void Reboot();
    
    // ... 其他私有方法和变量 ...
};
```

## 核心功能

### 1. 初始化和运行

`Application` 类负责初始化各个模块并启动主循环：

```cpp
void Application::Initialize() {
    // 初始化事件组
    event_group_ = xEventGroupCreate();
    
    // 初始化硬件
    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    auto codec = board.GetAudioCodec();
    auto led = board.GetLed();
    
    display->Initialize();
    led->Initialize();
    
    // 设置设备状态
    SetDeviceState(kDeviceStateStarting);
    
    // 初始化音频处理
    wake_word_detect_.Initialize(16000, 1);
    opus_encoder_.Initialize(16000, 1, 16000);
    opus_decoder_.Initialize(16000, 1);
    
    // 设置唤醒词回调
    wake_word_detect_.OnWakeWordDetected([this](const std::string& wake_word) {
        Schedule([this, wake_word]() {
            if (device_state_ == kDeviceStateIdle) {
                ESP_LOGI(TAG, "Wake word detected: %s", wake_word.c_str());
                StartListening();
                protocol_->SendWakeWordDetected(wake_word);
            }
        });
    });
    
    // 设置语音活动检测回调
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
    
    // 初始化通信协议
    protocol_ = new WebsocketProtocol();
    protocol_->SetServerUrl(CONFIG_SERVER_URL);
    
    // 设置 OTA 更新 URL
    ota_.SetCheckVersionUrl(CONFIG_CHECK_VERSION_URL);
    
    // 设置按钮回调
    board.OnButtonPressed([this]() {
        ToggleChatState();
    });
    
    board.OnButtonLongPressed([this]() {
        StartListening();
    });
    
    // 启动网络
    board.StartNetwork();
    
    // 启动音频编解码器
    codec->Start();
    
    // 设置音频输入/输出回调
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
    
    // 设置协议回调
    protocol_->OnIncomingAudio([this](std::vector<uint8_t>&& data) {
        Schedule([this, data = std::move(data)]() {
            // 处理接收到的音频数据
            std::vector<int16_t> pcm;
            opus_decoder_.Decode(data, pcm);
            
            // 将解码后的音频数据添加到播放队列
            audio_output_queue_.push(std::move(pcm));
        });
    });
    
    protocol_->OnIncomingJson([this, display](const cJSON* root) {
        // 处理接收到的 JSON 数据
        auto type = cJSON_GetObjectItem(root, "type");
        if (type == NULL || type->type != cJSON_String) {
            return;
        }
        
        if (strcmp(type->valuestring, "tts") == 0) {
            // 处理 TTS 消息
            auto state = cJSON_GetObjectItem(root, "state");
            if (state != NULL && state->type == cJSON_String) {
                if (strcmp(state->valuestring, "start") == 0) {
                    SetDeviceState(kDeviceStateSpeaking);
                } else if (strcmp(state->valuestring, "end") == 0) {
                    SetDeviceState(kDeviceStateIdle);
                } else if (strcmp(state->valuestring, "sentence_start") == 0) {
                    auto text = cJSON_GetObjectItem(root, "text");
                    if (text != NULL) {
                        ESP_LOGI(TAG, "<< %s", text->valuestring);
                        Schedule([this, display, message = std::string(text->valuestring)]() {
                            display->SetChatMessage("assistant", message.c_str());
                        });
                    }
                }
            }
        } else if (strcmp(type->valuestring, "stt") == 0) {
            // 处理 STT 消息
            auto text = cJSON_GetObjectItem(root, "text");
            if (text != NULL) {
                ESP_LOGI(TAG, ">> %s", text->valuestring);
                Schedule([this, display, message = std::string(text->valuestring)]() {
                    display->SetChatMessage("user", message.c_str());
                });
            }
        } else if (strcmp(type->valuestring, "llm") == 0) {
            // 处理 LLM 消息
            auto emotion = cJSON_GetObjectItem(root, "emotion");
            if (emotion != NULL) {
                Schedule([this, display, emotion_str = std::string(emotion->valuestring)]() {
                    display->SetEmotion(emotion_str.c_str());
                });
            }
        } else if (strcmp(type->valuestring, "iot") == 0) {
            // 处理 IoT 消息
            auto commands = cJSON_GetObjectItem(root, "commands");
            if (commands != NULL) {
                auto& thing_manager = iot::ThingManager::GetInstance();
                for (int i = 0; i < cJSON_GetArraySize(commands); ++i) {
                    auto command = cJSON_GetArrayItem(commands, i);
                    thing_manager.Invoke(command);
                }
            }
        }
    });
    
    protocol_->OnAudioChannelOpened([this, codec, &board]() {
        // 音频通道打开时的处理
        ESP_LOGI(TAG, "Audio channel opened");
        
        // 启用音频输入
        codec->EnableInput(true);
        
        // 发送 IoT 设备描述符
        last_iot_states_.clear();
        auto& thing_manager = iot::ThingManager::GetInstance();
        protocol_->SendIotDescriptors(thing_manager.GetDescriptorsJson());
    });
    
    protocol_->OnAudioChannelClosed([this, codec]() {
        // 音频通道关闭时的处理
        ESP_LOGI(TAG, "Audio channel closed");
        
        // 禁用音频输入
        codec->EnableInput(false);
        
        // 清空音频输出队列
        while (!audio_output_queue_.empty()) {
            audio_output_queue_.pop();
        }
        
        // 如果设备状态为监听中或说话中，则设置为空闲状态
        if (device_state_ == kDeviceStateListening || device_state_ == kDeviceStateSpeaking) {
            SetDeviceState(kDeviceStateIdle);
        }
    });
    
    protocol_->OnNetworkError([this](const std::string& message) {
        // 网络错误处理
        ESP_LOGE(TAG, "Network error: %s", message.c_str());
        
        // 关闭音频通道
        if (protocol_->IsAudioChannelOpened()) {
            protocol_->CloseAudioChannel();
        }
        
        // 设置设备状态为空闲
        SetDeviceState(kDeviceStateIdle);
    });
    
    // 启动协议
    protocol_->Start();
    
    // 设置设备状态为空闲
    SetDeviceState(kDeviceStateIdle);
}

void Application::Run() {
    // 主循环
    while (true) {
        // 等待事件
        EventBits_t bits = xEventGroupWaitBits(
            event_group_,
            AUDIO_INPUT_READY_EVENT | AUDIO_OUTPUT_READY_EVENT,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(100)
        );
        
        // 处理音频输入
        if (bits & AUDIO_INPUT_READY_EVENT) {
            InputAudio();
        }
        
        // 处理音频输出
        if (bits & AUDIO_OUTPUT_READY_EVENT) {
            OutputAudio();
        }
        
        // 更新 IoT 设备状态
        UpdateIotStates();
        
        // 检查新版本
        static uint32_t last_check_version_time = 0;
        uint32_t now = esp_timer_get_time() / 1000000;
        if (now - last_check_version_time > 3600) {
            last_check_version_time = now;
            CheckNewVersion();
        }
    }
}
```

### 2. 状态管理

`Application` 类负责管理设备状态，不同状态下系统会有不同的行为：

```cpp
void Application::SetDeviceState(DeviceState state) {
    if (device_state_ == state) {
        return;
    }
    
    ESP_LOGI(TAG, "Device state: %d -> %d", device_state_, state);
    device_state_ = state;
    
    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    auto led = board.GetLed();
    
    switch (state) {
        case kDeviceStateIdle:
            display->SetStatus(Lang::Strings::STANDBY);
            display->SetEmotion("neutral");
            break;
        case kDeviceStateConnecting:
            display->SetStatus(Lang::Strings::CONNECTING);
            display->SetChatMessage("system", "");
            break;
        case kDeviceStateListening:
            display->SetStatus(Lang::Strings::LISTENING);
            display->SetEmotion("neutral");
            break;
        case kDeviceStateSpeaking:
            display->SetStatus(Lang::Strings::SPEAKING);
            break;
        case kDeviceStateUpgrading:
            display->SetStatus(Lang::Strings::UPGRADING);
            break;
        case kDeviceStateActivating:
            display->SetStatus(Lang::Strings::ACTIVATING);
            break;
        case kDeviceStateFatalError:
            display->SetStatus(Lang::Strings::FATAL_ERROR);
            break;
        default:
            break;
    }
    
    led->OnStateChanged();
}
```

### 3. 音频处理

`Application` 类负责处理音频输入和输出：

```cpp
void Application::InputAudio() {
    auto codec = Board::GetInstance().GetAudioCodec();
    std::vector<int16_t> data;
    if (!codec->InputData(data)) {
        return;
    }
    
    // 如果音频通道已打开，则处理音频数据
    if (protocol_->IsAudioChannelOpened()) {
        // 检测唤醒词和语音活动
        wake_word_detect_.ProcessAudio(data);
        
        // 如果设备状态为监听中，则编码并发送音频数据
        if (device_state_ == kDeviceStateListening) {
            ProcessAudio(data);
        }
    }
}

void Application::OutputAudio() {
    auto codec = Board::GetInstance().GetAudioCodec();
    
    // 如果音频输出队列为空，则返回
    if (audio_output_queue_.empty()) {
        return;
    }
    
    // 获取并移除队列中的第一个音频数据
    std::vector<int16_t> pcm = std::move(audio_output_queue_.front());
    audio_output_queue_.pop();
    
    // 输出音频数据
    codec->OutputData(pcm);
}

void Application::ProcessAudio(const std::vector<int16_t>& data) {
    // 编码音频数据
    std::vector<uint8_t> encoded;
    opus_encoder_.Encode(data, encoded);
    
    // 发送编码后的音频数据
    protocol_->SendAudio(encoded);
}
```

### 4. 用户交互

`Application` 类负责处理用户交互，如按钮按下和唤醒词检测：

```cpp
void Application::ToggleChatState() {
    if (device_state_ == kDeviceStateIdle) {
        StartListening();
    } else if (device_state_ == kDeviceStateListening) {
        StopListening();
    } else if (device_state_ == kDeviceStateSpeaking) {
        AbortSpeaking();
    }
}

void Application::StartListening(ListeningMode mode) {
    if (device_state_ != kDeviceStateIdle) {
        return;
    }
    
    // 如果音频通道未打开，则打开音频通道
    if (!protocol_->IsAudioChannelOpened()) {
        SetDeviceState(kDeviceStateConnecting);
        if (!protocol_->OpenAudioChannel()) {
            SetDeviceState(kDeviceStateIdle);
            return;
        }
    }
    
    // 设置设备状态为监听中
    SetDeviceState(kDeviceStateListening);
    
    // 发送开始监听命令
    protocol_->SendStartListening(mode);
}

void Application::StopListening() {
    if (device_state_ != kDeviceStateListening) {
        return;
    }
    
    // 发送停止监听命令
    protocol_->SendStopListening();
}

void Application::AbortSpeaking(AbortReason reason) {
    if (device_state_ != kDeviceStateSpeaking) {
        return;
    }
    
    // 发送中断说话命令
    protocol_->SendAbortSpeaking(reason);
    
    // 设置设备状态为空闲
    SetDeviceState(kDeviceStateIdle);
}
```

### 5. 后台任务

`Application` 类使用 `BackgroundTask` 类处理耗时操作，避免阻塞主线程：

```cpp
void Application::Schedule(std::function<void()> callback) {
    background_task_.Schedule(std::move(callback));
}

void Application::WaitForCompletion() {
    background_task_.WaitForCompletion();
}
```

### 6. OTA 更新

`Application` 类负责检查和安装新版本的固件：

```cpp
void Application::CheckNewVersion() {
    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    
    // 设置 POST 数据
    ota_.SetPostData(board.GetJson());
    
    // 检查新版本
    if (!ota_.CheckVersion()) {
        ESP_LOGW(TAG, "Check new version failed");
        return;
    }
    
    // 如果有新版本，则开始升级
    if (ota_.HasNewVersion()) {
        Alert(Lang::Strings::OTA_UPGRADE, Lang::Strings::UPGRADING, "happy", Lang::Sounds::P3_UPGRADE);
        
        // 等待设备状态为空闲
        do {
            vTaskDelay(pdMS_TO_TICKS(3000));
        } while (GetDeviceState() != kDeviceStateIdle);
        
        // 使用主任务进行升级，不可取消
        Schedule([this, display]() {
            SetDeviceState(kDeviceStateUpgrading);
            
            display->SetIcon(FONT_AWESOME_DOWNLOAD);
            std::string message = std::string(Lang::Strings::NEW_VERSION) + ota_.GetFirmwareVersion();
            display->SetChatMessage("system", message.c_str());
            
            // 开始升级
            ota_.StartUpgrade([display](int progress, size_t speed) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%d%% %zuKB/s", progress, speed / 1024);
                display->SetChatMessage("system", buffer);
            });
            
            // 如果升级失败，显示错误信息并重启
            display->SetStatus(Lang::Strings::UPGRADE_FAILED);
            ESP_LOGI(TAG, "Firmware upgrade failed...");
            vTaskDelay(pdMS_TO_TICKS(3000));
            Reboot();
        });
        
        return;
    }
    
    // 如果有激活码，则显示激活码
    if (ota_.HasActivationCode()) {
        SetDeviceState(kDeviceStateActivating);
        ShowActivationCode();
        
        // 等待设备状态为空闲
        for (int i = 0; i < 60; ++i) {
            if (device_state_ == kDeviceStateIdle) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        return;
    }
    
    // 标记当前版本为有效
    ota_.MarkCurrentVersionValid();
}
```

### 7. IoT 集成

`Application` 类负责管理 IoT 设备状态：

```cpp
void Application::UpdateIotStates() {
    // 如果音频通道未打开，则返回
    if (!protocol_->IsAudioChannelOpened()) {
        return;
    }
    
    // 获取 IoT 设备状态
    auto& thing_manager = iot::ThingManager::GetInstance();
    auto states = thing_manager.GetStatesJson();
    
    // 如果状态有变化，则发送新状态
    if (states != last_iot_states_) {
        last_iot_states_ = states;
        protocol_->SendIotStates(states);
    }
}
```

## 工作流程

### 1. 启动流程

1. 应用启动时，调用 `Application::Initialize()` 初始化各个模块
2. 初始化硬件、音频处理、通信协议等
3. 设置各种回调函数
4. 启动网络和音频编解码器
5. 设置设备状态为空闲
6. 调用 `Application::Run()` 启动主循环

### 2. 唤醒流程

1. 用户说出唤醒词或按下按钮
2. 如果是唤醒词，`WakeWordDetect` 类检测到唤醒词并调用回调函数
3. 调用 `Application::StartListening()` 开始监听
4. 打开音频通道并发送开始监听命令
5. 设置设备状态为监听中

### 3. 对话流程

1. 用户说话，音频数据通过 `InputAudio()` 方法获取
2. 音频数据经过处理后发送到服务器
3. 服务器返回文本和音频数据
4. 文本显示在屏幕上，音频数据通过 `OutputAudio()` 方法播放

### 4. 结束流程

1. 用户停止说话或按下按钮
2. 调用 `Application::StopListening()` 停止监听
3. 发送停止监听命令
4. 服务器处理完最后的音频数据后，设备状态变为说话中
5. 播放完所有音频数据后，设备状态变为空闲

## 总结

应用核心层是小智 AI 聊天机器人的中枢神经系统，它通过 `Application` 类协调各个模块的工作，管理设备状态和处理用户交互。应用核心层采用单例模式设计，确保整个系统中只有一个应用实例，并使用事件驱动和回调机制处理各种异步事件。通过应用核心层，小智 AI 聊天机器人能够实现流畅的语音交互体验。
