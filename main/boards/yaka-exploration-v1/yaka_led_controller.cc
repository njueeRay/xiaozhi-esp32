#include "yaka_led_controller.h"
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>

const char* YakaLedController::TAG = "YakaLedController";

YakaLedController::YakaLedController() : 
    rgb_led_(nullptr),
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
    
    if (rgb_led_) {
        delete rgb_led_;
    }
}

void YakaLedController::Initialize() {
    ESP_LOGI(TAG, "Initializing YAKA LED Controller");
    
    // LED引脚定义
    gpio_num_t led_pins[LED_COUNT] = {
        LED1_GPIO, LED2_GPIO, LED3_GPIO, LED4_GPIO, LED5_GPIO
    };
    
    // 初始化5个单色LED
    for (int i = 0; i < LED_COUNT; i++) {
        single_leds_[i] = new GpioLed(
            led_pins[i], 
            0, // 不反转输出
            static_cast<ledc_timer_t>(LEDC_TIMER_1 + i), 
            static_cast<ledc_channel_t>(LEDC_CHANNEL_1 + i)
        );
        single_leds_[i]->SetBrightness(50); // 设置亮度为50%
    }
    
    // 初始化RGB LED (WS2812)
    rgb_led_ = new SingleLed(BUILTIN_LED_GPIO);
    
    leds_state_ = false;
    ESP_LOGI(TAG, "LED Controller initialized with %d LEDs", LED_COUNT);
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

void YakaLedController::SetLEDBrightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 100) brightness = 100;
    
    ESP_LOGI(TAG, "Setting LED brightness to %d%%", brightness);
    
    for (int i = 0; i < LED_COUNT; i++) {
        if (single_leds_[i]) {
            single_leds_[i]->SetBrightness(brightness);
        }
    }
}

void YakaLedController::ShowStartupSequence() {
    ESP_LOGI(TAG, "Starting LED startup sequence");
    
    // 逐个点亮LED，创建启动效果
    for (int i = 0; i < LED_COUNT; i++) {
        SetSingleLED(i, true);
        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms延迟
    }
    
    // RGB LED状态将通过系统自动管理，不直接控制
    ESP_LOGI(TAG, "RGB LED status will be controlled by system state");
    
    leds_state_ = true;
    ESP_LOGI(TAG, "Startup sequence completed");
}

void YakaLedController::ShowShutdownSequence() {
    ESP_LOGI(TAG, "Starting LED shutdown sequence");
    
    // RGB LED状态将通过系统自动管理，不直接控制
    ESP_LOGI(TAG, "RGB LED status will be controlled by system state");
    vTaskDelay(pdMS_TO_TICKS(500)); // 保持延迟以显示关机意图
    
    // 逐个关闭LED
    for (int i = LED_COUNT - 1; i >= 0; i--) {
        SetSingleLED(i, false);
        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms延迟
    }
    
    // RGB LED关闭将由系统状态自动管理
    ESP_LOGI(TAG, "RGB LED shutdown will be managed by system state");
    
    leds_state_ = false;
    ESP_LOGI(TAG, "Shutdown sequence completed");
} 