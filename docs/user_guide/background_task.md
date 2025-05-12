# 小智 AI 聊天机器人 - 后台任务模块解析

## 后台任务概述

后台任务模块负责处理小智 AI 聊天机器人中的耗时操作，如音频编解码、网络请求等，避免这些操作阻塞主线程，保证系统的响应性。

## 核心组件

### BackgroundTask 类

`BackgroundTask` 类是后台任务模块的核心，它提供了一个任务队列和一个工作线程，用于异步执行耗时操作。 

```cpp
class BackgroundTask {
public:
    BackgroundTask(uint32_t stack_size = 4096 * 2);
    ~BackgroundTask();

    void Schedule(std::function<void()> callback);
    void WaitForCompletion();

private:
    std::mutex mutex_;
    std::list<std::function<void()>> main_tasks_;
    std::condition_variable condition_variable_;
    TaskHandle_t background_task_handle_ = nullptr;
    std::atomic<size_t> active_tasks_{0};

    void BackgroundTaskLoop();
};
```

### 核心方法

- `Schedule()`: 将任务添加到队列中，等待后台线程执行
- `WaitForCompletion()`: 等待所有任务完成
- `BackgroundTaskLoop()`: 后台线程的主循环，不断从队列中取出任务执行

## 工作流程

### 初始化

1. 在应用启动时，创建 BackgroundTask 实例：

```cpp
BackgroundTask background_task_;
```

2. 构造函数会创建一个 FreeRTOS 任务，运行 `BackgroundTaskLoop()` 方法：

```cpp
BackgroundTask::BackgroundTask(uint32_t stack_size) {
    xTaskCreate([](void* arg) {
        BackgroundTask* task = (BackgroundTask*)arg;
        task->BackgroundTaskLoop();
    }, "background_task", stack_size, this, 1, &background_task_handle_);
}
```

### 任务调度

1. 当需要执行耗时操作时，应用调用 `Schedule()` 方法：

```cpp
background_task_.Schedule([this]() {
    // 执行耗时操作
    // ...
});
```

2. `Schedule()` 方法将任务添加到队列中，并通知后台线程：

```cpp
void BackgroundTask::Schedule(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (active_tasks_ >= 30) {
        int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        if (free_sram < 10000) {
            ESP_LOGW(TAG, "active_tasks_ == %u, free_sram == %u", active_tasks_.load(), free_sram);
        }
    }
    active_tasks_++;
    main_tasks_.emplace_back([this, cb = std::move(callback)]() {
        cb();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_tasks_--;
            if (main_tasks_.empty() && active_tasks_ == 0) {
                condition_variable_.notify_all();
            }
        }
    });
    condition_variable_.notify_all();
}
```

### 任务执行

1. 后台线程在 `BackgroundTaskLoop()` 方法中不断从队列中取出任务执行：

```cpp
void BackgroundTask::BackgroundTaskLoop() {
    ESP_LOGI(TAG, "background_task started");
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_variable_.wait(lock, [this]() { return !main_tasks_.empty(); });
        
        std::list<std::function<void()>> tasks = std::move(main_tasks_);
        lock.unlock();

        for (auto& task : tasks) {
            task();
        }
    }
}
```

### 等待完成

1. 当需要等待所有任务完成时，应用调用 `WaitForCompletion()` 方法：

```cpp
background_task_.WaitForCompletion();
```

2. `WaitForCompletion()` 方法会阻塞当前线程，直到所有任务完成：

```cpp
void BackgroundTask::WaitForCompletion() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_variable_.wait(lock, [this]() {
        return main_tasks_.empty() && active_tasks_ == 0;
    });
}
```

## 应用场景

### 音频处理

后台任务模块用于处理音频编解码等耗时操作：

```cpp
void Application::ProcessAudio() {
    background_task_.Schedule([this]() {
        // 编码音频数据
        std::vector<uint8_t> encoded;
        opus_encoder_.Encode(pcm_data_, encoded);
        
        // 发送编码后的数据
        protocol_->SendAudio(encoded);
    });
}
```

### 网络请求

后台任务模块用于处理网络请求等耗时操作：

```cpp
void Application::CheckNewVersion() {
    background_task_.Schedule([this]() {
        // 检查新版本
        if (ota_.CheckVersion()) {
            if (ota_.HasNewVersion()) {
                // 通知用户有新版本
                // ...
            }
        }
    });
}
```

### 资源释放

后台任务模块用于在后台释放资源，避免阻塞主线程：

```cpp
void Application::CleanupResources() {
    background_task_.Schedule([this]() {
        // 释放资源
        // ...
    });
}
```

## 内存管理

后台任务模块会监控活跃任务数量和内存使用情况，当活跃任务过多或内存不足时，会输出警告日志：

```cpp
if (active_tasks_ >= 30) {
    int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    if (free_sram < 10000) {
        ESP_LOGW(TAG, "active_tasks_ == %u, free_sram == %u", active_tasks_.load(), free_sram);
    }
}
```

## 总结

后台任务模块是小智 AI 聊天机器人的重要组成部分，它使应用能够在不阻塞主线程的情况下执行耗时操作，提高系统的响应性和稳定性。通过 `BackgroundTask` 类，项目实现了简单而高效的任务调度机制，支持各种异步操作。