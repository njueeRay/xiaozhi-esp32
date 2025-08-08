#include "yaka_led_controller.h"
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>

const char* YakaLedController::TAG = "YakaLedController";

YakaLedController::YakaLedController() : 
    leds_state_(false) {
    
    for (int i = 0; i < LED_COUNT; i++) {
        single_leds_[i] = nullptr;
    }
}

YakaLedController::~YakaLedController() {
    for (int i = 0; i < LED_COUNT; i++) {
        if (single_leds_[i]) {
            delete single_leds_[i];
        }
    }
}

void YakaLedController::Initialize() {
    ESP_LOGI(TAG, "Initializing YAKA LED Controller");
    
    // LED引脚定义
    gpio_num_t led_pins[LED_COUNT] = {
        LED1_GPIO
    };
    
    // 初始化5个单色LED - 使用有效的LEDC定时器配置
    for (int i = 0; i < LED_COUNT; i++) {
        single_leds_[i] = new GpioLed(
            led_pins[i], 
            0);
    }
    ESP_LOGI(TAG, "LED Controller initialized with %d LEDs", LED_COUNT);    

    leds_state_ = false;
    ESP_LOGI(TAG, "LED Controller initialized - Finished!");
}

void YakaLedController::SetAllLEDs(bool state) {
    ESP_LOGI(TAG, "Setting all LEDs to %s", state ? "ON" : "OFF");
    
    for (int i = 0; i < LED_COUNT; i++) {
        if (single_leds_[i]) {
            if (state) {
                single_leds_[i]->TurnOn();
            } else {
                single_leds_[i]->TurnOff();
            }
        }
    }
    
    leds_state_ = state;
}

void YakaLedController::ToggleLEDs() {
    ESP_LOGI(TAG, "Toggling LEDs from %s to %s", 
             leds_state_ ? "ON" : "OFF", 
             leds_state_ ? "OFF" : "ON");
    SetAllLEDs(!leds_state_);
}

void YakaLedController::SetSingleLED(int index, bool state) {
    if (index >= 0 && index < LED_COUNT && single_leds_[index]) {
        ESP_LOGI(TAG, "Setting LED %d to %s", index + 1, state ? "ON" : "OFF");
        if (state) {
            single_leds_[index]->TurnOn();
        } else {
            single_leds_[index]->TurnOff();
        }
    } else {
        ESP_LOGW(TAG, "Invalid LED index: %d", index);
    }
}


void YakaLedController::ShowStartupSequence() {
    ESP_LOGI(TAG, "Starting LED startup sequence");
    
    // 逐个点亮LED，创建启动效果
    for (int i = 0; i < LED_COUNT; i++) {
        SetSingleLED(i, true);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1000ms延迟
    }
    
    leds_state_ = true;
    
    ESP_LOGI(TAG, "Startup sequence completed - All LEDs should be ON");
}

void YakaLedController::ShowShutdownSequence() {
    ESP_LOGI(TAG, "Starting LED shutdown sequence");
    
    vTaskDelay(pdMS_TO_TICKS(500)); // 保持延迟以显示关机意图
    
    // 逐个关闭LED
    for (int i = LED_COUNT - 1; i >= 0; i--) {
        SetSingleLED(i, false);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1000ms延迟
    }
        
    leds_state_ = false;
    ESP_LOGI(TAG, "Shutdown sequence completed");
} 