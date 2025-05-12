# 小智 AI 聊天机器人 - 系统信息与设置模块解析

## 系统信息与设置概述

系统信息与设置模块负责管理小智 AI 聊天机器人的系统信息和用户设置，包括设备信息、网络配置、音频参数等。

## 核心组件

### 1. SystemInfo 类

`SystemInfo` 类负责管理系统信息，如设备 ID、固件版本、硬件信息等。 

```cpp
class SystemInfo {
public:
static SystemInfo& GetInstance();
// 获取设备信息
const char GetDeviceId() const;
const char GetFirmwareVersion() const;
const char GetHardwareVersion() const;
const char GetBoardType() const;
// 获取系统信息
int GetFreeHeap() const;
int GetMinimumFreeHeap() const;
float GetCpuFrequency() const;
float GetCpuTemperature() const;
// 获取网络信息
const char GetIpAddress() const;
const char GetMacAddress() const;
int GetWifiRssi() const;
// 获取 JSON 格式的系统信息
std::string GetJson() const;
private:
// ... 私有成员 ...
};
```

### 2. Settings 类

`Settings` 类负责管理用户设置，如音量、语言、服务器地址等。

```cpp
class Settings {
public:
    static Settings& GetInstance();
    
    // 音频设置
    int GetVolume() const;
    void SetVolume(int volume);
    
    // 语言设置
    const char* GetLanguage() const;
    void SetLanguage(const char* language);
    
    // 服务器设置
    const char* GetServerUrl() const;
    void SetServerUrl(const char* url);
    
    // Wi-Fi 设置
    const char* GetWifiSsid() const;
    void SetWifiSsid(const char* ssid);
    const char* GetWifiPassword() const;
    void SetWifiPassword(const char* password);
    
    // 保存和加载设置
    void Save();
    void Load();
    
private:
    // ... 私有成员 ...
};
```

## 工作流程

### 系统信息获取

1. 获取系统信息实例：

```cpp
auto& system_info = SystemInfo::GetInstance();
```

2. 获取特定信息：

```cpp
const char* device_id = system_info.GetDeviceId();
const char* firmware_version = system_info.GetFirmwareVersion();
int free_heap = system_info.GetFreeHeap();
```

3. 获取 JSON 格式的系统信息：

```cpp
std::string json = system_info.GetJson();
```

### 设置管理

1. 获取设置实例：

```cpp
auto& settings = Settings::GetInstance();
```

2. 获取设置值：

```cpp
int volume = settings.GetVolume();
const char* language = settings.GetLanguage();
```

3. 修改设置值：

```cpp
settings.SetVolume(80);
settings.SetLanguage("zh-CN");
```

4. 保存设置：

```cpp
settings.Save();
```

## 存储机制

设置值存储在 ESP32 的非易失性存储（NVS）中，确保设备重启后设置仍然有效。

```cpp
void Settings::Save() {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("settings", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    err = nvs_set_i32(handle, "volume", volume_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting volume: %s", esp_err_to_name(err));
    }
    
    err = nvs_set_str(handle, "language", language_.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting language: %s", esp_err_to_name(err));
    }
    
    // ... 保存其他设置 ...
    
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing settings: %s", esp_err_to_name(err));
    }
    
    nvs_close(handle);
}
```

## 应用场景

### 设备信息上报

系统信息用于向服务器上报设备信息，便于服务器了解设备状态和版本。

```cpp
void Application::ReportDeviceInfo() {
    auto& system_info = SystemInfo::GetInstance();
    std::string json = system_info.GetJson();
    
    // 发送设备信息到服务器
    // ...
}
```

### 音量调节

设置模块用于管理音频音量，响应用户的音量调节请求。

```cpp
void Application::SetVolume(int volume) {
    auto& settings = Settings::GetInstance();
    settings.SetVolume(volume);
    settings.Save();
    
    auto codec = Board::GetInstance().GetAudioCodec();
    codec->SetOutputVolume(volume);
}
```

### 语言切换

设置模块用于管理界面语言，支持多语言显示。

```cpp
void Application::SetLanguage(const char* language) {
    auto& settings = Settings::GetInstance();
    settings.SetLanguage(language);
    settings.Save();
    
    // 重新加载语言资源
    Lang::LoadLanguage(language);
    
    // 更新界面显示
    auto display = Board::GetInstance().GetDisplay();
    display->SetStatus(Lang::Strings::STANDBY);
}
```

### Wi-Fi 配置

设置模块用于管理 Wi-Fi 连接信息，支持 Wi-Fi 配置和重连。

```cpp
void Application::ConfigureWifi(const char* ssid, const char* password) {
    auto& settings = Settings::GetInstance();
    settings.SetWifiSsid(ssid);
    settings.SetWifiPassword(password);
    settings.Save();
    
    // 重新连接 Wi-Fi
    Board::GetInstance().StartNetwork();
}
```

## 总结

系统信息与设置模块是小智 AI 聊天机器人的重要组成部分，它负责管理设备信息和用户设置，支持设备信息上报、音量调节、语言切换和 Wi-Fi 配置等功能。通过 `SystemInfo` 和 `Settings` 类，项目实现了灵活的信息管理和设置存储机制，提高了设备的可用性和用户体验。
