# YAKA 探索版 V1 开发指导文档

## 项目概述

基于 ESP32-S3 的小智 AI 固件项目，YAKA 探索版 V1 是一个具有双按键控制、多LED指示、4G模块和音频处理能力的智能设备。

## 硬件配置分析

### 按键系统
- **KEYA**: GPIO_NUM_42 (主按键A)
- **KEYB**: GPIO_NUM_15 (主按键B)  
- **PWR_EN**: GPIO_NUM_41 (电源控制引脚)

### LED系统
- **5个单色LED**: GPIO 37, 36, 35, 4, 5 (从左到右LED1-LED5)
- **1个RGB LED (WS2812)**: GPIO_NUM_8

### 电源与模块控制
- **4G_EN**: GPIO_NUM_14 (4G模块使能)
- **MIC_EN**: GPIO_NUM_9 (音频编解码芯片使能)

### 4G模块通信
- **4G_RX**: GPIO_NUM_43
- **4G_TX**: GPIO_NUM_44

### 音频编解码芯片 (ES8388)
- **MCLK**: GPIO_NUM_38
- **WS**: GPIO_NUM_47
- **BCLK**: GPIO_NUM_48
- **DIN**: GPIO_NUM_21
- **DOUT**: GPIO_NUM_45
- **I2C_SDA**: GPIO_NUM_1
- **I2C_SCL**: GPIO_NUM_2

## 需要实现的功能

### 核心功能要求
1. **长按开关机**: 长按按键A或B(>1s)实现开关机
2. **自动模块启动**: 开机后自动启动4G和MIC模块，点亮所有LED
3. **短按LED控制**: 短按按键A(90-250ms)切换5个单色LED状态

## 代码架构分析

### 现有项目结构
项目基于 ESP-IDF 框架，采用面向对象的C++设计：

1. **开发板基类**: `WifiBoard` - 提供WiFi连接功能
2. **按键处理**: `Button` 类 - 支持短按、长按、双击等事件
3. **LED控制**: 
   - `GpioLed` - 单色LED控制
   - `SingleLed` - WS2812 RGB LED控制
4. **音频处理**: `Es8388AudioCodec` - ES8388音频编解码器
5. **物联网**: `Thing` 系统 - 设备抽象和MCP服务器集成

### 关键代码文件位置
- 开发板实现: `main/boards/yaka-exploration-v1/yaka-exploration-v1.cc`
- 配置文件: `main/boards/yaka-exploration-v1/config.h`
- 按键处理: `main/boards/common/button.cc`
- LED控制: `main/led/gpio_led.cc`, `main/led/single_led.cc`
- 音频编解码: `main/audio_codecs/es8388_audio_codec.cc`

## 具体实现计划

### 1. 更新配置文件 (config.h)
需要添加缺失的GPIO定义：
```cpp
// 按键定义
#define KEYA_GPIO                 GPIO_NUM_42
#define KEYB_GPIO                 GPIO_NUM_15  
#define PWR_EN_GPIO               GPIO_NUM_41

// LED定义
#define LED1_GPIO                 GPIO_NUM_37
#define LED2_GPIO                 GPIO_NUM_36
#define LED3_GPIO                 GPIO_NUM_35
#define LED4_GPIO                 GPIO_NUM_4
#define LED5_GPIO                 GPIO_NUM_5

// 模块控制
#define G4_EN_GPIO                GPIO_NUM_14
#define MIC_EN_GPIO               GPIO_NUM_9

// 4G模块
#define MODULE_4G_RX_PIN          GPIO_NUM_43
#define MODULE_4G_TX_PIN          GPIO_NUM_44
```

### 2. 修改主板实现文件 (yaka-exploration-v1.cc)

#### 需要添加的类成员变量：
```cpp
private:
    Button key_a_button_;           // 按键A
    Button key_b_button_;           // 按键B  
    GpioLed* single_leds_[5];       // 5个单色LED
    bool leds_state_;               // LED状态
    bool power_enabled_;            // 电源状态
```

#### 需要实现的新方法：
```cpp
void InitializeGPIO();              // GPIO初始化
void InitializeLEDs();              // LED初始化  
void InitializePowerControl();      // 电源控制初始化
void InitializeButtons();           // 按键初始化(需要重写)
void EnableModules();               // 启用4G和MIC模块
void DisableModules();              // 禁用模块
void ToggleLEDs();                  // 切换LED状态
void SetAllLEDs(bool state);        // 设置所有LED状态
void PowerOn();                     // 开机处理
void PowerOff();                    // 关机处理
```

### 3. 按键事件处理逻辑

#### 长按开关机逻辑：
- 检测按键A或B持续低电平>1s
- 开机时设置PWR_EN为高电平，启用所有模块和LED
- 关机时设置PWR_EN为低电平，进入休眠模式

#### 短按LED控制逻辑：
- 检测按键A低电平持续90-250ms
- 切换5个单色LED的开关状态

### 4. LED控制系统
- 创建5个`GpioLed`实例控制单色LED
- 保持现有的WS2812 RGB LED作为状态指示
- 实现LED组控制功能

### 5. 电源管理系统
- 实现基于PWR_EN的电源控制
- 添加模块使能控制(4G_EN, MIC_EN)
- 实现开机自启动序列

### 6. 音频系统集成
- 确保ES8388编解码器正确初始化
- 配置I2S接口参数
- 实现MIC_EN控制逻辑

## 开发优先级

### 第一阶段（基础功能）
1. 更新config.h配置文件
2. 实现GPIO初始化
3. 实现基础的LED控制
4. 实现按键检测和事件处理

### 第二阶段（核心功能）  
1. 实现电源控制逻辑
2. 实现开关机功能
3. 实现模块自动启动

### 第三阶段（完善功能）
1. 4G模块集成
2. 音频系统完善
3. 系统稳定性优化

## 参考实现

### 参考的开发板实现：
- `esp-spot-s3`: 电源控制和长按关机
- `sensecap-watcher`: IO扩展器和按键处理
- `magiclick-2p4`: 多按键和LED控制
- `esp32-s3-touch-lcd-1.85`: 电源管理

### 关键参考代码段：
1. **GPIO初始化**: 参考`esp-spot-s3`的`InitializeGPIO()`
2. **按键长按处理**: 参考`esp-spot-s3`的长按关机逻辑
3. **LED控制**: 参考`magiclick-2p4`的LED电源控制
4. **电源管理**: 参考`sensecap-watcher`的电源控制

## 注意事项

1. **按键消抖**: 确保按键检测有适当的消抖处理
2. **电源时序**: 模块启动需要考虑电源时序
3. **功耗管理**: 关机状态需要最小化功耗
4. **错误处理**: 添加适当的错误处理和日志输出
5. **硬件兼容**: 确保GPIO配置与硬件设计匹配

## 下一步行动

1. 首先修改配置文件添加所有GPIO定义
2. 实现基础的GPIO初始化和LED控制
3. 逐步添加按键处理和电源管理功能
4. 测试验证各项功能的正确性
5. 优化用户体验和系统稳定性
