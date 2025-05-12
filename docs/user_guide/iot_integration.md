# 小智 AI 聊天机器人 - IoT 集成解析

## IoT 集成概述

小智 AI 聊天机器人支持 IoT（物联网）功能，允许用户通过语音控制智能设备。IoT 集成模块负责管理设备描述符、设备状态和命令执行。

## 核心组件

### 1. Thing 类

`Thing` 类是 IoT 设备的抽象基类，定义了设备的通用接口。 

```cpp
class Thing {
public:
virtual ~Thing() = default;
// 获取设备 ID
virtual const std::string& id() const = 0;
// 获取设备类型
virtual const std::string& type() const = 0;
// 获取设备名称
virtual const std::string& name() const = 0;
// 获取设备描述符
virtual cJSON GetDescriptor() const = 0;
// 获取设备状态
virtual cJSON GetState() const = 0;
// 执行命令
virtual bool Invoke(const cJSON command) = 0;
};
```

### 2. ThingManager 类

`ThingManager` 类负责管理所有 IoT 设备，包括注册设备、获取设备状态和执行命令。

```cpp
class ThingManager {
public:
    static ThingManager& GetInstance();

    // 注册设备
    void Register(std::unique_ptr<Thing> thing);
    
    // 获取设备描述符 JSON
    std::string GetDescriptorsJson() const;
    
    // 获取设备状态 JSON
    std::string GetStatesJson() const;
    
    // 执行命令
    bool Invoke(const cJSON* command);

private:
    std::vector<std::unique_ptr<Thing>> things_;
};
```

### 3. 具体设备实现

项目在 `main/iot/things/` 目录下实现了各种具体的 IoT 设备类，如灯光控制、温度传感器等。

## 工作流程

### 设备注册

1. 在应用启动时，创建并注册 IoT 设备：

```cpp
auto& thing_manager = iot::ThingManager::GetInstance();
thing_manager.Register(std::make_unique<LightThing>("light1", "主灯"));
thing_manager.Register(std::make_unique<TemperatureSensor>("temp1", "温度传感器"));
```

### 设备描述符发送

1. 当音频通道打开时，应用发送设备描述符到服务器：

```cpp
protocol_->OnAudioChannelOpened([this, codec, &board]() {
    // ...
    // IoT device descriptors
    last_iot_states_.clear();
    auto& thing_manager = iot::ThingManager::GetInstance();
    protocol_->SendIotDescriptors(thing_manager.GetDescriptorsJson());
});
```

### 设备状态更新

1. 当设备进入监听状态时，应用更新并发送设备状态：

```cpp
void Application::UpdateIotStates() {
    auto& thing_manager = iot::ThingManager::GetInstance();
    auto states = thing_manager.GetStatesJson();
    if (states != last_iot_states_) {
        last_iot_states_ = states;
        protocol_->SendIotStates(states);
    }
}
```

### 命令执行

1. 当接收到 IoT 命令时，应用执行相应的操作：

```cpp
protocol_->OnIncomingJson([this, display](const cJSON* root) {
    // ...
    } else if (strcmp(type->valuestring, "iot") == 0) {
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
```

## 设备描述符格式

设备描述符使用 JSON 格式，描述设备的 ID、类型、名称和支持的操作：

```json
{
  "things": [
    {
      "id": "light1",
      "type": "light",
      "name": "主灯",
      "properties": [
        {
          "id": "power",
          "type": "boolean",
          "name": "电源"
        },
        {
          "id": "brightness",
          "type": "integer",
          "name": "亮度",
          "min": 0,
          "max": 100
        }
      ]
    }
  ]
}
```

## 设备状态格式

设备状态也使用 JSON 格式，描述设备的当前状态：

```json
{
  "states": [
    {
      "id": "light1",
      "properties": {
        "power": true,
        "brightness": 80
      }
    }
  ]
}
```

## 命令格式

命令使用 JSON 格式，描述要执行的操作：

```json
{
  "id": "light1",
  "property": "power",
  "value": true
}
```

## 总结

IoT 集成模块使小智 AI 聊天机器人能够与智能设备交互，实现语音控制功能。通过 `Thing` 和 `ThingManager` 类，项目实现了灵活的 IoT 设备管理机制，支持各种类型的设备和操作。