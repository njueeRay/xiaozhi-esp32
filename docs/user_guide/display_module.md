# 小智 AI 聊天机器人 - 显示模块解析

## 显示模块概述

显示模块负责管理小智 AI 聊天机器人的显示功能，包括显示状态信息、聊天消息、表情等。项目支持多种显示设备，如 OLED 显示屏和 LCD 显示屏。

## 核心组件

### 1. Display 类

`Display` 类是显示设备的抽象基类，定义了显示功能的通用接口。 

```cpp
class Display {
public:
virtual ~Display() = default;
// 初始化显示设备
virtual void Initialize() = 0;
// 设置状态信息
virtual void SetStatus(const char status) = 0;
// 设置图标
virtual void SetIcon(uint16_t icon_code) = 0;
// 设置表情
virtual void SetEmotion(const char emotion) = 0;
// 设置聊天消息
virtual void SetChatMessage(const char role, const char message) = 0;
// 显示通知
virtual void ShowNotification(const char message) = 0;
// 清除显示
virtual void Clear() = 0;
// 更新显示
virtual void Update() = 0;
};
```

### 2. SSD1306Display 类

`SSD1306Display` 类实现了基于 SSD1306 OLED 显示屏的显示功能。

```cpp
class SSD1306Display : public Display {
public:
    SSD1306Display(int width, int height, int sda_pin, int scl_pin, int reset_pin = -1);
    virtual ~SSD1306Display();

    void Initialize() override;
    void SetStatus(const char* status) override;
    void SetIcon(uint16_t icon_code) override;
    void SetEmotion(const char* emotion) override;
    void SetChatMessage(const char* role, const char* message) override;
    void ShowNotification(const char* message) override;
    void Clear() override;
    void Update() override;

private:
    // ... 私有成员 ...
};
```

### 3. LcdDisplay 类

`LcdDisplay` 类实现了基于 LCD 显示屏的显示功能，支持更丰富的显示效果，如彩色图像和动画。

```cpp
class LcdDisplay : public Display {
public:
    LcdDisplay(int width, int height);
    virtual ~LcdDisplay();

    void Initialize() override;
    void SetStatus(const char* status) override;
    void SetIcon(uint16_t icon_code) override;
    void SetEmotion(const char* emotion) override;
    void SetChatMessage(const char* role, const char* message) override;
    void ShowNotification(const char* message) override;
    void Clear() override;
    void Update() override;

    // LCD 特有方法
    void SetBacklight(int level);
    void DrawImage(const uint8_t* image_data, int x, int y, int width, int height);
    void DrawAnimation(const uint8_t* animation_data, int frame_count, int frame_delay);

private:
    // ... 私有成员 ...
};
```

### 4. NoDisplay 类

`NoDisplay` 类是一个空实现，用于不支持显示功能的设备。

```cpp
class NoDisplay : public Display {
public:
    NoDisplay() = default;
    virtual ~NoDisplay() = default;

    void Initialize() override {}
    void SetStatus(const char* status) override {}
    void SetIcon(uint16_t icon_code) override {}
    void SetEmotion(const char* emotion) override {}
    void SetChatMessage(const char* role, const char* message) override {}
    void ShowNotification(const char* message) override {}
    void Clear() override {}
    void Update() override {}
};
```

## 工作流程

### 初始化

1. 在应用启动时，创建并初始化显示设备：

```cpp
auto display = board.GetDisplay();
display->Initialize();
```

### 状态显示

1. 当设备状态变化时，更新状态显示：

```cpp
void Application::SetDeviceState(DeviceState state) {
    // ...
    auto display = board.GetDisplay();
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
        // ...
    }
}
```

### 聊天消息显示

1. 当接收到聊天消息时，更新显示：

```cpp
protocol_->OnIncomingJson([this, display](const cJSON* root) {
    // ...
    if (strcmp(type->valuestring, "tts") == 0) {
        // ...
        } else if (strcmp(state->valuestring, "sentence_start") == 0) {
            auto text = cJSON_GetObjectItem(root, "text");
            if (text != NULL) {
                ESP_LOGI(TAG, "<< %s", text->valuestring);
                Schedule([this, display, message = std::string(text->valuestring)]() {
                    display->SetChatMessage("assistant", message.c_str());
                });
            }
        }
    } else if (strcmp(type->valuestring, "stt") == 0) {
        auto text = cJSON_GetObjectItem(root, "text");
        if (text != NULL) {
            ESP_LOGI(TAG, ">> %s", text->valuestring);
            Schedule([this, display, message = std::string(text->valuestring)]() {
                display->SetChatMessage("user", message.c_str());
            });
        }
    }
    // ...
});
```

### 表情显示

1. 当接收到表情信息时，更新显示：

```cpp
} else if (strcmp(type->valuestring, "llm") == 0) {
    auto emotion = cJSON_GetObjectItem(root, "emotion");
    if (emotion != NULL) {
        Schedule([this, display, emotion_str = std::string(emotion->valuestring)]() {
            display->SetEmotion(emotion_str.c_str());
        });
    }
}
```

### 通知显示

1. 当需要显示通知时，调用相应方法：

```cpp
std::string message = std::string(Lang::Strings::VERSION) + ota_.GetCurrentVersion();
display->ShowNotification(message.c_str());
```

## 显示内容

### 状态信息

状态信息显示在屏幕顶部，表示设备当前的状态，如"待机"、"连接中"、"监听中"、"说话中"等。

### 图标

图标用于表示特定的功能或状态，如下载图标表示正在更新固件。

### 表情

表情用于表达 AI 助手的情感状态，如"中性"、"高兴"、"悲伤"等。对于 LCD 显示屏，表情可能需要通过图像来表示。

### 聊天消息

聊天消息显示用户和 AI 助手的对话内容。根据角色不同（用户、助手、系统），消息可能有不同的显示样式。

## 实现细节

### OLED 显示屏

OLED 显示屏通常使用 I2C 或 SPI 接口与 ESP32 通信。`SSD1306Display` 类使用 I2C 接口控制 SSD1306 OLED 显示屏，支持显示文本和简单图形。

由于 OLED 显示屏的分辨率和显示能力有限，显示内容需要经过优化，如使用小字体、滚动显示长文本等。

### LCD 显示屏

LCD 显示屏通常使用 SPI 接口与 ESP32 通信。`LcdDisplay` 类支持更丰富的显示功能，如彩色图像、动画等。

LCD 显示屏的分辨率和显示能力更强，可以显示更多内容和更复杂的图形，但也需要更多的处理能力和内存。

## 多语言支持

显示模块支持多语言显示，语言字符串存储在 `main/assets/` 目录下的 JSON 文件中。通过 `Lang::Strings` 命名空间访问当前语言的字符串。

```cpp
display->SetStatus(Lang::Strings::STANDBY);
```

## 总结

显示模块是小智 AI 聊天机器人的重要组成部分，它使设备能够与用户进行交互，显示状态信息、聊天消息和通知。通过 `Display` 类和相关子类，项目实现了灵活的显示功能，支持多种显示设备和语言。