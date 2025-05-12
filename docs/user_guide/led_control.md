# 小智 AI 聊天机器人 - LED 控制模块解析

## LED 控制概述

LED 控制模块负责管理小智 AI 聊天机器人的 LED 指示灯，用于显示设备状态、提供视觉反馈等。项目支持多种 LED 类型，如单个 LED 和 LED 环形灯带。

## 核心组件

### 1. Led 类

`Led` 类是 LED 控制的抽象基类，定义了 LED 功能的通用接口。 

```cpp
class Led {
public:
virtual ~Led() = default;
virtual void Initialize() = 0;
virtual void OnStateChanged() = 0;
};
```

### 核心方法

- `Initialize()`: 初始化 LED
- `OnStateChanged()`: 当设备状态变化时更新 LED 显示

### 2. SingleLed 类

`SingleLed` 类实现了单个 LED 的控制功能。

```cpp
class SingleLed : public Led {
public:
    SingleLed(int pin, bool active_high = true);
    virtual ~SingleLed();
    
    void Initialize() override;
    void OnStateChanged() override;
    
    void SetBrightness(int brightness);
    void SetBlinking(int on_ms, int off_ms);
    void SetBreathing(int period_ms);
    
private:
    // ... 私有成员 ...
};
```

### 3. CircularStrip 类

`CircularStrip` 类实现了 LED 环形灯带的控制功能，支持更复杂的灯光效果。

```cpp
class CircularStrip : public Led {
public:
    CircularStrip(int pin, int num_leds);
    virtual ~CircularStrip();
    
    void Initialize() override;
    void OnStateChanged() override;
    
    void SetColor(uint32_t color);
    void SetColorAll(uint32_t color);
    void SetColorRange(int start, int end, uint32_t color);
    void SetSpinning(uint32_t color, int period_ms);
    void SetBreathing(uint32_t color, int period_ms);
    void SetProgress(uint32_t color, int progress);
    
private:
    // ... 私有成员 ...
};
```

## 工作流程

### 初始化

1. 在应用启动时，创建并初始化 LED：

```cpp
auto led = board.GetLed();
led->Initialize();
```

### 状态变化响应

1. 当设备状态变化时，更新 LED 显示：

```cpp
void Application::SetDeviceState(DeviceState state) {
    // ...
    auto led = board.GetLed();
    led->OnStateChanged();
    // ...
}
```

### 语音活动指示

1. 当检测到语音活动时，更新 LED 显示：

```cpp
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

## LED 效果

### 单个 LED

单个 LED 支持以下效果：
- 常亮：表示设备处于特定状态
- 闪烁：表示设备正在处理或等待
- 呼吸：表示设备处于待机状态

### LED 环形灯带

LED 环形灯带支持更丰富的效果：
- 单色：所有 LED 显示相同颜色
- 范围：部分 LED 显示特定颜色
- 旋转：颜色沿灯带旋转
- 呼吸：亮度周期性变化
- 进度：显示进度条效果

## 状态指示

不同的设备状态对应不同的 LED 效果：

- **空闲状态**：
  - 单个 LED：呼吸效果
  - LED 环形灯带：呼吸效果，蓝色

- **连接中**：
  - 单个 LED：快速闪烁
  - LED 环形灯带：旋转效果，蓝色

- **监听中**：
  - 单个 LED：常亮
  - LED 环形灯带：常亮，蓝色
  - 检测到语音时：亮度增加或颜色变化

- **说话中**：
  - 单个 LED：慢速闪烁
  - LED 环形灯带：旋转效果，绿色

- **升级中**：
  - 单个 LED：快速闪烁
  - LED 环形灯带：进度条效果，显示升级进度

## 实现细节

### 单个 LED

单个 LED 通常通过 GPIO 直接控制，使用 PWM 调节亮度。

```cpp
void SingleLed::SetBrightness(int brightness) {
    // 使用 PWM 设置亮度
    // ...
}
```

### LED 环形灯带

LED 环形灯带通常使用 WS2812 或类似协议，需要特殊的时序控制。

```cpp
void CircularStrip::SetColor(uint32_t color) {
    // 设置 LED 颜色
    // ...
}
```

## 总结

LED 控制模块为小智 AI 聊天机器人提供了直观的状态指示功能，增强了用户体验。通过抽象基类 `Led` 和具体实现 `SingleLed` 和 `CircularStrip`，项目实现了灵活的 LED 控制机制，支持不同类型的 LED 硬件和多种灯光效果。