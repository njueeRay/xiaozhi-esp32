# 小智 AI 聊天机器人 - OTA 更新功能解析

## OTA 更新概述

OTA（Over-The-Air）更新是小智 AI 聊天机器人的重要功能，它允许设备通过网络接收和安装新版本的固件，无需用户手动操作。

## Ota 类

`Ota` 类负责处理固件的在线更新功能，包括检查新版本、下载新固件和安装更新。

### 核心属性 

```cpp
// 版本检查 URL
std::string check_version_url_;
// HTTP 头部
std::map<std::string, std::string> headers_;
// 当前版本和新版本信息
std::string current_version_;
std::string firmware_version_;
std::string firmware_url_;
// 激活信息
std::string activation_code_;
std::string activation_message_;
// 服务器时间
bool has_server_time_ = false;
```

### 核心方法

#### 版本检查

```cpp
// 设置版本检查 URL
void SetCheckVersionUrl(const std::string& url);

// 设置 HTTP 头部
void SetHeader(const std::string& key, const std::string& value);

// 设置 POST 数据
void SetPostData(const std::string& data);

// 检查新版本
bool CheckVersion();

// 是否有新版本
bool HasNewVersion() const;

// 获取当前版本
const std::string& GetCurrentVersion() const;

// 获取固件版本
const std::string& GetFirmwareVersion() const;

// 标记当前版本为有效
void MarkCurrentVersionValid();
```

`CheckVersion()` 方法是版本检查的核心，它向服务器发送请求，获取最新版本信息，并与当前版本进行比较，判断是否有新版本可用。

#### 固件更新

```cpp
// 开始升级
void StartUpgrade(std::function<void(int progress, size_t speed)> progress_callback);
```

`StartUpgrade()` 方法负责下载和安装新固件。它接受一个回调函数，用于报告升级进度和下载速度。

#### 激活功能

```cpp
// 是否有激活码
bool HasActivationCode() const;

// 获取激活码
const std::string& GetActivationCode() const;

// 获取激活消息
const std::string& GetActivationMessage() const;
```

这些方法用于处理设备激活功能，当设备需要激活时，服务器会返回激活码和激活消息。

#### 服务器时间

```cpp
// 是否有服务器时间
bool HasServerTime() const;
```

此方法用于检查是否已从服务器获取时间信息。

### 工作流程

#### 版本检查流程

1. 应用调用 `SetCheckVersionUrl()` 设置版本检查 URL
2. 应用调用 `SetHeader()` 设置必要的 HTTP 头部
3. 应用调用 `SetPostData()` 设置 POST 数据（如设备信息）
4. 应用调用 `CheckVersion()` 检查新版本
5. 如果 `HasNewVersion()` 返回 true，表示有新版本可用

```cpp
void Application::CheckNewVersion() {
    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    // Check if there is a new firmware version available
    ota_.SetPostData(board.GetJson());

    const int MAX_RETRY = 10;
    int retry_count = 0;

    while (true) {
        if (!ota_.CheckVersion()) {
            retry_count++;
            if (retry_count >= MAX_RETRY) {
                ESP_LOGE(TAG, "Too many retries, exit version check");
                return;
            }
            ESP_LOGW(TAG, "Check new version failed, retry in %d seconds (%d/%d)", 60, retry_count, MAX_RETRY);
            vTaskDelay(pdMS_TO_TICKS(60000));
            continue;
        }
        retry_count = 0;

        if (ota_.HasNewVersion()) {
            // 有新版本，开始升级
            // ...
        }

        // 标记当前版本为有效
        ota_.MarkCurrentVersionValid();
        // ...
    }
}
```

#### 固件更新流程

1. 应用检测到有新版本可用
2. 应用通知用户有新版本
3. 应用调用 `StartUpgrade()` 开始升级
4. 升级过程中，回调函数报告进度
5. 升级成功后，设备重启并运行新固件

```cpp
if (ota_.HasNewVersion()) {
    Alert(Lang::Strings::OTA_UPGRADE, Lang::Strings::UPGRADING, "happy", Lang::Sounds::P3_UPGRADE);
    // Wait for the chat state to be idle
    do {
        vTaskDelay(pdMS_TO_TICKS(3000));
    } while (GetDeviceState() != kDeviceStateIdle);

    // Use main task to do the upgrade, not cancelable
    Schedule([this, display]() {
        SetDeviceState(kDeviceStateUpgrading);
        
        display->SetIcon(FONT_AWESOME_DOWNLOAD);
        std::string message = std::string(Lang::Strings::NEW_VERSION) + ota_.GetFirmwareVersion();
        display->SetChatMessage("system", message.c_str());

        // ... 准备升级 ...

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
```

#### 激活流程

1. 应用检测到设备需要激活
2. 应用显示激活码和激活消息
3. 用户使用激活码激活设备
4. 设备重新检查版本，确认激活状态

```cpp
if (ota_.HasActivationCode()) {
    // Activation code is valid
    SetDeviceState(kDeviceStateActivating);
    ShowActivationCode();

    // Check again in 60 seconds or until the device is idle
    for (int i = 0; i < 60; ++i) {
        if (device_state_ == kDeviceStateIdle) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    continue;
}
```

## 安全考虑

OTA 更新涉及到设备安全，项目采取了以下措施确保更新过程的安全性：

1. **HTTPS 连接**：使用 HTTPS 协议与服务器通信，确保数据传输的安全性。
2. **固件验证**：在安装新固件前，验证固件的完整性和真实性。
3. **版本回滚**：如果新固件出现问题，可以回滚到之前的版本。

## 总结

OTA 更新功能是小智 AI 聊天机器人的重要组成部分，它使设备能够保持最新状态，获取新功能和修复。通过 `Ota` 类，项目实现了完整的 OTA 更新流程，包括版本检查、固件下载和安装。

